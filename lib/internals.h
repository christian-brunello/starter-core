/*
 * libstarter-core - internals.h
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

#ifndef LIBSTARTER_CORE_INTERNALS_H_INCLUDED
#define LIBSTARTER_CORE_INTERNALS_H_INCLUDED

// G_MESSAGES_DEBUG=sp-mdns

#include <glib.h>

#define LIBSTARTER_CORE_LOG_DOMAIN "libstarter-core"

#define LOGD(...) g_log (LIBSTARTER_CORE_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI(...) g_log (LIBSTARTER_CORE_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW(...) g_log (LIBSTARTER_CORE_LOG_DOMAIN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE(...) g_log (LIBSTARTER_CORE_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

#endif /* LIBSTARTER_CORE_INTERNALS_H_INCLUDED */
