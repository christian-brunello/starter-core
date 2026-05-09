/*
 * libstarter-core - input.c
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <starter/error.h>
#include <starter/input.h>

#include "internals.h"

typedef struct
{
  gchar *name;
  gchar *descr;
  STUnit unit;
  gdouble val;
  gdouble min;
  gdouble max;
  gdouble step;
  guint64 flags;
} STInputPrivate;

enum
{
  ST_INPUT_SIGNAL_CHANGED,
  ST_INPUT_SIGNAL_NAME_CHANGED,
  ST_INPUT_SIGNAL_DESCR_CHANGED,
  ST_INPUT_SIGNAL_UNIT_CHANGED,
  ST_INPUT_SIGNAL_VAL_CHANGED,
  ST_INPUT_SIGNAL_MIN_CHANGED,
  ST_INPUT_SIGNAL_MAX_CHANGED,
  ST_INPUT_SIGNAL_STEP_CHANGED,
  ST_INPUT_SIGNAL_FLAGS_CHANGED,
  ST_INPUT_SIGNAL_COUNT
};

static gint st_input_signals[ST_INPUT_SIGNAL_COUNT];

G_DEFINE_TYPE_WITH_PRIVATE (STInput, st_input, G_TYPE_OBJECT)
#define ST_INPUT_GET_PRIVATE(obj) \
    ((STInputPrivate *) st_input_get_instance_private (ST_INPUT (obj)))
     static gboolean ismul (double a, double b)
{
  gboolean r = FALSE;
  gint xa = (a * 1000000);
  gint xb = (b * 1000000);

  if (xb == 0)
    {
      if (xa == 0)
	r = TRUE;
    }
  else
    {
      if (xa % xb == 0)
	r = TRUE;
    }

  return r;
}

static void
st_input_emit_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_CHANGED], 0);
}

static void
st_input_emit_name_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_NAME_CHANGED], 0);
}

static void
st_input_emit_descr_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_DESCR_CHANGED], 0);
}

static void
st_input_emit_unit_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_UNIT_CHANGED], 0);
}

static void
st_input_emit_val_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_VAL_CHANGED], 0);
}

static void
st_input_emit_min_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_MIN_CHANGED], 0);
}

static void
st_input_emit_max_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_MAX_CHANGED], 0);
}

static void
st_input_emit_step_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_STEP_CHANGED], 0);
}

static void
st_input_emit_flags_changed_signal (STInput * self)
{
  g_signal_emit (self, st_input_signals[ST_INPUT_SIGNAL_FLAGS_CHANGED], 0);
}

static void
st_input_finalize (GObject * object)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (object);

  LOGD ("finalize STInput %p", object);

  g_free (priv->name);
  g_free (priv->descr);

  G_OBJECT_CLASS (st_input_parent_class)->finalize (object);
}

static void
st_input_class_init (STInputClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = st_input_finalize;

  st_input_signals[ST_INPUT_SIGNAL_CHANGED] = g_signal_new ("changed",
							    G_TYPE_FROM_CLASS
							    (klass),
							    G_SIGNAL_RUN_LAST,
							    0, NULL, NULL,
							    NULL, G_TYPE_NONE,
							    0);

  st_input_signals[ST_INPUT_SIGNAL_NAME_CHANGED] =
    g_signal_new ("name-changed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  st_input_signals[ST_INPUT_SIGNAL_DESCR_CHANGED] =
    g_signal_new ("descr-changed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  st_input_signals[ST_INPUT_SIGNAL_UNIT_CHANGED] =
    g_signal_new ("unit-changed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  st_input_signals[ST_INPUT_SIGNAL_VAL_CHANGED] = g_signal_new ("val-changed",
								G_TYPE_FROM_CLASS
								(klass),
								G_SIGNAL_RUN_LAST,
								0, NULL, NULL,
								NULL,
								G_TYPE_NONE,
								0);

  st_input_signals[ST_INPUT_SIGNAL_MIN_CHANGED] = g_signal_new ("min-changed",
								G_TYPE_FROM_CLASS
								(klass),
								G_SIGNAL_RUN_LAST,
								0, NULL, NULL,
								NULL,
								G_TYPE_NONE,
								0);

  st_input_signals[ST_INPUT_SIGNAL_MAX_CHANGED] = g_signal_new ("max-changed",
								G_TYPE_FROM_CLASS
								(klass),
								G_SIGNAL_RUN_LAST,
								0, NULL, NULL,
								NULL,
								G_TYPE_NONE,
								0);

  st_input_signals[ST_INPUT_SIGNAL_STEP_CHANGED] =
    g_signal_new ("step-changed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  st_input_signals[ST_INPUT_SIGNAL_FLAGS_CHANGED] =
    g_signal_new ("flags-changed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
st_input_init (STInput * self)
{

}

STInput *
st_input_new (const gchar * name, const gchar * description, STUnit unit,
	      gdouble min, gdouble max, gdouble step, gdouble val,
	      guint64 flags)
{
  STInput *r;
  STInputPrivate *priv;

  g_assert (name != NULL);
  g_assert (min <= val && ismul (min, step));
  g_assert (max >= val && ismul (max, step));
  g_assert (ismul (val, step));

  r = g_object_new (ST_TYPE_INPUT, NULL);
  priv = ST_INPUT_GET_PRIVATE (r);

  priv->name = g_strdup (name);
  priv->descr = g_strdup (description);
  priv->unit = unit;
  priv->min = min;
  priv->max = max;
  priv->step = step;
  priv->val = val;
  priv->flags = flags;

  return r;
}

STInput *
st_input_dup (const STInput * self)
{
  STInput *r;
  STInputPrivate *priv;
  STInputPrivate *xpriv;

  r = g_object_new (ST_TYPE_INPUT, NULL);
  priv = ST_INPUT_GET_PRIVATE (r);
  xpriv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  priv->name = g_strdup (xpriv->name);
  priv->descr = g_strdup (xpriv->descr);
  priv->unit = xpriv->unit;
  priv->min = xpriv->min;
  priv->max = xpriv->max;
  priv->step = xpriv->step;
  priv->val = xpriv->val;
  priv->flags = xpriv->flags;

  return r;
}

const char *
st_input_get_name (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->name;
}

char *
st_input_dup_name (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return g_strdup (priv->name);
}

gboolean
st_input_set_name (STInput * self, const char *name, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean chg = FALSE;

  if (strcmp (priv->name, name) != 0)
    chg = TRUE;

  g_free (priv->name);

  priv->name = g_strdup (name);

  if (chg)
    {
      st_input_emit_name_changed_signal (self);
      st_input_emit_changed_signal (self);
    }

  return TRUE;
}

const char *
st_input_get_description (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->descr;
}

char *
st_input_dup_description (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return g_strdup (priv->descr);
}

gboolean
st_input_set_description (STInput * self, const char *description,
			  GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean chg = FALSE;

  if (strcmp (priv->descr, description) != 0)
    chg = TRUE;

  g_free (priv->descr);

  priv->descr = g_strdup (description);

  if (chg)
    {
      st_input_emit_descr_changed_signal (self);
      st_input_emit_changed_signal (self);
    }

  return TRUE;
}

STUnit
st_input_get_unit (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->unit;
}

gboolean
st_input_set_unit (STInput * self, STUnit unit, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean chg = FALSE;

  if (priv->unit != unit)
    chg = TRUE;

  priv->unit = unit;

  if (chg)
    {
      st_input_emit_unit_changed_signal (self);
      st_input_emit_changed_signal (self);
    }

  return TRUE;
}

gdouble
st_input_get_min (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->min;
}

gboolean
st_input_set_min (STInput * self, gdouble min, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean r = FALSE;

  if (min <= priv->val && ismul (min, priv->step))
    {
      gboolean chg = FALSE;

      if (priv->min != min)
	chg = TRUE;

      priv->min = min;
      r = TRUE;

      if (chg)
	{
	  st_input_emit_min_changed_signal (self);
	  st_input_emit_changed_signal (self);
	}
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE,
		 "minimum value is not compatible with val/step");

  return r;
}

gdouble
st_input_get_max (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->max;
}

gboolean
st_input_set_max (STInput * self, gdouble max, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean r = FALSE;

  if (max >= priv->val && ismul (max, priv->step))
    {
      gboolean chg = FALSE;

      if (priv->max != max)
	chg = TRUE;

      priv->max = max;
      r = TRUE;

      if (chg)
	{
	  st_input_emit_max_changed_signal (self);
	  st_input_emit_changed_signal (self);
	}
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE,
		 "maximum value is not compatible with val/step");

  return r;
}

gdouble
st_input_get_step (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->step;
}

gboolean
st_input_set_step (STInput * self, gdouble step, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean r = FALSE;

  if (ismul (priv->min, step) && ismul (priv->max, step)
      && ismul (priv->val, step))
    {
      gboolean chg = FALSE;

      if (priv->step != step)
	chg = TRUE;

      priv->step = step;
      r = TRUE;

      if (chg)
	{
	  st_input_emit_step_changed_signal (self);
	  st_input_emit_changed_signal (self);
	}
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE,
		 "input min/max/val not divisible by step");

  return r;
}

gdouble
st_input_get_val (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->val;
}

gboolean
st_input_set_val (STInput * self, gdouble val, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean r = FALSE;

  if (val >= priv->min && val <= priv->max && ismul (val, priv->step))
    {
      gboolean chg = FALSE;

      if (priv->val != val)
	chg = TRUE;

      priv->val = val;
      r = TRUE;

      if (chg)
	{
	  st_input_emit_val_changed_signal (self);
	  st_input_emit_changed_signal (self);
	}
    }
  else
    g_set_error (error, ST_ERROR, ST_ERROR_INVALID_VALUE,
		 "val is not compatible with min/max/step");

  return r;
}

guint64
st_input_get_flags (const STInput * self)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE ((STInput *) self);

  return priv->flags;
}

gboolean
st_input_set_flags (STInput * self, guint64 val, GError ** error)
{
  STInputPrivate *priv = ST_INPUT_GET_PRIVATE (self);
  gboolean r = FALSE;
  gboolean chg = FALSE;

  if (priv->flags != val)
    chg = TRUE;

  priv->flags = val;
  r = TRUE;

  if (chg)
    {
      st_input_emit_flags_changed_signal (self);
      st_input_emit_changed_signal (self);
    }

  return r;
}
