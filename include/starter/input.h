/*
 * libstarter-core - input.h
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

#ifndef STARTER_INPUT_H_INCLUDED
#define STARTER_INPUT_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include <starter/defs.h>

G_BEGIN_DECLS
#define ST_TYPE_INPUT (st_input_get_type ())
G_DECLARE_DERIVABLE_TYPE (STInput, st_input, ST, INPUT, GObject)
     struct _STInputClass
     {
       GObjectClass parent_class;
     };

     STInput *st_input_new (const gchar * name, const gchar * description,
			    STUnit unit, gdouble min, gdouble max,
			    gdouble step, gdouble val, guint64 flags);
     STInput *st_input_dup (const STInput * self);

     const char *st_input_get_name (const STInput * self);
     char *st_input_dup_name (const STInput * self);
     gboolean st_input_set_name (STInput * self, const char *name,
				 GError ** error);

     const char *st_input_get_description (const STInput * self);
     char *st_input_dup_description (const STInput * self);
     gboolean st_input_set_description (STInput * self,
					const char *description,
					GError ** error);

     STUnit st_input_get_unit (const STInput * self);
     gboolean st_input_set_unit (STInput * self, STUnit unit,
				 GError ** error);

     gdouble st_input_get_min (const STInput * self);
     gboolean st_input_set_min (STInput * self, gdouble min, GError ** error);

     gdouble st_input_get_max (const STInput * self);
     gboolean st_input_set_max (STInput * self, gdouble max, GError ** error);

     gdouble st_input_get_step (const STInput * self);
     gboolean st_input_set_step (STInput * self, gdouble step,
				 GError ** error);

     gdouble st_input_get_val (const STInput * self);
     gboolean st_input_set_val (STInput * self, gdouble val, GError ** error);

     guint64 st_input_get_flags (const STInput * self);
     gboolean st_input_set_flags (STInput * self, guint64 val,
				  GError ** error);

G_END_DECLS
#endif /* STARTER_INPUT_H_INCLUDED */
