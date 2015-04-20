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
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include <config.h>

#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <gio/gio.h>
#include "gvfsdaemonutils.h"
#include "gvfsdaemonprotocol.h"


char *
g_error_to_daemon_reply (GError *error, guint32 seq_nr, gsize *len_out)
{
  char *buffer;
  const char *domain;
  gsize domain_len, message_len;
  GVfsDaemonSocketProtocolReply *reply;
  gsize len;
  
  domain = g_quark_to_string (error->domain);
  domain_len = strlen (domain);
  message_len = strlen (error->message);

  len = G_VFS_DAEMON_SOCKET_PROTOCOL_REPLY_SIZE +
    domain_len + 1 + message_len + 1;
  buffer = g_malloc (len);

  reply = (GVfsDaemonSocketProtocolReply *)buffer;
  reply->type = g_htonl (G_VFS_DAEMON_SOCKET_PROTOCOL_REPLY_ERROR);
  reply->seq_nr = g_htonl (seq_nr);
  reply->arg1 = g_htonl (error->code);
  reply->arg2 = g_htonl (domain_len + 1 + message_len + 1);

  memcpy (buffer + G_VFS_DAEMON_SOCKET_PROTOCOL_REPLY_SIZE,
	  domain, domain_len + 1);
  memcpy (buffer + G_VFS_DAEMON_SOCKET_PROTOCOL_REPLY_SIZE + domain_len + 1,
	  error->message, message_len + 1);
  
  *len_out = len;
  
  return buffer;
}

/**
 * gvfs_file_info_populate_default:
 * @info: file info to populate
 * @name_string: a bytes string of possibly the full path to the given file
 * @type: type of this file
 *
 * Calls gvfs_file_info_populate_names_as_local() and 
 * gvfs_file_info_populate_content_types() on the given @name_string.
 **/
void
gvfs_file_info_populate_default (GFileInfo  *info,
                                 const char *name_string,
			         GFileType   type)
{
  char *edit_name;

  g_return_if_fail (G_IS_FILE_INFO (info));
  g_return_if_fail (name_string != NULL);

  edit_name = gvfs_file_info_populate_names_as_local (info, name_string);
  gvfs_file_info_populate_content_types (info, edit_name, type);
  g_free (edit_name);
}

/**
 * gvfs_file_info_populate_names_as_local:
 * @info: the file info to fill
 * @name_string: a bytes string of possibly the full path to the given file
 *
 * Sets the name of the file info to @name_string and determines display and 
 * edit name for it.
 *
 * This generates the display name based on what encoding is used for local filenames.
 * It might be a good thing to use if you have no idea of the remote system filename
 * encoding, but if you know the actual encoding use, or if you allow per-mount
 * configuration of filename encoding in your backend you should not use this.
 * 
 * Returns: the utf-8 encoded edit name for the given file.
 **/
char *
gvfs_file_info_populate_names_as_local (GFileInfo  *info,
					const char *name_string)
{
  //const char *slash;
  char *edit_name;

  g_return_val_if_fail (G_IS_FILE_INFO (info), NULL);
  g_return_val_if_fail (name_string != NULL, NULL);

#if 0
  slash = strrchr (name_string, '/');
  if (slash && slash[1])
    name_string = slash + 1;
#endif
  edit_name = g_filename_display_basename (name_string);
  g_file_info_set_edit_name (info, edit_name);

  if (strstr (edit_name, "\357\277\275") != NULL)
    {
      char *display_name;
      
      display_name = g_strconcat (edit_name, _(" (invalid encoding)"), NULL);
      g_file_info_set_display_name (info, display_name);
      g_free (display_name);
    }
  else
    g_file_info_set_display_name (info, edit_name);

  return edit_name;
}

/**
 * gvfs_file_info_populate_content_types:
 * @info: the file info to fill
 * @basename: utf-8 encoded base name of file
 * @type: type of this file
 *
 * Takes the base name and guesses content type and icon with it. This function
 * is intended for remote files. Do not use it for directories.
 **/
void
gvfs_file_info_populate_content_types (GFileInfo  *info,
				       const char *basename,
				       GFileType   type)
{
  char *free_mimetype = NULL;
  const char *mimetype;
  GIcon *icon;
  GIcon *symbolic_icon;

  g_return_if_fail (G_IS_FILE_INFO (info));
  g_return_if_fail (basename != NULL);

  g_file_info_set_file_type (info, type);

  switch (type)
    {
      case G_FILE_TYPE_DIRECTORY:
	mimetype = "inode/directory";
	break;
      case G_FILE_TYPE_SYMBOLIC_LINK:
	mimetype = "inode/symlink";
	break;
      case G_FILE_TYPE_SPECIAL:
	mimetype = "inode/special";
	break;
      case G_FILE_TYPE_SHORTCUT:
	mimetype = "inode/shortcut";
	break;
      case G_FILE_TYPE_MOUNTABLE:
	mimetype = "inode/mountable";
	break;
      case G_FILE_TYPE_REGULAR:
	free_mimetype = g_content_type_guess (basename, NULL, 0, NULL);
	mimetype = free_mimetype;
	break;
      case G_FILE_TYPE_UNKNOWN:
      default:
        mimetype = "application/octet-stream";
	break;
    }

  g_file_info_set_content_type (info, mimetype);
  g_file_info_set_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE, mimetype);

  if (type == G_FILE_TYPE_DIRECTORY)
    {
      icon = g_themed_icon_new ("folder");
      symbolic_icon = g_themed_icon_new ("folder-symbolic");
    }
  else
    {
      icon = g_content_type_get_icon (mimetype);
      symbolic_icon = g_content_type_get_symbolic_icon (mimetype);
    }
  
  g_file_info_set_icon (info, icon);
  g_object_unref (icon);
  g_file_info_set_symbolic_icon (info, symbolic_icon);
  g_object_unref (symbolic_icon);

  g_free (free_mimetype);
}

