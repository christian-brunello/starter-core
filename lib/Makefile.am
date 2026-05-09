#
# libstarter-core - Makefile.am
#
# Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, see <http://www.gnu.org/licenses/>.

lib_LTLIBRARIES = libstarter-core.la

libstarter_core_la_SOURCES = internals.h \
			     error.c \
			     input.c \
			     output.c \
			     mdns-service.c \
			     mysql-client.c \
			     mdns.c \
			     stats.c \
			     stats-collector.c \
			     process-stats-collector.c \
			     client.c \
			     server.c \
			     core.c

libstarter_core_la_CPPFLAGS = $(AM_CPPFLAGS) $(ST_CORE_CFLAGS) -I$(top_srcdir)/include
libstarter_core_la_LIBADD = $(ST_CORE_LIBS) -L$(top_builddir)/lib 
libstarter_core_la_LDFLAGS = -version-info 0:0:0
