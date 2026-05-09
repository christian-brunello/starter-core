/*
 * starter-core - engine.c
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

#include <stdarg.h>
#include <stdio.h>

#include <starter/error.h>

#include "internals.h"

extern VerboseLevel verbose_level;

void
verbose_printf (VerboseLevel vl, const char *fmt, ...)
{
  if (vl <= verbose_level)
    {
      va_list va;

      va_start (va, fmt);

      fprintf (stderr, "%s: ", PACKAGE);
      vfprintf (stderr, fmt, va);
      fputc ('\n', stderr);
    }

}

void
assign_delete (Assign * self)
{
  g_free (self->id);
  expr_unref (self->rval);

  g_free (self);
}

Assign *
assign_new (const gchar * id, Expr * rval)
{
  Assign *r;

  r = g_malloc0 (sizeof (Assign));

  r->id = g_strdup (id);
  r->rval = expr_ref (rval);

  return r;
}

gboolean
assign_apply (Assign * self, GHashTable * inputs, GHashTable * outputs,
	      GHashTable * variables, GError ** error)
{
  g_autoptr (GString) s;
  gpointer p;

  s = g_string_new ("");
  assign_describe (self, s);

  if ((p = g_hash_table_lookup (outputs, self->id)) != NULL)
    {
      /* Reference to an output, call st_client_set_output */
      gdouble expr_res;
      ClientOutputMap *m = p;
      const GPtrArray *coutputs;
      gdouble val;
      guint64 flags;

      VERBOSE_3_PRINTF ("Target of %s is an output", s->str);

      if (!expr_eval
	  (self->rval, inputs, outputs, variables, &expr_res, error))
	return FALSE;

      coutputs = st_client_get_outputs (m->client);
      val = st_output_get_val (coutputs->pdata[m->index]);
      flags = st_output_get_flags (coutputs->pdata[m->index]);

      VERBOSE_3_PRINTF ("Current output %s value: %lf, flags: %lu", self->id,
			val, flags);

      if (val == expr_res)
	{
	  VERBOSE_2_PRINTF
	    ("Avoid call `st_client_set_output' because value is not changed");
	  return TRUE;
	}
      else if (flags & ST_IO_FLAG_MANUAL_OVERRIDE)
	{
	  VERBOSE_2_PRINTF
	    ("Avoid call `st_client_set_output' because of ST_IO_FLAG_MANUAL_OVERRIDE");
	  return TRUE;
	}

      VERBOSE_3_PRINTF
	("Call st_client_set_output with output: %s, value: %lf, flags: %llu",
	 self->id, expr_res, flags);

      if (!st_client_set_output (m->client, self->id, expr_res, flags))
	{
	  g_set_error (error,
		       ST_ERROR,
		       ST_ERROR_INVALID_VALUE,
		       "error in st_client_set_output with output: %s",
		       self->id);

	  return FALSE;
	}

      VERBOSE_3_PRINTF ("Output %s assigned successfully", self->id);

      return TRUE;
    }

  if ((p = g_hash_table_lookup (variables, self->id)) != NULL)
    {
      /* Reference to a variable, assign expression */

      VERBOSE_3_PRINTF ("Target of %s is a variable, assign expression",
			s->str);

      st_var_set_value (p, self->rval);

      return TRUE;
    }

  g_set_error (error,
	       ST_ERROR,
	       ST_ERROR_INVALID_VALUE,
	       "cannot lookup identrifier %s", self->id);

  return FALSE;
}

gchar *
assign_describe (Assign * self, GString * s)
{
  g_autoptr (GString) es = g_string_new ("");

  g_string_printf (s,
		   "Assign %p {id: %s, rval: %s}",
		   self, self->id, expr_describe (self->rval, es));

  return s->str;
}

gboolean
assign_have_ref_to (Assign * self, const gchar * id, GHashTable * variables)
{
  return expr_have_ref_to (self->rval, id, variables);
}

void
label_entry_delete (LabelEntry * self)
{
  expr_unref (self->cond);
  assign_delete (self->ass);

  g_free (self);
}

LabelEntry *
label_entry_new (Expr * cond, Assign * ass)
{
  LabelEntry *r;

  r = g_malloc0 (sizeof (LabelEntry));

  r->cond = expr_ref (cond);
  r->ass = ass;

  return r;
}

gchar *
label_entry_describe (LabelEntry * self, GString * s)
{
  g_autoptr (GString) cs;
  g_autoptr (GString) as;

  cs = g_string_new ("");
  as = g_string_new ("");

  g_string_printf (s,
		   "LabelEntry %p {cond: %s, ass: %s}",
		   self,
		   expr_describe (self->cond, cs),
		   assign_describe (self->ass, as));

  return s->str;
}

