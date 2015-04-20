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

#include <glib.h>
#include <locale.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

static char *attr_type = "string";
static gboolean nofollow_symlinks = FALSE;

static GOptionEntry entries[] =
{
  { "type", 't', 0, G_OPTION_ARG_STRING, &attr_type, N_("Type of the attribute"), N_("TYPE") },
  { "nofollow-symlinks", 'n', 0, G_OPTION_ARG_NONE, &nofollow_symlinks, N_("Don't follow symbolic links"), NULL },
	{ NULL }
};

static char *
hex_unescape (const char *str)
{
  int i;
  char *unescaped_str, *p;
  unsigned char c;
  int len;

  len = strlen (str);
  unescaped_str = g_malloc (len + 1);

  p = unescaped_str;
  for (i = 0; i < len; i++)
    {
      if (str[i] == '\\' &&
	  str[i+1] == 'x' &&
	  len - i >= 4)
	{
	  c =
	    (g_ascii_xdigit_value (str[i+2]) << 4) |
	    g_ascii_xdigit_value (str[i+3]);
	  *p++ = c;
	  i += 3;
	}
      else
	*p++ = str[i];
    }
  *p++ = 0;

  return unescaped_str;
}

static GFileAttributeType
attribute_type_from_string (const char *str)
{
  if (strcmp (str, "string") == 0)
    return G_FILE_ATTRIBUTE_TYPE_STRING;
  if (strcmp (str, "stringv") == 0)
    return G_FILE_ATTRIBUTE_TYPE_STRINGV;
  if (strcmp (str, "bytestring") == 0)
    return G_FILE_ATTRIBUTE_TYPE_BYTE_STRING;
  if (strcmp (str, "boolean") == 0)
    return G_FILE_ATTRIBUTE_TYPE_BOOLEAN;
  if (strcmp (str, "uint32") == 0)
    return G_FILE_ATTRIBUTE_TYPE_UINT32;
  if (strcmp (str, "int32") == 0)
    return G_FILE_ATTRIBUTE_TYPE_INT32;
  if (strcmp (str, "uint64") == 0)
    return G_FILE_ATTRIBUTE_TYPE_UINT64;
  if (strcmp (str, "int64") == 0)
    return G_FILE_ATTRIBUTE_TYPE_INT64;
  if (strcmp (str, "object") == 0)
    return G_FILE_ATTRIBUTE_TYPE_OBJECT;
  if (strcmp (str, "unset") == 0)
    return G_FILE_ATTRIBUTE_TYPE_INVALID;
  return -1;
}

static void
show_help (GOptionContext *context, const char *error)
{
  char *help;

  if (error)
    g_printerr (_("Error: %s"), error);

  help = g_option_context_get_help (context, TRUE, NULL);
  g_printerr ("%s", help);
  g_free (help);
  g_option_context_free (context);
}

int
main (int argc, char *argv[])
{
  GError *error;
  GOptionContext *context;
  GFile *file;
  const char *attribute;
  GFileAttributeType type;
  gpointer value;
  gboolean bool;
  guint32 uint32;
  gint32 int32;
  guint64 uint64;
  gint64 int64;
  gchar *param;
  gchar *summary;

  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, GVFS_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  error = NULL;
  param = g_strdup_printf ("%s %s %s...", _("LOCATION"), _("ATTRIBUTE"), _("VALUE"));
  summary = _("Set a file attribute of LOCATION.");

  context = g_option_context_new (param);
  g_option_context_set_summary (context, summary);
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);

  if (error != NULL)
    {
      g_printerr (_("Error parsing commandline options: %s\n"), error->message);
      g_printerr ("\n");
      g_printerr (_("Try \"%s --help\" for more information."), g_get_prgname ());
      g_printerr ("\n");
      g_error_free (error);
      return 1;
    }

  if (argc < 2)
    {
      show_help (context, _("Location not specified\n"));
      return 1;
    }

  file = g_file_new_for_commandline_arg (argv[1]);

  if (argc < 3)
    {
      show_help (context, _("Attribute not specified\n"));
      return 1;
    }

  attribute = argv[2];

  type = attribute_type_from_string (attr_type);
  if ((argc < 4) && (type != G_FILE_ATTRIBUTE_TYPE_INVALID))
    {
      show_help (context, _("Value not specified\n"));
      return 1;
    }

  g_option_context_free (context);
  g_free (param);

  switch (type)
    {
    case G_FILE_ATTRIBUTE_TYPE_STRING:
      value = argv[3];
      break;
    case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING:
      value = hex_unescape (argv[3]);
      break;
    case G_FILE_ATTRIBUTE_TYPE_BOOLEAN:
      bool = g_ascii_strcasecmp (argv[3], "true") == 0;
      value = &bool;
      break;
    case G_FILE_ATTRIBUTE_TYPE_UINT32:
      uint32 = atol (argv[3]);
      value = &uint32;
      break;
    case G_FILE_ATTRIBUTE_TYPE_INT32:
      int32 = atol (argv[3]);
      value = &int32;
      break;
    case G_FILE_ATTRIBUTE_TYPE_UINT64:
      uint64 = g_ascii_strtoull (argv[3], NULL, 10);
      value = &uint64;
      break;
    case G_FILE_ATTRIBUTE_TYPE_INT64:
      int64 = g_ascii_strtoll (argv[3], NULL, 10);
      value = &int64;
      break;
    case G_FILE_ATTRIBUTE_TYPE_STRINGV:
      value = &argv[3];
      break;
    case G_FILE_ATTRIBUTE_TYPE_INVALID:
      value = NULL;
      break;
    case G_FILE_ATTRIBUTE_TYPE_OBJECT:
    default:
      g_printerr (_("Invalid attribute type %s\n"), attr_type);
      return 1;
    }

  if (!g_file_set_attribute (file,
			     attribute,
			     type,
			     value,
			     0, NULL, &error))
    {
      g_printerr (_("Error setting attribute: %s\n"), error->message);
      g_error_free (error);
      return 1;
    }

  return 0;
}
