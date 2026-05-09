#
# starter-core - Makefile.am
#
# Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

noinst_PROGRAMS = time-example oil-example beep-example

time_example_SOURCES = time.c
oil_example_SOURCES = oil.c
beep_example_SOURCES = beep.c

AM_CPPFLAGS = $(ST_CORE_CFLAGS) -I$(top_srcdir)/include
AM_LDFLAGS = $(ST_CORE_LIBS) -L$(top_builddir)/lib -lstarter-core