gboolean
label_entry_have_ref_to (LabelEntry * self, const gchar * id,
			 GHashTable * variables)
{
  return expr_have_ref_to (self->cond, id, variables)
    || assign_have_ref_to (self->ass, id, variables);
}

void
lable_block_delete (LabelBlock * self)
{
  g_ptr_array_unref (self->entries);
  g_free (self);
}

LabelBlock *
label_block_new (GPtrArray * entries)
{
  LabelBlock *r;

  r = g_malloc0 (sizeof (LabelBlock));

  r->entries = entries;

  return r;
}

gchar *
label_block_describe (LabelBlock * self, GString * s)
{
  g_autoptr (GString) es;
  g_autoptr (GString) ts;
  guint i;

  es = g_string_new ("");
  ts = g_string_new ("");

  for (i = 0; i < self->entries->len; i++)
    g_string_append_printf (es,
			    "%s%s",
			    i == 0 ? "" : ", ",
			    label_entry_describe (self->entries->pdata[i],
						  ts));

  g_string_printf (s, "LabelBlock %p {entries: [%s]}", self, es->str);

  return s->str;
}

gboolean
label_block_apply (LabelBlock * self, GHashTable * inputs,
		   GHashTable * outputs, GHashTable * variables,
		   GError ** error)
{
  g_autoptr (GString) s;
  g_autoptr (GString) s2;
  guint i;

  s = g_string_new ("");
  s2 = g_string_new ("");

  VERBOSE_3_PRINTF ("Apply %s", label_block_describe (self, s));

  for (i = 0; i < self->entries->len; i++)
    {
      LabelEntry *entry = self->entries->pdata[i];
      gdouble cond_res;

      VERBOSE_3_PRINTF ("Process %s", label_entry_describe (entry, s));

      if (!expr_eval
	  (entry->cond, inputs, outputs, variables, &cond_res, error))
	return FALSE;

      if (cond_res)
	{
	  VERBOSE_1_PRINTF
	    ("Condition \"%s\" evals to TRUE, apply assignment \"%s\"",
	     expr_describe (entry->cond, s), assign_describe (entry->ass,
							      s2));

	  return assign_apply (entry->ass, inputs, outputs, variables, error);
	}
      else
	VERBOSE_1_PRINTF ("Condition \"%s\" evals to FALSE",
			  expr_describe (entry->cond, s));
    }

  VERBOSE_3_PRINTF ("Label block completed");

  return TRUE;
}

gboolean
label_block_have_ref_to (LabelBlock * self, const gchar * id,
			 GHashTable * variables)
{
  gboolean r = FALSE;
  guint i;

  for (i = 0; i < self->entries->len; i++)
    if (r = label_entry_have_ref_to (self->entries->pdata[i], id, variables))
      break;

  return r;
}

void
trigger_delete (Trigger * self)
{
  expr_unref (self->cond);
  g_ptr_array_unref (self->ass);
}

Trigger *
trigger_new (Expr * cond, GPtrArray * ass)
{
  Trigger *r;

  r = g_malloc0 (sizeof (Trigger));

  r->cond = expr_ref (cond);
  r->ass = ass;

  return r;
}

gchar *
trigger_describe (Trigger * self, GString * s)
{
  g_autoptr (GString) cs;
  g_autoptr (GString) as;
  g_autoptr (GString) ts;
  guint i;

  cs = g_string_new ("");
  as = g_string_new ("");
  ts = g_string_new ("");

  if (self->ass)
    for (i = 0; i < self->ass->len; i++)
      g_string_append_printf (as,
			      "%s%s",
			      i == 0 ? "" : ", ",
			      assign_describe (self->ass->pdata[i], ts));

  g_string_printf (s, "Trigger %p {cond: %s, ass: [%s]}", self,
		   expr_describe (self->cond, cs), as->str);

  return s->str;
}

gboolean
trigger_apply (Trigger * self, GHashTable * inputs, GHashTable * outputs,
	       GHashTable * variables, GError ** error)
{
  g_autoptr (GString) s;
  g_autoptr (GString) s2;
  gdouble cond_res;

  s = g_string_new ("");
  s2 = g_string_new ("");

  VERBOSE_3_PRINTF ("Apply %s", trigger_describe (self, s));

  if (!expr_eval (self->cond, inputs, outputs, variables, &cond_res, error))
    return FALSE;

  if (cond_res)
    {
      guint i;

      VERBOSE_3_PRINTF ("Condition evals to TRUE, apply assignments");

      for (i = 0; i < self->ass->len; i++)
	{
	  VERBOSE_1_PRINTF
	    ("Condition \"%s\" evals to TRUE, apply assignment \"%s\"",
	     expr_describe (self->cond, s),
	     assign_describe (self->ass->pdata[i], s2));

	  if (!assign_apply
	      (self->ass->pdata[i], inputs, outputs, variables, error))
	    return FALSE;
	}
    }
  else
    VERBOSE_1_PRINTF ("Condition \"%s\" evals to FALSE",
		      expr_describe (self->cond, s));

  return TRUE;
}

