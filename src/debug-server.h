/*
 * starter-core - debug-server.h
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

#ifndef ST_CORE_DEBUG_SERVER_H_INCLUDED
#define ST_CORE_DEBUG_SERVER_H_INCLUDED

#include <glib.h>
#include <glib-object.h>

#include "internals.h"

G_BEGIN_DECLS

#define ST_CORE_DEBUG_DEFAULT_BIND "0.0.0.0:17373"

#define ST_TYPE_CORE_DEBUG (st_core_debug_get_type ())
G_DECLARE_FINAL_TYPE (STCoreDebug, st_core_debug, ST, CORE_DEBUG, GObject)

STCoreDebug *st_core_debug_new (STCore * core);

gboolean st_core_debug_start (STCoreDebug * self, const gchar * bind_spec,
			      GError ** error);
void st_core_debug_stop (STCoreDebug * self);

gboolean st_core_debug_is_active (STCoreDebug * self);
const gchar *st_core_debug_get_bind_host (STCoreDebug * self);
guint16 st_core_debug_get_bind_port (STCoreDebug * self);
const gchar *st_core_debug_get_client_address (STCoreDebug * self);

/*
 * Sniff helpers: update ListPeers snapshot (for peers) and broadcast
 * D-Bus signals to connected debugger clients. No-op-safe if self is NULL.
 */
void st_core_debug_emit_peer_changed (STCoreDebug * self,
				      const gchar * name,
				      const gchar * address, guint16 port,
				      guint state, const gchar * event);
void st_core_debug_emit_input_changed (STCoreDebug * self, STInput * in);
void st_core_debug_emit_output_changed (STCoreDebug * self, STOutput * out);
void st_core_debug_emit_stats_changed (STCoreDebug * self, STStats * stats);
void st_core_debug_emit_var_changed (STCoreDebug * self, STVar * var);

/*
 * Control: inject into the core process image.
 * SetInput updates the local STInput cache (triggers val-changed / rules).
 * SetOutput calls st_client_set_output (bypasses MANUAL_OVERRIDE gating).
 * Returns TRUE on success.
 */
gboolean st_core_debug_set_input (STCoreDebug * self, const gchar * id,
				  gdouble value, guint64 flags,
				  GError ** error);
gboolean st_core_debug_set_output (STCoreDebug * self, const gchar * id,
				   gdouble value, guint64 flags,
				   GError ** error);

/*
 * Breakpoints / watchpoints (cooperative pause before engine_apply).
 *
 * Watchpoint: hit when id's value changes (input/output/var).
 * Breakpoint: hit when rules are about to run because of id.
 * Both pause rule evaluation until Continue().
 *
 * intercept_* return TRUE → caller must skip engine_apply.
 */
guint st_core_debug_add_breakpoint (STCoreDebug * self, const gchar * id,
				    GError ** error);
gboolean st_core_debug_remove_breakpoint (STCoreDebug * self, guint handle,
					  GError ** error);
guint st_core_debug_add_watchpoint (STCoreDebug * self, const gchar * id,
				    GError ** error);
gboolean st_core_debug_remove_watchpoint (STCoreDebug * self, guint handle,
					  GError ** error);
void st_core_debug_continue (STCoreDebug * self);
gboolean st_core_debug_is_paused (STCoreDebug * self);

gboolean st_core_debug_intercept_change (STCoreDebug * self,
					 const gchar * id, gdouble value);
gboolean st_core_debug_intercept_apply (STCoreDebug * self,
					const gchar * id);

G_END_DECLS
#endif /* ST_CORE_DEBUG_SERVER_H_INCLUDED */
