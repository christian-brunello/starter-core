
/*
 * starter-core - expr.c
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <starter/error.h>

#include "internals.h"

Expr *
expr_ref (Expr * self)
{
  self->nref++;

  return self;
}

void
expr_unref (Expr * self)
{
  if (self->nref > 1)
    self->nref--;
  else
    {
      if (self->callbacks.destroy)
	(*self->callbacks.destroy) (self);

      if (self->left)
	expr_unref (self->left);

      if (self->right)
	expr_unref (self->right);

      g_free (self);
    }
}

static gboolean
literal_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		   GHashTable * variables, gdouble * res, GError ** error)
{
  *res = self->data.literal;

  return TRUE;
}

static void
literal_expr_destroy (Expr * self)
{

}

static gchar *
literal_expr_describe (Expr * self, GString * s)
{
  g_string_printf (s, "%lf", self->data.literal);

  return s->str;
}

Expr *
expr_new_literal (gdouble val)
{
  Expr *r;

  r = g_malloc0 (sizeof (Expr));

  r->type = EXPR_TYPE_LITERAL;
  r->data.literal = val;

  r->callbacks.eval = literal_expr_eval;
  r->callbacks.destroy = literal_expr_destroy;
  r->callbacks.describe = literal_expr_describe;
  r->nref = 1;

  return r;
}

static gboolean
op_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
	      GHashTable * variables, gdouble * res, GError ** error)
{
  gboolean r = TRUE;
  gdouble lval;
  gdouble rval;

  if (!expr_eval (self->left, inputs, outputs, variables, &lval, error)
      || !expr_eval (self->right, inputs, outputs, variables, &rval, error))
    return FALSE;

  if (strcmp (self->data.op, "==") == 0)
    *res = lval == rval;
  else if (strcmp (self->data.op, "!=") == 0)
    *res = lval != rval;
  else if (strcmp (self->data.op, "<") == 0)
    *res = lval < rval;
  else if (strcmp (self->data.op, "<=") == 0)
    *res = lval <= rval;
  else if (strcmp (self->data.op, ">") == 0)
    *res = lval > rval;
  else if (strcmp (self->data.op, ">=") == 0)
    *res = lval >= rval;
  else if (strcmp (self->data.op, "&&") == 0)
    *res = lval && rval;
  else if (strcmp (self->data.op, "||") == 0)
    *res = lval || rval;
  else if (strcmp (self->data.op, "+") == 0)
    *res = lval + rval;
  else if (strcmp (self->data.op, "-") == 0)
    *res = lval - rval;
  else if (strcmp (self->data.op, "*") == 0)
    *res = lval * rval;
  else if (strcmp (self->data.op, "/") == 0)
    *res = lval / rval;
  else
    {
      g_set_error (error,
		   ST_ERROR,
		   ST_ERROR_INVALID_VALUE,
		   "unknown operator: %s", self->data.op);

      r = FALSE;
    }

  return r;
}

static void
op_expr_destroy (Expr * self)
{
  g_free (self->data.op);
}

static gchar *
op_expr_describe (Expr * self, GString * s)
{
  g_autoptr (GString) ls;
  g_autoptr (GString) rs;

  ls = g_string_new ("");
  rs = g_string_new ("");

  g_string_printf (s, "(%s) %s (%s)",
		   expr_describe (self->left, ls),
		   self->data.op, expr_describe (self->right, rs));

  return s->str;
}

static gboolean
op_expr_have_ref_to (Expr * self, const gchar * id, GHashTable * variables)
{
  return expr_have_ref_to (self->left, id, variables)
    || expr_have_ref_to (self->right, id, variables);
}

Expr *
expr_new_op (const gchar * op, Expr * left, Expr * right)
{
  Expr *r;

  r = g_malloc0 (sizeof (Expr));

  r->type = EXPR_TYPE_OP;
  r->data.op = g_strdup (op);

  r->callbacks.eval = op_expr_eval;
  r->callbacks.destroy = op_expr_destroy;
  r->callbacks.describe = op_expr_describe;
  r->callbacks.have_ref_to = op_expr_have_ref_to;

  r->left = expr_ref (left);
  r->right = expr_ref (right);
  r->nref = 1;

  return r;
}

static gboolean
ref_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
	       GHashTable * variables, gdouble * res, GError ** error)
{
  gpointer p;

  if (inputs && (p = g_hash_table_lookup (inputs, self->data.ref)) != NULL)
    {
      ClientInputMap *cmap = p;
      const GPtrArray *cinputs = st_client_get_inputs (cmap->client);

      LOGD ("found input at index %u, name: %s, value: %lf",
	    cmap->index,
	    st_input_get_name (cinputs->pdata[cmap->index]),
	    st_input_get_val (cinputs->pdata[cmap->index]));

      *res = st_input_get_val (cinputs->pdata[cmap->index]);

      return TRUE;
    }

  if (outputs && (p = g_hash_table_lookup (outputs, self->data.ref)) != NULL)
    {
      ClientOutputMap *cmap = p;
      const GPtrArray *outputs = st_client_get_outputs (cmap->client);

      *res = st_output_get_val (outputs->pdata[cmap->index]);

      return TRUE;
    }

  if (variables
      && (p = g_hash_table_lookup (variables, self->data.ref)) != NULL)
    {
      Expr *expr;

      expr = st_var_get_value (p);

      return expr_eval (expr, inputs, outputs, variables, res, error);
    }

  g_set_error (error,
	       ST_ERROR,
	       ST_ERROR_INVALID_VALUE,
	       "%s: undefined identifier: %s", __FUNCTION__, self->data.ref);

  return FALSE;
}

static void
ref_expr_destroy (Expr * self)
{
  g_free (self->data.ref);
}

static gchar *
ref_expr_describe (Expr * self, GString * s)
{
  g_string_printf (s, "%s", self->data.ref);

  return s->str;
}

static gboolean
ref_expr_have_ref_to (Expr * self, const gchar * id, GHashTable * variables)
{
  gboolean r = FALSE;

  if (strcmp (self->data.ref, id) == 0)
    r = TRUE;
  else
    {
      gpointer p;

      if ((p = g_hash_table_lookup (variables, self->data.ref)) != NULL)
	r = expr_have_ref_to (st_var_get_value (p), id, variables);
    }

  return r;
}

Expr *
expr_new_ref (const gchar * ref)
{
  Expr *r;

  r = g_malloc0 (sizeof (Expr));

  r->type = EXPR_TYPE_REF;
  r->data.ref = g_strdup (ref);

  r->callbacks.eval = ref_expr_eval;
  r->callbacks.destroy = ref_expr_destroy;
  r->callbacks.describe = ref_expr_describe;
  r->callbacks.have_ref_to = ref_expr_have_ref_to;
  r->nref = 1;

  return r;
}

static void
func_expr_destroy (Expr * self)
{
  g_free (self->data.func.id);
  g_ptr_array_unref (self->data.func.args);
}

static gchar *
func_expr_describe (Expr * self, GString * s)
{
  g_autoptr (GString) args;
  g_autoptr (GString) arg;
  guint i;

  args = g_string_new ("");
  arg = g_string_new ("");

  for (i = 0; i < self->data.func.args->len; i++)
    g_string_append_printf (args, "%s%s", i == 0 ? "" : ", ",
			    expr_describe (self->data.func.args->pdata[i],
					   arg));

  g_string_printf (s, "%s(%s)", self->data.func.id, args->str);

  return s->str;
}

static gboolean
min_func_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		    GHashTable * variables, gdouble * res, GError ** error)
{
  gdouble r = G_MAXDOUBLE;
  guint i;

  for (i = 0; i < self->data.func.args->len; i++)
    {
      gdouble val;

      if (!expr_eval
	  (self->data.func.args->pdata[i], inputs, outputs, variables, &val,
	   error))
	return FALSE;

      if (val < r)
	r = val;
    }

  *res = r;

  return TRUE;
}

static gboolean
max_func_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		    GHashTable * variables, gdouble * res, GError ** error)
{
  gdouble r = G_MINDOUBLE;
  guint i;

  for (i = 0; i < self->data.func.args->len; i++)
    {
      gdouble val;

      if (!expr_eval
	  (self->data.func.args->pdata[i], inputs, outputs, variables, &val,
	   error))
	return FALSE;

      if (val > r)
	r = val;
    }

  *res = r;

  return TRUE;
}

static gboolean
round_func_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		      GHashTable * variables, gdouble * res, GError ** error)
{
  gdouble val;

  if (!expr_eval
      (self->data.func.args->pdata[0x00], inputs, outputs, variables, &val,
       error))
    return FALSE;

  *res = round (val);

  return TRUE;
}

static gboolean
limit_func_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		      GHashTable * variables, gdouble * res, GError ** error)
{
  gdouble val;
  gdouble min;
  gdouble max;

  if (!expr_eval
      (self->data.func.args->pdata[0x00], inputs, outputs, variables, &val,
       error)
      || !expr_eval (self->data.func.args->pdata[0x01], inputs, outputs,
		     variables, &min, error)
      || !expr_eval (self->data.func.args->pdata[0x02], inputs, outputs,
		     variables, &max, error))
    return FALSE;

  *res = val < min ? min : val > max ? max : val;

  return TRUE;
}

static gboolean
eval_func_expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		     GHashTable * variables, gdouble * res, GError ** error)
{
  Expr *arg;

  arg = self->data.func.args->pdata[0x00];

  if (arg->type == EXPR_TYPE_LITERAL)
    return expr_eval (arg, inputs, outputs, variables, res, error);
  else
    {
      gdouble xres;
      Expr *xarg;

      if (!expr_eval (arg, inputs, outputs, variables, &xres, error))
	return FALSE;

      xarg = expr_new_literal (xres);

      self->data.func.args->pdata[0x00] = xarg;

      expr_unref (arg);
    }
}

static gboolean
changed_func_expr_eval (Expr * self, GHashTable * inputs,
			GHashTable * outputs, GHashTable * variables,
			gdouble * res, GError ** error)
{
  Expr *arg0;
  Expr *arg1;
  gdouble val0;
  gdouble val1;

  arg0 = self->data.func.args->pdata[0x00];
  arg1 = self->data.func.args->pdata[0x01];

  if (!expr_eval (arg0, inputs, outputs, variables, &val0, error)
      || !expr_eval (arg1, inputs, outputs, variables, &val1, error))
    return FALSE;

  if (val1 == G_MINDOUBLE)
    *res = 1;
  else
    *res = val0 != val1;

  arg1->data.literal = val0;

  return TRUE;
}

static gboolean
func_expr_have_ref_to (Expr * self, const gchar * id, GHashTable * variables)
{
  guint i;

  for (i = 0; i < self->data.func.args->len; i++)
    {
      if (expr_have_ref_to (self->data.func.args->pdata[i], id, variables))
	return TRUE;
    }

  return FALSE;
}

static Expr *
expr_new_func (const char *id,
	       GPtrArray * args,
	       gboolean (*eval) (Expr *, GHashTable *, GHashTable *,
				 GHashTable *, gdouble *, GError **))
{
  Expr *r;

  r = g_malloc0 (sizeof (Expr));

  r->type = EXPR_TYPE_FUNC;
  r->data.func.id = g_strdup (id);
  r->data.func.args = g_ptr_array_ref (args);

  r->callbacks.eval = eval;
  r->callbacks.destroy = func_expr_destroy;
  r->callbacks.describe = func_expr_describe;
  r->callbacks.have_ref_to = func_expr_have_ref_to;
  r->nref = 1;

  return r;
}

Expr *
expr_new_func_min (GPtrArray * args)
{
  return expr_new_func ("MIN", args, min_func_expr_eval);
}

Expr *
expr_new_func_max (GPtrArray * args)
{
  return expr_new_func ("MAX", args, max_func_expr_eval);
}

Expr *
expr_new_func_round (GPtrArray * args)
{
  return expr_new_func ("ROUND", args, round_func_expr_eval);
}

Expr *
expr_new_func_limit (GPtrArray * args)
{
  return expr_new_func ("LIMIT", args, limit_func_expr_eval);
}

Expr *
expr_new_func_eval (GPtrArray * args)
{
  return expr_new_func ("EVAL", args, eval_func_expr_eval);
}

Expr *
expr_new_func_changed (GPtrArray * args)
{
  g_ptr_array_add (args, expr_new_literal (G_MINDOUBLE));

  return expr_new_func ("CHANGED", args, changed_func_expr_eval);
}

gdouble
expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
	   GHashTable * variables, gdouble * res, GError ** error)
{
  if (self->callbacks.eval)
    {
      gboolean stts;
      g_autoptr (GString) s;

      stts =
	(*self->callbacks.eval) (self, inputs, outputs, variables, res,
				 error);
      s = g_string_new ("");

      if (stts)
	VERBOSE_3_PRINTF ("Expression \"%s\" evaluated to %lf",
			  expr_describe (self, s), *res);

      return stts;
    }
  else
    {
      g_autoptr (GString) s = g_string_new ("");

      g_set_error (error,
		   ST_ERROR,
		   ST_ERROR_INVALID_VALUE,
		   "eval not implemented in expression: %s",
		   expr_describe (self, s));
    }

  return FALSE;
}

gchar *
expr_describe (Expr * self, GString * s)
{
  if (self->callbacks.describe)
    (*self->callbacks.describe) (self, s);
  else
    g_string_printf (s,
		     "Expr %p {type: %d, left: %p, right: %p, nref: %d}",
		     self, self->type, self->left, self->right, self->nref);

  return s->str;
}

gboolean
expr_have_ref_to (Expr * self, const gchar * id, GHashTable * variables)
{
  gboolean r = FALSE;

  if (self->callbacks.have_ref_to)
    r = (*self->callbacks.have_ref_to) (self, id, variables);

  return r;
}
