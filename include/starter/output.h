/*
 * libstarter-core - output.h
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

#ifndef STARTER_OUTPUT_H_INCLUDED
#define STARTER_OUTPUT_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include <starter/defs.h>

G_BEGIN_DECLS
#define ST_TYPE_OUTPUT (st_output_get_type ())
G_DECLARE_DERIVABLE_TYPE (STOutput, st_output, ST, OUTPUT, GObject)
     struct _STOutputClass
     {
       GObjectClass parent_class;
     };

     STOutput *st_output_new (const gchar * name, const gchar * description,
			      STUnit unit, gdouble min, gdouble max,
			      gdouble step, gdouble val, guint64 flags);
     STOutput *st_output_dup (const STOutput * self);

     const char *st_output_get_name (const STOutput * self);
     char *st_output_dup_name (const STOutput * self);
     gboolean st_output_set_name (STOutput * self, const char *name,
				  GError ** error);

     const char *st_output_get_description (const STOutput * self);
     char *st_output_dup_description (const STOutput * self);
     gboolean st_output_set_description (STOutput * self,
					 const char *description,
					 GError ** error);

     STUnit st_output_get_unit (const STOutput * self);
     gboolean st_output_set_unit (STOutput * self, STUnit unit,
				  GError ** error);

     gdouble st_output_get_min (const STOutput * self);
     gboolean st_output_set_min (STOutput * self, gdouble min,
				 GError ** error);

     gdouble st_output_get_max (const STOutput * self);
     gboolean st_output_set_max (STOutput * self, gdouble max,
				 GError ** error);

     gdouble st_output_get_step (const STOutput * self);
     gboolean st_output_set_step (STOutput * self, gdouble step,
				  GError ** error);

     gdouble st_output_get_val (const STOutput * self);
     gboolean st_output_set_val (STOutput * self, gdouble val,
				 GError ** error);

     guint64 st_output_get_flags (const STOutput * self);
     gboolean st_output_set_flags (STOutput * self, guint64 val,
				   GError ** error);
G_END_DECLS
#endif /* STARTER_OUTPUT_H_INCLUDED */
