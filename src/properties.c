/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * src/properties.c - control panel applet for gnome-volume-manager
 *
 * Robert Love <rml@novell.com>
 *
 * (C) Copyright 2005 Novell, Inc.
 *
 * Licensed under the GNU GPL v2.  See COPYING.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gnome.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <libhal.h>

#include "gvm.h"

#define GLADE_XML_FILE	"gnome-volume-properties.glade"


typedef enum {
	TYPE_BOOL,
	TYPE_STRING,
} type_t;

enum {
	AUTOBROWSE,
	AUTOBURN,
	AUTOBURN_AUDIO_CD_COMMAND,
	AUTOBURN_DATA_CD_COMMAND,
	AUTOIPOD,
	AUTOIPOD_COMMAND,
	AUTOKEYBOARD,
	AUTOKEYBOARD_COMMAND,
	AUTOMOUNT_DRIVES,
	AUTOMOUNT_MEDIA,
	AUTOMOUSE,
	AUTOMOUSE_COMMAND,
	AUTOOPEN,
	AUTOPHOTO,
	AUTOPHOTO_COMMAND,
	AUTOPILOT,
	AUTOPILOT_COMMAND,
	AUTOPLAY_CDA,
	AUTOPLAY_CDA_COMMAND,
	AUTOPLAY_DVD,
	AUTOPLAY_DVD_COMMAND,
	AUTOPOCKETPC,
	AUTOPOCKETPC_COMMAND,
	AUTOPRINTER,
	AUTOPRINTER_COMMAND,
	AUTORUN,
	AUTOSCANNER,
	AUTOSCANNER_COMMAND,
	AUTOTABLET,
	AUTOTABLET_COMMAND,
	AUTOVIDEOCAM,
	AUTOVIDEOCAM_COMMAND,
	AUTORUN_PATH,
	AUTOOPEN_PATH,
};

static struct {
	char *key;
	type_t type;
	GtkWidget *widget;
	gboolean need_daemon;
} gvm_settings[] = {
	{ GCONF_ROOT "autobrowse",                TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoburn",                  TYPE_BOOL,   NULL, FALSE },
	{ GCONF_ROOT "autoburn_audio_cd_command", TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoburn_data_cd_command",  TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoipod",                  TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoipod_command",          TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autokeyboard",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autokeyboard_command",      TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "automount_drives",          TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "automount_media",           TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "automouse",                 TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "automouse_command",         TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoopen",                  TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autophoto",                 TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autophoto_command",         TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autopalmsync",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autopalmsync_command",      TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoplay_cda",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoplay_cda_command",      TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoplay_dvd",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoplay_dvd_command",      TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autopocketpc",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autopocketpc_command",      TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autoprinter",               TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoprinter_command",       TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autorun",                   TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoscanner",               TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autoscanner_command",       TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autotablet",                TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autotablet_command",        TYPE_STRING, NULL, FALSE },
	{ GCONF_ROOT "autovideocam",              TYPE_BOOL,   NULL, TRUE  },
	{ GCONF_ROOT "autovideocam_command",      TYPE_STRING, NULL, FALSE },
	/* The following entries do not yet have a UI */
	/*{ GCONF_ROOT "autorun_path",         TYPE_STRING, NULL, FALSE },*/
	/*{ GCONF_ROOT "autoopen_path",        TYPE_STRING, NULL, FALSE },*/
};

static GHashTable *gvm_settings_hash = NULL;
static GConfClient *gconf = NULL;
static GladeXML *xml = NULL;
static gboolean updating = FALSE;
static GtkWidget *props = NULL;

static void
changed_cb (GtkEntry *entry, const char *key)
{
	const char *str;
	
	if (updating)
		return;
	
	str = gtk_entry_get_text (entry);
	gconf_client_set_string (gconf, key, str ? str : "", NULL);
}

static void
toggled_cb (GtkToggleButton *toggle, const char *key)
{
	GtkWidget *hbox;
	gboolean bool;
	char *name;
	
	bool = gtk_toggle_button_get_active (toggle);
	
	if (!updating)
		gconf_client_set_bool (gconf, key, bool, NULL);
	
	name = strrchr (key, '/') + 1;
	if (!strncmp (name, "automount_", 10)) {
		bool = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[AUTOMOUNT_MEDIA].widget)) ||
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[AUTOMOUNT_DRIVES].widget));
		gtk_widget_set_sensitive (gvm_settings[AUTOOPEN].widget, bool);
		gtk_widget_set_sensitive (gvm_settings[AUTORUN].widget, bool);
	} else {
		name = g_strdup_printf ("%s_hbox", name);
		if ((hbox = glade_xml_get_widget (xml, name)))
			gtk_widget_set_sensitive (hbox, bool);
		g_free (name);
	}
}

