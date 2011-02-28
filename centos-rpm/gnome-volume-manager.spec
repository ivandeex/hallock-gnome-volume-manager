Summary: The GNOME Volume Manager
Name: gnome-volume-manager
Version: 2.15.0
Release: 5%{?dist}
License: GPL
Group: Applications/System
Source0: gnome-volume-manager-%{version}.tar.bz2
Source1: magicdev-wrapper
Source2: gthumb-import
Source3: gnome-cdda-handler
Source4: cdda-url-handler.schemas
Patch0: gnome-volume-manager-0.9.10.add-to-base.patch
Patch1: gnome-volume-manager-1.5.11-rh-defaults.patch
Patch2: gnome-volume-manager-2.15.0.add-console-user-check.patch 
Patch3: gnome-volume-manager-2.15.0-check-screensaver.patch
BuildRoot: /var/tmp/%{name}-root
BuildPrereq: libgnomeui-devel, libglade2-devel, dbus-devel >= 0.90, dbus-glib-devel >= 0.70
BuildPrereq: hal-devel >= 0.5.0
BuildRequires: perl-XML-Parser
BuildRequires: gettext
Requires: hal >= 0.5.0, kernel >= 2.6, control-center >= 2.0
Requires: dbus >= 0.90
Requires: gthumb
Requires: gnome-media
Requires: gnome-mount >= 0.4
Requires(pre): GConf2 >= 2.14
Requires(post): GConf2 >= 2.14
Requires(preun): GConf2 >= 2.14
Obsoletes: magicdev
Provides: magicdev
ExcludeArch: s390
ExcludeArch: s390x

%description
The GNOME Volume Manager monitors volume-related events and responds with
user-specified policy.  The GNOME Volume Manager can automount hot-plugged
drives, automount inserted removable media, autorun programs, automatically
play audio CDs and video DVDs, and automatically import photos from a digital
camera.  The GNOME Volume Manager does this entirely in user-space and without
polling.

The GNOME Voume Manager sits at the top end of a larger picture that aims to
integrate the Linux system from the kernel on up through the desktop and its
applications.

%prep
%setup -q
%patch0 -p1 -b .add-to-base
%patch1 -p1 -b .rh-defaults
%patch2 -p1 -b .add-console-user-check
%patch3 -p0 -b .screensaver
%build
%configure CFLAGS="-ggdb3 -O0"
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xdg/autostart

install -m 755 %{SOURCE1} $RPM_BUILD_ROOT%{_bindir}/magicdev
install -m 755 %{SOURCE2} $RPM_BUILD_ROOT%{_bindir}/gthumb-import
install -d $RPM_BUILD_ROOT%{_libexecdir}/
install -m 755 %{SOURCE3} $RPM_BUILD_ROOT%{_libexecdir}/gnome-cdda-handler
install -m 755 %{SOURCE4} $RPM_BUILD_ROOT%{_sysconfdir}/gconf/schemas/cdda-url-handler.schemas

%find_lang %name

%clean
rm -rf $RPM_BUILD_ROOT

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
SCHEMAS="gnome-volume-manager.schemas cdda-url-handler.schemas"
for S in $SCHEMAS; do
  gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/$S >/dev/null || :
done

%pre
if [ "$1" -gt 1 ]; then
  export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
  SCHEMAS="gnome-volume-manager.schemas cdda-url-handler.schemas"
  for S in $SCHEMAS; do
    gconftool-2 --makefile-uninstall-rule %{_sysconfdir}/gconf/schemas/$S >/dev/null || :
  done
fi

%preun
if [ "$1" -eq 0 ]; then
  export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
  SCHEMAS="gnome-volume-manager.schemas cdda-url-handler.schemas"
  for S in $SCHEMAS; do
    gconftool-2 --makefile-uninstall-rule %{_sysconfdir}/gconf/schemas/$S >/dev/null || :
  done
fi

