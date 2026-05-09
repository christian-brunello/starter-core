/*
 * starter-core - internals.h
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ST_CORE_INTERNALS_H_INCLUDED
#define ST_CORE_INTERNALS_H_INCLUDED

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <starter/mdns.h>
#include <starter/stats.h>
#include <starter/process-stats-collector.h>
#include <starter/input.h>
#include <starter/output.h>
#include <starter/client.h>
#include <starter/mysql-client.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#define ST_CORE_LOG_DOMAIN "starter-core"

#define LOGD(...) g_log (ST_CORE_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) g_log (ST_CORE_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) g_log (ST_CORE_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE(...) g_log (ST_CORE_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

typedef enum VerboseLevel
{
  VERBOSE_LEVEL_NULL,
  VERBOSE_LEVEL_1,
  VERBOSE_LEVEL_2,
  VERBOSE_LEVEL_3
} VerboseLevel;

void verbose_printf (VerboseLevel vl, const char *fmt, ...);
#define VERBOSE_1_PRINTF(fmt...) verbose_printf(VERBOSE_LEVEL_1, fmt)
#define VERBOSE_2_PRINTF(fmt...) verbose_printf(VERBOSE_LEVEL_2, fmt)
#define VERBOSE_3_PRINTF(fmt...) verbose_printf(VERBOSE_LEVEL_3, fmt)

struct _Expr;

typedef struct
{
  STClient *client;
  guint index;
} ClientInputMap;

typedef ClientInputMap ClientOutputMap;



typedef enum
{
  EXPR_TYPE_LITERAL,
  EXPR_TYPE_OP,
  EXPR_TYPE_REF,
  EXPR_TYPE_FUNC
} ExprType;

typedef struct _Expr
{
  ExprType type;
  union
  {
    gchar *ref;
    gdouble literal;
    gchar *op;
    struct
    {
      gchar *id;
      GPtrArray *args;
    } func;
  } data;
  struct
  {
    gboolean (*eval) (struct _Expr * expr, GHashTable * inputs,
		      GHashTable * outputs, GHashTable * variables, gdouble *,
		      GError **);
    void (*destroy) (struct _Expr *);
    gchar *(*describe) (struct _Expr *, GString *);
      gboolean (*have_ref_to) (struct _Expr * expr, const gchar * id,
			       GHashTable * variables);
  } callbacks;
  struct _Expr *left;
  struct _Expr *right;
  int nref;
} Expr;

Expr *expr_ref (Expr *);
void expr_unref (Expr *);
Expr *expr_new_literal (gdouble val);
Expr *expr_new_op (const gchar * op, Expr * left, Expr * right);
Expr *expr_new_ref (const gchar * ref);
Expr *expr_new_func_min (GPtrArray * args);
Expr *expr_new_func_max (GPtrArray * args);
Expr *expr_new_func_round (GPtrArray * args);
Expr *expr_new_func_limit (GPtrArray * args);
Expr *expr_new_func_eval (GPtrArray * args);
Expr *expr_new_func_changed (GPtrArray * args);
gdouble expr_eval (Expr * self, GHashTable * inputs, GHashTable * outputs,
		   GHashTable * variables, gdouble * res, GError ** error);
gchar *expr_describe (Expr * self, GString * s);
gboolean expr_have_ref_to (Expr * self, const gchar * id,
			   GHashTable * variables);

#define ST_TYPE_VAR (st_var_get_type ())

G_DECLARE_FINAL_TYPE (STVar, st_var, ST, VAR, GObject)
     STVar *st_var_new (const gchar * name, Expr * value);
     const gchar *st_var_get_name (const STVar * self);
     Expr *st_var_get_value (const STVar * self);
     void st_var_set_value (STVar * self, Expr * value);

     typedef struct
     {
       gchar *id;
       Expr *rval;
     } Assign;

     void assign_delete (Assign * self);
     Assign *assign_new (const gchar * id, Expr * rval);
     gboolean assign_apply (Assign * self, GHashTable * inputs,
			    GHashTable * outputs, GHashTable * variables,
			    GError ** error);
     gchar *assign_describe (Assign * self, GString * s);
     gboolean assign_have_ref_to (Assign * self, const gchar * id,
				  GHashTable * variables);

     typedef struct
     {
       Expr *cond;
       GPtrArray *ass;
     } Trigger;

     void trigger_delete (Trigger * self);
     Trigger *trigger_new (Expr * cond, GPtrArray * ass);
     gchar *trigger_describe (Trigger * self, GString * s);
     gboolean trigger_apply (Trigger * self, GHashTable * inputs,
			     GHashTable * outputs, GHashTable * variables,
			     GError ** error);
     gboolean trigger_have_ref_to (Trigger * self, const gchar * id,
				   GHashTable * variables);

     typedef struct
     {
       Expr *cond;
       Assign *ass;
     } LabelEntry;

     void label_entry_delete (LabelEntry * self);
     LabelEntry *label_entry_new (Expr * cond, Assign * ass);
     gchar *label_entry_describe (LabelEntry * self, GString * s);
     gboolean label_entry_have_ref_to (LabelEntry * self, const gchar * id,
				       GHashTable * variables);

     typedef struct
     {
       GPtrArray *entries;
     } LabelBlock;

     void lable_block_delete (LabelBlock * self);
     LabelBlock *label_block_new (GPtrArray * entries);
     gchar *label_block_describe (LabelBlock * self, GString * s);
     gboolean label_block_apply (LabelBlock * self, GHashTable * inputs,
				 GHashTable * outputs, GHashTable * variables,
				 GError ** error);
     gboolean label_block_have_ref_to (LabelBlock * self, const gchar * id,
				       GHashTable * variables);

     typedef struct
     {
       GPtrArray *labels;
       GPtrArray *triggers;
       GHashTable *variables;
     } Engine;

     void engine_delete (Engine * self);
     Engine *engine_new (GPtrArray * labels, GPtrArray * triggers);
     gchar *engine_describe (Engine * self, GString * s);
     gboolean engine_apply (Engine * self, GHashTable * inputs,
			    GHashTable * outputs, const char *id,
			    GError ** error);

     typedef struct _STCore STCore;

     struct _STCore
     {
       GSettings *settings;
       gboolean no_discovery;
       gchar *srvmatch;
       STMDNS *mdns;
       STStats *self_stats;
       STStatsCollector *self_coll;
       GHashTable *clients;
       GHashTable *all_inputs;
       GHashTable *all_outputs;
       STMYSQLClient *mysql;
       gchar *rules_file;
       gchar *rules_preprocessor;
       Engine *engine;
     };

#define DISCOVERY_ENABLED(self) ((self)->no_discovery == FALSE)

     gboolean st_core_mdns_init (STCore * self);
     void st_core_mdns_finish (STCore * self);

     void st_core_input_changed_callback (STInput * in, gpointer user_data);
     void st_core_output_changed_callback (STOutput * out,
					   gpointer user_data);
     void st_core_stats_changed_callback (STStats * stats,
					  gpointer user_data);
     GHashTable *st_core_setup_inputs_hash_table (STCore * self);
     GHashTable *st_core_setup_outputs_hash_table (STCore * self);

#endif /* ST_CORE_INTERNALS_H_INCLUDED */