static void
gconf_changed_cb (GConfClient *client __attribute__((__unused__)),
		  guint id __attribute__((__unused__)),
		  GConfEntry *entry,
		  gpointer data __attribute__((__unused__)))
{
	GConfValue *value;
	gpointer result;
	int which;
	
	g_return_if_fail (gconf_entry_get_key (entry) != NULL);
	
	if (!(value = gconf_entry_get_value (entry)))
		return;
	
	if (!(result = g_hash_table_lookup (gvm_settings_hash, entry->key)))
		return;
	
	which = GPOINTER_TO_INT (result) - 1;
	
	updating = TRUE;
	
	if (gvm_settings[which].type == TYPE_STRING) {
		gtk_entry_set_text (GTK_ENTRY (gvm_settings[which].widget), gconf_value_get_string (value));
	} else if (gvm_settings[which].type == TYPE_BOOL) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gvm_settings[which].widget),
					      gconf_value_get_bool (value));
	} else {
		g_assert_not_reached ();
	}
	
	updating = FALSE;
}

static void
set_sensitivity (void)
{
	GtkWidget *hbox;
	gboolean bool;
	size_t i;
	
	/* this is the only strange one */
	bool = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[AUTOMOUNT_MEDIA].widget)) ||
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[AUTOMOUNT_DRIVES].widget));
	gtk_widget_set_sensitive (gvm_settings[AUTORUN].widget, bool);
	
	/* checkboxes can enable/disable the ability to change other settings */
	for (i = 0; i < G_N_ELEMENTS (gvm_settings); i++) {
		if (gvm_settings[i].type == TYPE_BOOL) {
			/* check if we have dependents */
			char *name;
			
			name = strrchr (gvm_settings[i].key, '/') + 1;
			name = g_strdup_printf ("%s_hbox", name);
			if ((hbox = glade_xml_get_widget (xml, name))) {
				bool = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[i].widget));
				gtk_widget_set_sensitive (hbox, bool);
			}
			g_free (name);
		}
	}
}

/*
 * close_cb - gtk call-back on the "Close" button
 */
static void
close_cb (GtkWidget *dialog,
	  int response __attribute__((__unused__)),
	  gpointer user_data __attribute__((__unused__)))
{
	gtk_widget_destroy (dialog);
	gtk_main_quit ();
}

/*
 * set_icon - helper function to bind an icon from a given GnomeIconTheme to
 * a given GtkImage.
 */
static void
set_icon (GtkImage *image, GtkIconTheme *theme, const char *name)
{
	GdkPixbuf *pixbuf;
	struct stat st;
	
	if (*name == '/') {
		if (stat (name, &st) == 0 && S_ISREG (st.st_mode)) {
			gtk_image_set_from_file (image, name);
			return;
		} else
			name = "gnome-dev-broken-image";
	}
	
	if ((pixbuf = gtk_icon_theme_load_icon (theme, name, 48, 0, NULL))) {
		gtk_image_set_from_pixbuf (image, pixbuf);
		g_object_unref (pixbuf);
	}
}

