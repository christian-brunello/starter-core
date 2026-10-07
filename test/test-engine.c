/*
 * starter-core - test-engine.c
 *
 * Tests for the rules engine (variables / triggers).
 */

#include <glib.h>
#include <starter/error.h>

#include "internals.h"

/* Provided by main.c in the daemon; define here for the test binary. */
VerboseLevel verbose_level = VERBOSE_LEVEL_NULL;

typedef struct
{
  Engine *engine;
  guint var_changed;
} Fixture;

static void
on_var_changed (STVar * var, gpointer user_data)
{
  Fixture *f = user_data;

  (void) var;
  f->var_changed++;
}

static void
fixture_setup (Fixture * f, gconstpointer user_data)
{
  (void) user_data;

  f->engine =
    engine_new (g_ptr_array_new_full (0, (GDestroyNotify) lable_block_delete),
		g_ptr_array_new_full (0, (GDestroyNotify) trigger_delete));
  f->var_changed = 0;
}

static void
fixture_teardown (Fixture * f, gconstpointer user_data)
{
  (void) user_data;

  engine_delete (f->engine);
}

static STVar *
add_var (Fixture * f, const gchar * name, gdouble lit)
{
  Expr *e = expr_new_literal (lit);
  STVar *v = st_var_new (name, e);

  expr_unref (e);
  g_hash_table_insert (f->engine->variables, g_strdup (name), v);
  g_signal_connect (v, "changed", G_CALLBACK (on_var_changed), f);

  return v;
}

static void
test_var_set_skips_equal_expr (Fixture * f, gconstpointer user_data)
{
  STVar *v;
  Expr *a;
  Expr *b;

  (void) user_data;

  v = add_var (f, "x", 0);
  a = expr_new_literal (1);
  b = expr_new_literal (1);

  g_assert_true (st_var_set_value (v, a));
  g_assert_cmpuint (f->var_changed, ==, 1);

  /* Structurally equal, different object — must not emit again. */
  g_assert_false (st_var_set_value (v, b));
  g_assert_cmpuint (f->var_changed, ==, 1);

  expr_unref (a);
  expr_unref (b);
}

static void
test_trigger_continues_after_failed_assign (Fixture * f,
					    gconstpointer user_data)
{
  GPtrArray *ass;
  Trigger *tr;
  GError *error = NULL;
  GHashTable *empty_io;
  STVar *v_ok;
  Expr *cond;
  Expr *lit_fail;
  Expr *lit_ok;
  gdouble res = -1;

  (void) user_data;

  g_test_expect_message (ST_CORE_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			 "*trigger assign failed*");

  v_ok = add_var (f, "ok", 0);
  cond = expr_new_literal (1);
  lit_fail = expr_new_literal (9);
  lit_ok = expr_new_literal (42);

  ass = g_ptr_array_new_full (0, (GDestroyNotify) assign_delete);
  g_ptr_array_add (ass, assign_new ("missing_target", lit_fail));
  g_ptr_array_add (ass, assign_new ("ok", lit_ok));

  tr = trigger_new (cond, ass);
  expr_unref (cond);
  expr_unref (lit_fail);
  expr_unref (lit_ok);

  empty_io =
    g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_assert_false (trigger_apply
		  (tr, empty_io, empty_io, f->engine->variables, &error));
  g_assert_error (error, ST_ERROR, ST_ERROR_UNDEFINED_IDENTIFIER);
  g_clear_error (&error);

  g_test_assert_expected_messages ();

  g_assert_true (expr_eval
		 (st_var_get_value (v_ok), empty_io, empty_io,
		  f->engine->variables, &res, &error));
  g_assert_no_error (error);
  g_assert_cmpfloat (res, ==, 42.0);

  trigger_delete (tr);
  g_hash_table_unref (empty_io);
}

static void
test_expr_equal_structural (void)
{
  Expr *a = expr_new_op ("+", expr_new_literal (1), expr_new_ref ("TIME:hour"));
  Expr *b = expr_new_op ("+", expr_new_literal (1), expr_new_ref ("TIME:hour"));
  Expr *c = expr_new_op ("+", expr_new_literal (2), expr_new_ref ("TIME:hour"));

  g_assert_true (expr_equal (a, b));
  g_assert_false (expr_equal (a, c));

  expr_unref (a);
  expr_unref (b);
  expr_unref (c);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add ("/starter-core/engine/var-set-skips-equal", Fixture, NULL,
	      fixture_setup, test_var_set_skips_equal_expr, fixture_teardown);
  g_test_add ("/starter-core/engine/trigger-continues-on-fail", Fixture, NULL,
	      fixture_setup, test_trigger_continues_after_failed_assign,
	      fixture_teardown);
  g_test_add_func ("/starter-core/engine/expr-equal",
		   test_expr_equal_structural);

  return g_test_run ();
}
