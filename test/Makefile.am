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

AM_CFLAGS = $(ST_CORE_CFLAGS) -I$(top_srcdir)/include
AM_LDFLAGS = $(ST_CORE_LIBS) -L$(top_builddir)/lib -lstarter-core -lm

TESTS = test-input test-output test-stats test-process-stats-collector test-mdns test-mysql-client test-server test-client test-client-server test-client-server-armageddon
check_PROGRAMS = $(TESTS)

test_input_SOURCES = test-input.c
test_input_CFLAGS = $(AM_CFLAGS)
test_input_LDADD = $(AM_LDFLAGS) 

test_output_SOURCES = test-output.c
test_output_CFLAGS = $(AM_CFLAGS)
test_output_LDADD = $(AM_LDFLAGS) 

test_stats_SOURCES = test-stats.c
test_stats_CFLAGS = $(AM_CFLAGS)
test_stats_LDADD = $(AM_LDFLAGS) 

test_process_stats_collector_SOURCES = test-process-stats-collector.c
test_process_stats_collector_CFLAGS = $(AM_CFLAGS)
test_process_stats_collector_LDADD = $(AM_LDFLAGS) 

test_mdns_SOURCES = test-mdns.c
test_mdns_CFLAGS = $(AM_CFLAGS)
test_mdns_LDADD = $(AM_LDFLAGS) 

test_mysql_client_SOURCES = test-mysql-client.c
test_mysql_client_CFLAGS = $(AM_CFLAGS)
test_mysql_client_LDFLAGS = $(AM_LDFLAGS) -export-dynamic
test_mysql_client_LDADD = $(LDADD)
test_mysql_client_DEPENDENCIES = .libs/libmysqlmock.so

test_server_SOURCES = test-server.c
test_server_CFLAGS = $(AM_CFLAGS)
test_server_LDADD = $(AM_LDFLAGS) 

test_client_SOURCES = test-client.c
test_client_CFLAGS = $(AM_CFLAGS)
test_client_LDADD = $(AM_LDFLAGS) 

test_client_server_SOURCES = test-client-server.c
test_client_server_CFLAGS = $(AM_CFLAGS)
test_client_server_LDADD = $(AM_LDFLAGS) 

test_client_server_armageddon_SOURCES = test-client-server-armageddon.c
test_client_server_armageddon_CFLAGS = $(AM_CFLAGS)
test_client_server_armageddon_LDADD = $(AM_LDFLAGS) 

# 1. Regola di compilazione automatica per il .so del mock
# Viene compilato in automatico prima di lanciare i test
.libs/libmysqlmock.so: mysql-mock.c
	@mkdir -p .libs
	$(CC) -shared -fPIC -o .libs/libmysqlmock.so $(srcdir)/mysql-mock.c $(test_mysql_client_CFLAGS)

# 3. L'AUTOMAZIONE CRUCIALE: Inietta il preload automaticamente in 'make check'
# Usiamo abs_builddir per avere il percorso assoluto inattaccabile da ld.so
AM_TESTS_ENVIRONMENT = \
	LD_PRELOAD=$(abs_builddir)/.libs/libmysqlmock.so; \
	export LD_PRELOAD;

# Pulisce il file .so quando lanci 'make clean'
CLEANFILES = .libs/libmysqlmock.so