%files -f %{name}.lang
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README TODO
%{_bindir}/*
%{_datadir}/applications/gnome-volume-properties.desktop
%{_datadir}/gnome-volume-manager
%{_sysconfdir}/gconf/schemas/*.schemas
%{_libexecdir}/gnome-cdda-handler
%{_datadir}/gnome/autostart/gnome-volume-manager.desktop

%changelog
* Fri Oct 12 2007 - Bastien Nocera <bnocera@redhat.com> - 2.15.0-5%{?dist}
- Use gnome-cd by default for audio CD playback, not Totem
Related: rhbz #218151

* Tue Nov 14 2006 David Zeuthen <davidz@redhat.com> - 2.15.0-4%{?dist}
- Disable automounter when screensaver is running (#215612)

* Wed Oct 18 2006 Matthias Clasen <mclasen@redhat.com> - 2.15.0-3
- Fix scripts according to the packaging guidelines

* Thu Aug 10 2006 John (J5) Palmieri <johnp@redhat.com> - 2.15.0-2
- Add patch to use the console user files to check for local users
  instead of using utmp which is unreliable

* Thu Aug 03 2006 John (J5) Palmieri <johnp@redhat.com> - 2.15.0-1
- Update to 2.15.0
- Add dist tag to release
- Fix defaults to run the autophoto program when a camera is inserted

* Wed Jul 19 2006 John (J5) Palmieri <johnp@redhat.com> - 1.5.15-4
- Add BR for dbus-glib-devel

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 1.5.15-3.1.1
- rebuild

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 1.5.15-3.1
- rebuild

* Fri Jun  9 2006 Matthias Clasen <mclasen@redhat.com> 1.5.15-3
- Drop gratitious dependency on gnome-mime-data
- Add missing BuildRequires

* Mon Jun  5 2006 Matthias Clasen <mclasen@redhat.com> 1.5.15-2
- add missing BuildRequires

* Mon May 22 2006 Radek Vokál <rvokal@redhat.com> 1.5.15-1.2
- rebuilt against new libnotify

* Mon May 15 2006 John (J5) Palmieri <johnp@redhat.com> - 1.5.15-1.1
- bump and rebuild

* Sun Mar 12 2006 Ray Strode <rstrode@redhat.com> - 1.5.15-1
- update to 1.5.15

* Fri Feb 24 2006 David Zeuthen <davidz@redhat.com> - 1.5.13-3
- Make sure autostart desktop file gets installed in /etc/xdg/autostart

* Fri Feb 24 2006 David Zeuthen <davidz@redhat.com> - 1.5.13-2
- Shuffle some patches around to actually build this thing

* Fri Feb 24 2006 David Zeuthen <davidz@redhat.com> - 1.5.13-1
- Update to upstream release 1.5.13
- Require gnome-mount
- Include patch for handling encrypted file systems

* Mon Feb 13 2006 Ray Strode <rstrode@redhat.com> - 1.5.11-2
- get rid of gnome-printer-add

* Fri Feb 10 2006 Jesse Keating <jkeating@redhat.com> - 1.5.11-1.2
- bump again for double-long bug on ppc(64)

* Tue Feb 07 2006 Jesse Keating <jkeating@redhat.com> - 1.5.11-1.1
- rebuilt for new gcc4.1 snapshot and glibc changes

* Thu Jan 26 2006 Matthias Clasen <mclasen@redhat.com> - 1.5.11-1
- Update 1.5.11

* Wed Jan 18 2006 John (J5) Palmieri <johnp@redhat.com> - 1.5.9-3
- Switch to using goptions instead of popt which is causing a crash

* Wed Jan 18 2006 John (J5) Palmieri <johnp@redhat.com> - 1.5.9-2
- Add the no-ui patch from upstream. Next release should have this.

* Fri Jan 13 2006 Matthias Clasen <mclasen@redhat.com> - 1.5.9-1
- Update to 1.5.9

* Wed Jan 04 2006 John (J5) Palmieri <johnp@redhat.com> - 1.5.7-2
- Added a patch to fix an array being passed to dbus

* Thu Dec 20 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.7-1
- Update to 1.5.7

* Thu Dec 15 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.5-1
- Update to 1.5.5

* Fri Dec 09 2005 Jesse Keating <jkeating@redhat.com> - 1.5.4-1.1
- rebuilt

* Fri Dec  2 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.4-1
- Update to 1.5.4

* Thu Dec 01 2005 John (J5) Palmieri <johnp@redhat.com> - 1.5.3-3
- rebuild for new dbus

* Tue Nov  8 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.3-2
- disable debug spew

* Wed Oct 12 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.3-1
- update to 1.5.3

* Thu Oct  6 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.2-1
- update to 1.5.2

* Thu Sep  8 2005 Matthias Clasen <mclasen@redhat.com> - 1.5.1-1
- update to 1.5.1

* Thu Mar 13 2005 John (J5) Palmieri <johnp@redhat.com> - 1.3.1-1
- update to upstream version 1.3.1

* Mon Mar 07 2005 John (J5) Palmieri <johnp@redhat.com> - 1.1.3-3
- Fixed up hal-api patch

* Fri Feb 25 2005 John (J5) Palmieri <johnp@redhat.com> - 1.1.3-2
- Reenable BuildPrereq for hal-devel

* Wed Feb 23 2005 John (J5) Palmieri <johnp@redhat.com> - 1.1.3-1
- Update to upstream version
- add patch to update to the new hal-0.5.0 API
- took out locking patch as it was added upstream
- took out policy-after-explicit-mount-only patch as it was added
  upstream
- removed %{_datadir}/control-center-2.0/capplets/* glob
- added %{_datadir}/applications/gnome-volume-properties.desktop 

* Mon Oct 18 2004 John (J5) Palmieri <johnp@redhat.com> - 1.1.0-5
- Merged the photo-defaults patch with a new rh-defaults patch
- Shut off autorun by default as it is a security concern and we
  currently don't have any use for it
- Added policy-after-explicit-mount-only patch which fixes problems
  such as a user mounting a removable drive from the command line
  and a Nautilus window pops up, and two Nautilus windows popping 
  up in browser mode when a device is mounted through Nautilus.

* Fri Oct 08 2004 John (J5) Palmieri <johnp@redhat.com> - 1.1.0-4
- exclude building on s390 and s390x

* Wed Oct 06 2004 John (J5) Palmieri <johnp@redhat.com> - 1.1.0-3
- Added gnome-cdda-handler and associated schema to cause the
  cdda url handler to use the values in g-v-m's gconf key for
  audio cd's.  This causes the g-v-m selected application for 
  audio cd's to launch when double clicking on an audio cd icon
  in Nautilus.

* Thu Sep 30 2004 John (J5) Palmieri <johnp@redhat.com>
- Added patch to add the dbus-glib-lowlevel.h header

* Mon Sep 20 2004 David Zeuthen <davidz@redhat.com>
- Update to upstream version 1.1.0
- Fix requirement for gnome-cd to be the gnome-media package

* Mon Sep 20 2004 David Zeuthen <davidz@redhat.com>
- Add locking patch

* Fri Sep 17 2004 John (J5) Palmieri <johnp@redhat.com>
- Update to upstream 1.0.2
- Readd patches I took out by accident
- Add a requirement for gthumb and gnome-cd

* Mon Sep 13 2004 John (J5) Palmieri <johnp@redhat.com>
- Update to upstream 1.0

* Fri Sep 03 2004 John (J5) Palmieri <johnp@redhat.com>
- gthumb-import now checks to see if there is a DCIM directory
  and open up to that directory if so 

* Wed Sep 01 2004 Davidz Zeuthen <davidz@redhat.com>
- Fix a few issues to keep up with hal

* Tue Aug 31 2004 John (J5) Palmieri <johnp@redhat.com>
- Update to upstream release 0.9.10
- Add gthumb-import wrapper to import photos correctly
- Add patch to set gthumb-import as the default photo importer
- Add patch to put desktop file in the X-Red-Hat-Base Catagory
- Fixes RH Bug 130866

* Fri Aug 20 2004 John (J5) Palmieri <johnp@redhat.com>
- Added Provides: magicdev to fix conflict

* Tue Aug 17 2004 David Zeuthen <davidz@redhat.com>
- Update to upstream release 0.9.9
- Apply mount-unmount patch

* Mon Jul 26 2004 Ray Strode <rstrode@redhat.com>
- Add magicdev compatibility wrapper script and obsolete magicdev.

* Thu May 27 2004 John (J5) Palmieri <johnp@redhat.com>
- Upstream CVS 

* Wed May 05 2004 John (J5) Palmieri <johnp@redhat.com>
- Made sure GConf Schemas are installed

* Tue Apr 20 2004 John (J5) Palmieri <johnp@redhat.com>
- initial packaging of 0.9.2