static struct {
	const char *name;
	const char *icon;
} icons[] = {
	{ "audio_cd_image",         "gnome-dev-cdrom-audio"   },
	{ "blank_cd_image",         "gnome-dev-disc-cdr"      },
	{ "digital_camera_image",   "applets-screenshooter"   },
	{ "dvd_video_image",        "gnome-dev-dvd"           },
	{ "ipod_image",             "gnome-dev-ipod"          },
	{ "keyboard_image",         "gnome-dev-keyboard"      },
	{ "mouse_image",            "gnome-dev-mouse-optical" },
	{ "palm_image",             "palm-pilot"              },
	/* FIXME: need a new icon */
	{ "pocketpc_image",         "palm-pilot"              },
	{ "printer_image",          "gnome-dev-printer"       },
	{ "removable_drives_image", "gnome-dev-removable"     },
	{ "scanner_image",          "xsane"                   },
	/* FIXME: need better icon */
	{ "tablet_image",           ICONSDIR "/gvm-dev-tablet.png" },
	/* FIXME: need better icon */
	{ "videocam_image",         "gnome-mime-video"        }
};

static void
theme_changed_cb (GtkIconTheme *theme, GladeXML *xml)
{
	GtkWidget *icon;
	size_t i;
	
	for (i = 0; i < G_N_ELEMENTS (icons); i++) {
		icon = glade_xml_get_widget (xml, icons[i].name);
		set_icon (GTK_IMAGE (icon), theme, icons[i].icon);
	}
}

static void
show_props (void)
{
	GtkIconTheme *theme;
	char *filename;
	size_t i;
	
	filename = g_concat_dir_and_file (GLADEDIR, GLADE_XML_FILE);
	xml = glade_xml_new (filename, NULL, PACKAGE);
	g_free (filename);
	
	if (xml == NULL) {
		GtkWidget *dialog;
		
		dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
						 _("Could not load the main interface"));
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  _("Please make sure that the "
							    "volume manager is properly "
							    "installed"));
		
		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		exit (EXIT_FAILURE);
	}
	
	props = glade_xml_get_widget (xml, "dialog1");
	g_signal_connect (props, "response", G_CALLBACK (close_cb), NULL);
	
	theme = gtk_icon_theme_get_default ();
	g_signal_connect (theme, "changed", G_CALLBACK (theme_changed_cb), xml);
	theme_changed_cb (theme, xml);
	
	gtk_window_set_default_icon_name ("gnome-dev-cdrom");
	
	gvm_settings_hash = g_hash_table_new (g_str_hash, g_str_equal);
	for (i = 0; i < G_N_ELEMENTS (gvm_settings); i++) {
		const char *name;
		
		g_hash_table_insert (gvm_settings_hash, gvm_settings[i].key, GINT_TO_POINTER (i + 1));
		
		name = strrchr (gvm_settings[i].key, '/') + 1;
		gvm_settings[i].widget = glade_xml_get_widget (xml, name);
		if (gvm_settings[i].type == TYPE_STRING) {
			GtkWidget *fileentry;
			char *str;
			
			str = gconf_client_get_string (gconf, gvm_settings[i].key, NULL);
			gtk_entry_set_text (GTK_ENTRY (gvm_settings[i].widget), str ? str : "");
			g_free (str);
			
			str = g_strdup_printf ("%s_fileentry", name);
			fileentry = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, str)));
			g_free (str);
			
			/* FIXME: do we really want to share the same history_id across CDR, CDA, and DVD command file entries? */
			gnome_entry_set_history_id (GNOME_ENTRY (fileentry), "CD_CAPPLET_ID");
			gtk_combo_set_case_sensitive (GTK_COMBO (fileentry), FALSE);
			
			g_signal_connect (gvm_settings[i].widget, "changed", G_CALLBACK (changed_cb), (void *) gvm_settings[i].key);
		} else if (gvm_settings[i].type == TYPE_BOOL) {
			gboolean bool;
			
			bool = gconf_client_get_bool (gconf, gvm_settings[i].key, NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gvm_settings[i].widget), bool);
			g_signal_connect (gvm_settings[i].widget, "toggled", G_CALLBACK (toggled_cb), (void *) gvm_settings[i].key);
		} else {
			g_assert_not_reached ();
		}
	}
	
	set_sensitivity ();
	
	gtk_widget_show_all (props);
}

/** Check if HAL is running
 *
 * @return			TRUE if HAL is running and working.
 *				FALSE otherwise.
 */