gboolean
trigger_have_ref_to (Trigger * self, const gchar * id, GHashTable * variables)
{
  gboolean r = FALSE;

  if (expr_have_ref_to (self->cond, id, variables))
    r = TRUE;
  else
    {
      guint i;

      for (i = 0; i < self->ass->len; i++)
	if (r = assign_have_ref_to (self->ass->pdata[i], id, variables))
	  break;
    }

  return r;
}

void
engine_delete (Engine * self)
{
  g_ptr_array_unref (self->labels);
  g_ptr_array_unref (self->triggers);

  g_free (self);
}

Engine *
engine_new (GPtrArray * labels, GPtrArray * triggers)
{
  Engine *r;

  r = g_malloc0 (sizeof (Engine));

  r->labels = labels;
  r->triggers = triggers;
  r->variables =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

  return r;
}

static void
foreach_describe_variable (gpointer key, gpointer value, gpointer user_data)
{
  g_autoptr (GString) es;
  g_autoptr (GString) vs;
  GString *s = user_data;

  es = g_string_new ("");
  vs = g_string_new ("");

  g_string_printf (es, "STVar %p {name: %s, value: %s}", value,
		   st_var_get_name (value),
		   expr_describe (st_var_get_value (value), vs));

  g_string_append_printf (user_data,
			  "%s%s", s->len == 0 ? "" : ", ", es->str);
}

gchar *
engine_describe (Engine * self, GString * s)
{
  g_autoptr (GString) ls;
  g_autoptr (GString) ts;
  g_autoptr (GString) vs;
  g_autoptr (GString) xs;
  guint i;

  ls = g_string_new ("");
  ts = g_string_new ("");
  vs = g_string_new ("");
  xs = g_string_new ("");

  for (i = 0; i < self->labels->len; i++)
    g_string_append_printf (ls,
			    "%s%s",
			    i == 0 ? "" : ", ",
			    label_block_describe (self->labels->pdata[i],
						  xs));

  for (i = 0; i < self->triggers->len; i++)
    g_string_append_printf (ts,
			    "%s%s",
			    i == 0 ? "" : ", ",
			    trigger_describe (self->triggers->pdata[i], xs));

  g_hash_table_foreach (self->variables, foreach_describe_variable, vs);

  g_string_printf (s,
		   "Engine %p {labels: [%s], triggers: [%s], variables: [%s]}",
		   self, ls->str, ts->str, vs->str);

  return s->str;
}

gboolean
engine_apply (Engine * self, GHashTable * inputs, GHashTable * outputs,
	      const gchar * id, GError ** error)
{
  g_autoptr (GString) s;
  guint i;

  s = g_string_new ("");



  for (i = 0; i < self->labels->len; i++)
    {
      GError *xerror = NULL;

      if (id == NULL
	  || label_block_have_ref_to (self->labels->pdata[i], id,
				      self->variables))
	{
	  if (id)
	    VERBOSE_1_PRINTF ("Iterate label block because of \"%s\"", id);
	  else
	    VERBOSE_1_PRINTF
	      ("Iterate label block because of startup, connection or disconnection event");

	  if (!label_block_apply
	      (self->labels->pdata[i], inputs, outputs, self->variables,
	       &xerror))
	    {
	      fprintf (stderr, "%s: ** ERROR ** apply %s: %s\n", PACKAGE,
		       label_block_describe (self->labels->pdata[i], s),
		       xerror ? xerror->message : "unknown error");
	      g_error_free (xerror);
	    }
	}
      else
	VERBOSE_3_PRINTF
	  ("skip label block %s because does not have references to %s",
	   label_block_describe (self->labels->pdata[i], s), id);
    }

  for (i = 0; i < self->triggers->len; i++)
    {
      GError *xerror = NULL;

      if (id == NULL
	  || trigger_have_ref_to (self->triggers->pdata[i], id,
				  self->variables))
	{
	  if (id)
	    VERBOSE_1_PRINTF ("Iterate trigger because of \"%s\"", id);
	  else
	    VERBOSE_1_PRINTF
	      ("Iterate trigger because of startup, connection or disconnection event");

	  if (!trigger_apply
	      (self->triggers->pdata[i], inputs, outputs, self->variables,
	       &xerror))
	    {
	      fprintf (stderr, "%s: ** ERROR ** apply %s: %s\n", PACKAGE,
		       trigger_describe (self->triggers->pdata[i], s),
		       xerror ? xerror->message : "unknown error");
	      g_error_free (xerror);
	    }
	}
      else
	VERBOSE_3_PRINTF
	  ("skip trigger %s because does not have references to %s",
	   trigger_describe (self->triggers->pdata[i], s), id);
    }

  return TRUE;
}
