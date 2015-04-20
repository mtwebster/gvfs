/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2006-2007 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Author: David Zeuthen <davidz@redhat.com>
 *         Chrsitian Kellner <gicmo@gnome.org>
 */

#ifndef __HAL_UTILS_H__
#define __HAL_UTILS_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

const char * get_disc_icon (const char *disc_type);
const char * get_disc_name (const char *disc_type, gboolean is_blank);

GIcon *      get_themed_icon_with_fallbacks (const char *icon_name,
                                             const char *fallbacks);

char **dupv_and_uniqify (char **str_array);

G_END_DECLS

#endif /* __HAL_UTILS_H__ */
