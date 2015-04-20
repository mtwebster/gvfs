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

#ifndef __G_VFS_JOB_CLOSE_WRITE_H__
#define __G_VFS_JOB_CLOSE_WRITE_H__

#include <gvfsjob.h>
#include <gvfsbackend.h>
#include <gvfswritechannel.h>

G_BEGIN_DECLS

#define G_VFS_TYPE_JOB_CLOSE_WRITE         (g_vfs_job_close_write_get_type ())
#define G_VFS_JOB_CLOSE_WRITE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), G_VFS_TYPE_JOB_CLOSE_WRITE, GVfsJobCloseWrite))
#define G_VFS_JOB_CLOSE_WRITE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), G_VFS_TYPE_JOB_CLOSE_WRITE, GVfsJobCloseWriteClass))
#define G_VFS_IS_JOB_CLOSE_WRITE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), G_VFS_TYPE_JOB_CLOSE_WRITE))
#define G_VFS_IS_JOB_CLOSE_WRITE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), G_VFS_TYPE_JOB_CLOSE_WRITE))
#define G_VFS_JOB_CLOSE_WRITE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), G_VFS_TYPE_JOB_CLOSE_WRITE, GVfsJobCloseWriteClass))

typedef struct _GVfsJobCloseWriteClass   GVfsJobCloseWriteClass;

struct _GVfsJobCloseWrite
{
  GVfsJob parent_instance;

  char *etag;
  GVfsWriteChannel *channel;
  GVfsBackend *backend;
  GVfsBackendHandle handle;
};

struct _GVfsJobCloseWriteClass
{
  GVfsJobClass parent_class;
};

GType g_vfs_job_close_write_get_type (void) G_GNUC_CONST;

GVfsJob *g_vfs_job_close_write_new (GVfsWriteChannel   *channel,
				    GVfsBackendHandle  handle,
				    GVfsBackend       *backend);

void     g_vfs_job_close_write_set_etag (GVfsJobCloseWrite *job,
					 const char *etag);

G_END_DECLS

#endif /* __G_VFS_JOB_CLOSE_WRITE_H__ */
