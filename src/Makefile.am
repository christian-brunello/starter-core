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

bin_PROGRAMS = starter-core

starter_core_SOURCES = internals.h rules.y lex.l main.c mdns.c expr.c var.c engine.c 

starter_core_CPPFLAGS = $(AM_CPPFLAGS) $(ST_CORE_CFLAGS) -I$(top_srcdir)/include
starter_core_LDADD = $(ST_CORE_LIBS) -L$(top_builddir)/lib -lstarter-core -lm
starter_core_LDFLAGS = 

AM_YFLAGS = -d

EXTRA_DIST = schemas/org.starter.gschema.xml

schemasdir = $(datadir)/glib-2.0/schemas

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(schemasdir); \
	$(GLIB_COMPILE_SCHEMAS) --targetdir=$(DESTDIR)$(schemasdir) schemas/
