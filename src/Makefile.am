iconsdir = $(pkgdatadir)/icons
icons_DATA = gvm-dev-tablet.png

INCLUDES = 					   \
	$(CAPPLET_CFLAGS) 			   \
	$(GVM_CFLAGS)			   	   \
	$(NOTIFY_CFLAGS)			   \
	-DDBUS_API_SUBJECT_TO_CHANGE		   \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"   \
	-DBINDIR=\"$(bindir)\"			   \
	-DGLADEDIR=\"$(pkgdatadir)\"		   \
	-DICONSDIR=\"$(iconsdir)\"		   \
	-DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gnome-volume-manager gnome-volume-properties

gnome_volume_manager_SOURCES = manager.c clipboard.c gvm.h
gnome_volume_manager_LDADD = $(GVM_LIBS) $(NOTIFY_LIBS)

gnome_volume_properties_SOURCES = properties.c gvm.h
gnome_volume_properties_LDADD = $(CAPPLET_LIBS)

EXTRA_DIST = $(icons_DATA)
