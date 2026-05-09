/*
 * libstarter-core - defs.h
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

#ifndef STARTER_DEFS_H_INCLUDED
#define STARTER_DEFS_H_INCLUDED

#include <inttypes.h>
#include <glib.h>

G_BEGIN_DECLS
#define ST_IO_FLAG_NULL            0x0000000000000000LL
#define ST_IO_FLAG_MANUAL_OVERRIDE 0x0000000000000001LL
#define ST_IO_FLAG_NO_HISTORY      0x0000000000000002LL
#define ST_IO_FLAG_USR1            0x1000000000000000LL
#define ST_IO_FLAG_USR2            0x2000000000000000LL
#define ST_IO_FLAG_USR3            0x4000000000000000LL
#define ST_IO_FLAG_USR4            0x8000000000000000LL
#define ST_IO_FLAG_USR_MASK        0xF000000000000000LL
  typedef enum
{
  ST_UNIT_NULL,
  ST_UNIT_CELSIUS,
  ST_UNIT_PERCENT,
  ST_UNIT_BAR,
  ST_UNIT_ONOFF,
  ST_UNIT_ENUM,
  ST_UNIT_V,
  ST_UNIT_W,
  ST_UNIT_VAR,
  ST_UNIT_KW,
  ST_UNIT_WH,
  ST_UNIT_KWH,
  ST_UNIT_SECONDS,
  ST_UNIT_MINUTES,
  ST_UNIT_HOURS
} STUnit;

#define ST_INVALID_VALUE G_MAXDOUBLE

G_END_DECLS
#endif /* STARTER_DEFS_H_INCLUDED */