static gboolean
check_if_hal_is_running (void)
{
	LibHalContext *ctx;
	char **devices;
	int nr;
	DBusError error;
	DBusConnection *conn;
	
	if (!(ctx = libhal_ctx_new ()))
		return FALSE;
	
	dbus_error_init (&error);
	
	conn = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set (&error)) {
		dbus_error_free (&error);
		return FALSE;
	}
	
	libhal_ctx_set_dbus_connection (ctx, conn);
	
	if (!libhal_ctx_init (ctx, &error)) {
		dbus_error_free (&error);
		return FALSE;
	}
	
	/*
	 * Do something to ping the HAL daemon - the above functions will
	 * succeed even if hald is not running, so long as DBUS is.  But we
	 * want to exit silently if hald is not running, to behave on
	 * pre-2.6 systems.
	 */
	if (!(devices = libhal_get_all_devices (ctx, &nr, &error))) {
		dbus_error_free (&error);
		
		libhal_ctx_shutdown (ctx, NULL);
		libhal_ctx_free (ctx);
		
		return FALSE;
	}
	
	libhal_free_string_array (devices);
	
	libhal_ctx_shutdown (ctx, NULL);
	libhal_ctx_free (ctx);
	
	return TRUE;
}

/*
 * check_clipboard - check if the CLIPBOARD_NAME clipboard is available
 *
 * Returns TRUE if the CLIPBOARD_NAME clipboard is out there and FALSE
 * otherwise
 */
static gboolean
check_clipboard (void)
{
	Atom clipboard_atom = gdk_x11_get_xatom_by_name (CLIPBOARD_NAME);

	return XGetSelectionOwner (GDK_DISPLAY (), clipboard_atom) != None;
}

/*
 * check_daemon - check if the daemon itself is running. if not, and if we
 * have configuration options set that would make it worthwhile to run, run
 * it.
 */
static void
check_daemon (void)
{
	gboolean need_daemon = FALSE;
	GError *error = NULL;
	char *argv[2];
	size_t i;
	
	for (i = 0; !need_daemon && i < G_N_ELEMENTS (gvm_settings); i++) {
		if (gvm_settings[i].need_daemon)
			need_daemon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gvm_settings[i].widget));
	}
	
	if (!need_daemon || check_clipboard ()) 
		return;
	
	argv[0] = BINDIR "/gnome-volume-manager";
	argv[1] = NULL;
	g_spawn_async (g_get_home_dir (), argv, NULL, 0, NULL,
			NULL, NULL, &error);
	
	if (error) {
		GtkWidget *message = gtk_message_dialog_new
			(GTK_WINDOW (props), GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			 _("Error starting gnome-volume-manager daemon:\n%s"),
			 error->message);
		
		gtk_dialog_run (GTK_DIALOG (message));
		
		g_error_free (error);
		
		exit (EXIT_FAILURE);
	}
}


int
main (int argc, char **argv)
{
	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv,
			    GNOME_PARAM_NONE, GNOME_PARAM_NONE);
	
	glade_gnome_init ();
	
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);
	
	/* bail out now if hald is not even running */
	if (!check_if_hal_is_running ()) {
		GtkWidget *dialog;
		
		dialog = gtk_message_dialog_new (NULL,
						 0, GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 _("Volume management not supported"));
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  _("The \"hald\" service is required but not currently "
							    "running. Enable the service and rerun this application, "
							    "or contact your system administrator.\n\n"
							    "Note: You need Linux kernel 2.6 for volume "
							    "management to work."));
		
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		exit (EXIT_FAILURE);
	}
	
	gconf = gconf_client_get_default ();
	gconf_client_add_dir (gconf, GCONF_ROOT_SANS_SLASH,
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_notify_add (gconf, GCONF_ROOT_SANS_SLASH,
				 gconf_changed_cb, NULL, NULL, NULL);
	
	show_props ();
	check_daemon ();
	
	gtk_main ();
	
	g_hash_table_destroy (gvm_settings_hash);
	g_object_unref (gconf);
	g_object_unref (xml);
	
	return 0;
}
