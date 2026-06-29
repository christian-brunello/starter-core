/*
 * starter core - mysql-mock.c
 *
 * Authored by Gemini AI (Google) as a collaborative engineering effort.
 * * CRITICAL LIMITATION: Only this specific file is dedicated to the public 
 * domain under the terms of the Creative Commons Zero (CC0 1.0 Universal) license. 
 * This dedication does NOT apply to any other files, source code, or architecture 
 * within the STARTER framework, which remain strictly protected.
 *
 * You can copy, modify, distribute and perform the work in this file, even for 
 * commercial purposes, all without asking permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

static gchar *last_intercepted_query = NULL;
static gboolean simulate_query_failure = FALSE;
static gchar *mock_row_value = NULL;

void mock_mysql_set_query_failure (gboolean fail) { simulate_query_failure = fail; }
void mock_mysql_set_row_value (const gchar *val) { mock_row_value = (gchar *)val; }
const gchar *mock_mysql_get_last_query (void) { return last_intercepted_query; }

MYSQL *mysql_init (MYSQL *mysql) { return (MYSQL *) 0xDEADBEEF; }
void mysql_close (MYSQL *mysql) {}
const char *mysql_error (MYSQL *mysql) { return "Simulated Mock Database Engine Error Engine Exception"; }
void mysql_free_result (MYSQL_RES *res) {}

int 
mysql_options (MYSQL *mysql, enum mysql_option option, const void *arg) 
{ 
  return 0; 
}

MYSQL *
mysql_real_connect (MYSQL *mysql, const char *host, const char *user,
                    const char *passwd, const char *db, unsigned int port,
                    const char *unix_socket, unsigned long clientflag)
{
  return mysql;
}

int
mysql_query (MYSQL *mysql, const char *q)
{
  if (last_intercepted_query) 
    free (last_intercepted_query);
  
  last_intercepted_query = strdup (q);

  if (simulate_query_failure) 
    return 1;
    
  return 0;
}

MYSQL_RES *
mysql_store_result (MYSQL *mysql)
{
  if (mock_row_value != NULL) 
    return (MYSQL_RES *) 0xCAFEBABE;
  return NULL;
}

uint64_t
mysql_num_rows (MYSQL_RES *res)
{
  if (res == (MYSQL_RES *) 0xCAFEBABE && mock_row_value != NULL) 
    return 1;
  return 0;
}

MYSQL_ROW
mysql_fetch_row (MYSQL_RES *res)
{
  static char *row[1];
  row[0] = mock_row_value;
  return row;
}

