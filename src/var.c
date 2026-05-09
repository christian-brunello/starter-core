/*
 * starter-core - var.c
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

#include "internals.h"

struct _STVar
{
  GObject parent_instance;
  gchar *name;
  Expr *value;
};

enum
{
  ST_VAR_SIGNAL_CHANGED,
  ST_VAR_SIGNAL_COUNT
};

G_DEFINE_TYPE (STVar, st_var, G_TYPE_OBJECT)
     static gint st_var_signals[ST_VAR_SIGNAL_COUNT];

     static void st_var_finalize (GObject * gobject)
{
  STVar *self = ST_VAR (gobject);

  LOGD ("finalize STVar %p", gobject);

  g_free (self->name);

  // Do not touch expr, it's owned by engine

  // chain up to the parent class
  G_OBJECT_CLASS (st_var_parent_class)->finalize (gobject);
}

static void
st_var_class_init (STVarClass * klass)
{
  GObjectClass *self_class = G_OBJECT_CLASS (klass);

  st_var_signals[ST_VAR_SIGNAL_CHANGED] = g_signal_new ("changed",
							G_TYPE_FROM_CLASS
							(klass),
							G_SIGNAL_RUN_LAST, 0,
							NULL, NULL, NULL,
							G_TYPE_NONE, 0);

  self_class->finalize = st_var_finalize;
}

static void
st_var_init (STVar * self)
{

}



STVar *
st_var_new (const gchar * name, Expr * value)
{
  STVar *r;

  r = g_object_new (ST_TYPE_VAR, NULL);

  r->name = g_strdup (name);
  r->value = expr_ref (value);

  return r;
}

const gchar *
st_var_get_name (const STVar * self)
{
  return self->name;
}

Expr *
st_var_get_value (const STVar * self)
{
  return self->value;
}

void
st_var_set_value (STVar * self, Expr * value)
{
  gboolean changed = self->value != value;

  expr_unref (self->value);

  self->value = expr_ref (value);

  if (changed)
    g_signal_emit (self, st_var_signals[ST_VAR_SIGNAL_CHANGED], 0);
}
