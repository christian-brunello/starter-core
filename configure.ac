#
# starter-core - configure.ac
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
# Process this file with autoconf to produce configure.

AC_INIT([starter-core], [0.1.0], [startercontroller@gmail.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_CC
AM_PROG_AR

AC_PROG_YACC
AC_PROG_LEX([noyywrap])

LT_INIT

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.44 gobject-2.0 >= 2.44 gio-2.0 >= 2.44 gio-unix-2.0 avahi-glib avahi-client])
PKG_CHECK_MODULES([MYSQLCLIENT], [mysqlclient])

GLIB_GSETTINGS

AC_PATH_TOOL([GLIB_COMPILE_SCHEMAS], [glib-compile-schemas], [:])
if test "$GLIB_COMPILE_SCHEMAS" = ":" ; then
    AC_MSG_ERROR([glib-compile-schemas not found. Please install it and try again.])
fi

ST_CORE_CFLAGS="$GLIB_CFLAGS $MYSQLCLIENT_CFLAGS"
ST_CORE_LIBS="$GLIB_LIBS $MYSQLCLIENT_LIBS"

AC_SUBST(ST_CORE_CFLAGS)
AC_SUBST(ST_CORE_LIBS)

# Compatibility checks for APIs that vary across GLib / MySQL versions
save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
CFLAGS="$CFLAGS $GLIB_CFLAGS $MYSQLCLIENT_CFLAGS"
LIBS="$LIBS $GLIB_LIBS $MYSQLCLIENT_LIBS"

AC_CHECK_FUNCS([g_memdup2 g_ptr_array_copy g_ptr_array_steal g_string_replace])

AC_CHECK_DECL([MYSQL_OPT_RECONNECT],
  [AC_DEFINE([HAVE_MYSQL_OPT_RECONNECT], [1],
    [Define if MYSQL_OPT_RECONNECT is available])],
  [],
  [[#include <mysql.h>]])

CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"

AC_PATH_TOOL([MAKEINFO], [makeinfo], [:])
if test "$MAKEINFO" = ":" ; then
    AC_MSG_ERROR([makeinfo not found. Please install it and try again.])
fi

# Optional remote debug server (GDBus TCP). Runtime bind via
# --debug-server-bind=HOST:PORT (default 0.0.0.0:17373).
AC_ARG_ENABLE([debug-server],
  [AS_HELP_STRING([--enable-debug-server],
    [build starter-core with remote debug server interface])],
  [], [enable_debug_server=no])

AS_IF([test "x$enable_debug_server" = "xyes"],
  [AC_DEFINE([ENABLE_DEBUG_SERVER], [1],
    [Define to 1 to build the remote debug server])])
AM_CONDITIONAL([ENABLE_DEBUG_SERVER],
  [test "x$enable_debug_server" = "xyes"])

# =========================================================

# Definisce i file Makefile da creare
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  lib/Makefile
  src/Makefile
  test/Makefile
  examples/Makefile
  doc/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([
  debug-server: ${enable_debug_server}
])

