/*
 * libstarter-core - glib-compat.h
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

#ifndef STARTER_GLIB_COMPAT_H_INCLUDED
#define STARTER_GLIB_COMPAT_H_INCLUDED

#include <string.h>
#include <glib.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

G_BEGIN_DECLS

/*
 * g_memdup2(): since GLib 2.68.
 * Prefer a local implementation over g_memdup(): the latter takes guint
 * (not gsize) and is removed/hidden on newer GLib.
 */
#ifndef HAVE_G_MEMDUP2
static inline gpointer
g_memdup2 (gconstpointer mem, gsize byte_size)
{
  gpointer new_mem;

  if (mem != NULL && byte_size != 0)
    {
      new_mem = g_malloc (byte_size);
      memcpy (new_mem, mem, byte_size);
    }
  else
    new_mem = NULL;

  return new_mem;
}
#endif

/* g_ptr_array_copy(): since GLib 2.62 */
#ifndef HAVE_G_PTR_ARRAY_COPY
GPtrArray *st_g_ptr_array_copy (GPtrArray * array, GCopyFunc func,
				gpointer user_data);
#define g_ptr_array_copy st_g_ptr_array_copy
#endif

/* g_ptr_array_steal(): since GLib 2.64 */
#ifndef HAVE_G_PTR_ARRAY_STEAL
gpointer *st_g_ptr_array_steal (GPtrArray * array, gsize * len);
#define g_ptr_array_steal st_g_ptr_array_steal
#endif

/* g_string_replace(): since GLib 2.68 */
#ifndef HAVE_G_STRING_REPLACE
guint st_g_string_replace (GString * string, const gchar * find,
			   const gchar * replace, guint limit);
#define g_string_replace st_g_string_replace
#endif

G_END_DECLS

#endif /* STARTER_GLIB_COMPAT_H_INCLUDED */
