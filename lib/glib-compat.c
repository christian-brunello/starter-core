/*
 * libstarter-core - glib-compat.c
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

#include "glib-compat.h"

#if !defined(HAVE_G_PTR_ARRAY_COPY) || !defined(HAVE_G_PTR_ARRAY_STEAL)
/*
 * Layout of GLib's internal GRealPtrArray for 2.44–2.62 (fields we touch
 * match upstream garray.c). Only used when the symbols are missing.
 */
typedef struct
{
  gpointer *pdata;
  guint len;
  guint alloc;
  guint ref_count;
  GDestroyNotify element_free_func;
} STRealPtrArray;
#endif

#ifndef HAVE_G_PTR_ARRAY_COPY
GPtrArray *
st_g_ptr_array_copy (GPtrArray * array, GCopyFunc func, gpointer user_data)
{
  GPtrArray *new_array;
  STRealPtrArray *rarray;
  guint i;

  g_return_val_if_fail (array != NULL, NULL);

  rarray = (STRealPtrArray *) array;
  new_array = g_ptr_array_sized_new (array->len);
  g_ptr_array_set_free_func (new_array, rarray->element_free_func);

  if (func != NULL)
    {
      for (i = 0; i < array->len; i++)
	g_ptr_array_add (new_array, func (array->pdata[i], user_data));
    }
  else
    {
      for (i = 0; i < array->len; i++)
	g_ptr_array_add (new_array, array->pdata[i]);
    }

  return new_array;
}
#endif

#ifndef HAVE_G_PTR_ARRAY_STEAL
gpointer *
st_g_ptr_array_steal (GPtrArray * array, gsize * len)
{
  STRealPtrArray *rarray;
  gpointer *segment;

  g_return_val_if_fail (array != NULL, NULL);

  rarray = (STRealPtrArray *) array;
  segment = rarray->pdata;

  if (len != NULL)
    *len = rarray->len;

  rarray->pdata = NULL;
  rarray->len = 0;
  rarray->alloc = 0;

  return segment;
}
#endif

#ifndef HAVE_G_STRING_REPLACE
guint
st_g_string_replace (GString * string, const gchar * find,
		     const gchar * replace, guint limit)
{
  gsize f_len, r_len, pos;
  gchar *cur, *next;
  guint n = 0;

  g_return_val_if_fail (string != NULL, 0);
  g_return_val_if_fail (find != NULL, 0);
  g_return_val_if_fail (replace != NULL, 0);

  f_len = strlen (find);
  r_len = strlen (replace);
  cur = string->str;

  if (f_len == 0)
    return 0;

  while ((next = strstr (cur, find)) != NULL)
    {
      pos = next - string->str;
      g_string_erase (string, pos, f_len);
      g_string_insert (string, pos, replace);
      cur = string->str + pos + r_len;
      n++;
      if (limit != 0 && n == limit)
	break;
    }

  return n;
}
#endif
