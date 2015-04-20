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

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

static GMainLoop *main_loop;

static gboolean dont_pair_moves = FALSE;
static GOptionEntry entries[] = {
  { "no-pair", 'N', 0, G_OPTION_ARG_NONE, &dont_pair_moves, N_("Don't send single MOVED events"), NULL },
  { NULL }
};

static gboolean
dir_monitor_callback (GFileMonitor* monitor,
		      GFile* child,
		      GFile* other_file,
		      GFileMonitorEvent eflags)
{

  char *name = g_file_get_parse_name (child);

  g_print ("Directory Monitor Event:\n");
  g_print ("Child = %s\n", name);
  g_free (name);

  if (other_file)
    {
      name = g_file_get_parse_name (other_file);
      g_print ("Other = %s\n", name);
      g_free (name);
    }

  switch (eflags)
    {
    case G_FILE_MONITOR_EVENT_CHANGED:
      g_print ("Event = CHANGED\n");
      break;
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
      g_print ("Event = CHANGES_DONE_HINT\n");
      break;
    case G_FILE_MONITOR_EVENT_DELETED:
      g_print ("Event = DELETED\n");
      break;
    case G_FILE_MONITOR_EVENT_CREATED:
      g_print ("Event = CREATED\n");
      break;
    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
      g_print ("Event = PRE_UNMOUNT\n");
      break;
    case G_FILE_MONITOR_EVENT_UNMOUNTED:
      g_print ("Event = UNMOUNTED\n");
      break;
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
      g_print ("Event = ATTRIB CHANGED\n");
      break;
    case G_FILE_MONITOR_EVENT_MOVED:
      g_print ("Event = MOVED\n");
      break;
    }

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GFileMonitor* dmonitor;
  GError *error;
  GOptionContext *context;
  GFile *file;
  gchar *param;
  gchar *summary;

  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, GVFS_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  error = NULL;
  param = g_strdup_printf ("[%s...]", _("LOCATION"));
  summary = _("Monitor directories for changes.");

  context = g_option_context_new (param);
  g_option_context_set_summary (context, summary);
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);
  g_free (param);

  if (error != NULL)
    {
      g_printerr (_("Error parsing commandline options: %s\n"), error->message);
      g_printerr ("\n");
      g_printerr (_("Try \"%s --help\" for more information."), g_get_prgname ());
      g_printerr ("\n");
      g_error_free (error);
      return 1;
    }

  if (argc > 1)
    {
      int i;
      GFileMonitorFlags flags = G_FILE_MONITOR_WATCH_MOUNTS;

      if (!dont_pair_moves)
	flags |= G_FILE_MONITOR_SEND_MOVED;

      for (i = 1; i < argc; i++)
	{
	  file = g_file_new_for_commandline_arg (argv[i]);
	  dmonitor = g_file_monitor_directory (file, flags, NULL, NULL);
	  if (dmonitor != NULL)
	    g_signal_connect (dmonitor, "changed", (GCallback)dir_monitor_callback, NULL);
	  else
	    {
	      g_print ("Monitoring not supported for %s\n", argv[1]);
	      return 1;
	    }
	}
    }

  main_loop = g_main_loop_new (NULL, FALSE);

  g_main_loop_run (main_loop);

  return 0;
}
