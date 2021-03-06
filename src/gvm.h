/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef _GVM_H
#define _GVM_H

/* structure to represent the configuration of the volume manager */
struct gvm_configuration {
	GConfClient *client;
	
	gboolean automount_drives;
	gboolean automount_media;
	gboolean autobrowse;
	gboolean autorun;
	char *autorun_path;
	gboolean autoopen;
	char *autoopen_path;
	
	gboolean autoburn;
	char *autoburn_audio_cd_command;
	char *autoburn_data_cd_command;
	
	gboolean autoplay_cda;
	char *autoplay_cda_command;
	
	gboolean autoplay_dvd;
	char *autoplay_dvd_command;
	
	gboolean autoplay_vcd;
	char *autoplay_vcd_command;
	
	gboolean autophoto;
	char *autophoto_command;
	
	gboolean autoipod;
	char *autoipod_command;
	
	gboolean autoprinter;
	char *autoprinter_command;
	
	gboolean autoscanner;
	char *autoscanner_command;
	
	gboolean autokeyboard;
	char *autokeyboard_command;
	
	gboolean automouse;
	char *automouse_command;
	
	gboolean autotablet;
	char *autotablet_command;
	
	gboolean autovideocam;
	char *autovideocam_command;
	
	gboolean autopilot;
	char *autopilot_command;
	
	gboolean autopocketpc;
	char *autopocketpc_command;
        
        double percent_threshold;
        double percent_used;
};

/* where our settings are rooted in the gconf tree */
#define GCONF_ROOT_SANS_SLASH	"/desktop/gnome/volume_manager"
#define GCONF_ROOT		GCONF_ROOT_SANS_SLASH "/"

#define CLIPBOARD_NAME		"GVM_SELECTION"

extern gboolean gvm_get_clipboard (void);

#endif	/* _GVM_H */
