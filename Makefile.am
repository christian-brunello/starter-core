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

SUBDIRS = include lib src examples doc

starterconfdir = $(sysconfdir)/starter
starterconf_DATA = rules.conf

systemdsystemunitdir = /lib/systemd/system
systemdsystemunit_DATA = starter-core.service

starter-core.service: starter-core.service.in
	$(AM_V_GEN)sed -e 's|[@]bindir[@]|$(bindir)|g' \
	               -e 's|[@]sbindir[@]|$(sbindir)|g' \
		       -e 's|[@]sysconfdir[@]|$(sysconfdir)|g' \
	               < $< > $@-t && mv $@-t $@

all-local: starter-core.service

EXTRA_DIST = starter-core.service.in rules.conf starter-core.pc.in
CLEANFILES = starter-core.service starter-core.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = starter-core.pc

starter-core.pc: starter-core.pc.in
	sed -e 's|[@]prefix[@]|$(prefix)|g' \
	    -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
	    -e 's|[@]libdir[@]|$(libdir)|g' \
	    -e 's|[@]includedir[@]|$(includedir)|g' \
	    -e 's|[@]PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	    -e 's|[@]PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	    $< > $@
