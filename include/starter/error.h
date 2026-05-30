/*
 * libstarter-core - error.h
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

#ifndef STARTER_ERROR_H_INCLUDED
#define STARTER_ERROR_H_INCLUDED

#include <glib.h>

G_BEGIN_DECLS typedef enum
{
  ST_ERROR_INVALID_VALUE,
  ST_ERROR_INVALID_RESPONSE,
  ST_ERROR_NOT_IMPLEMENTED,
  ST_ERROR_SYSTEM,
} STError;

#define ST_ERROR (st_error_quark ())

GQuark st_error_quark (void);

G_END_DECLS
#endif /* STARTER_ERROR_H_INCLUDED */
