#
# spec file for package kdesvn (Version 0.4.2)
#
# Copyright (c) 2005 Víctor Fernández Martínez. Valencia, Spain.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# norootforbuild
BuildRequires:  kdelibs3-devel subversion-devel update-desktop-files

Name:       kdesvn
Summary:    A Subversion client for KDE
Summary(de):Ein Subversionclient für KDE
Summary(es):Un cliente de Subversion para KDE
Version:    0.6.2
Release:    1polinux
URL:        http://www.alwins-world.de/programs/kdesvn/
Source:     http://www.alwins-world.de/programs/download/kdesvn/%{name}-%{version}.tar.gz
License:    LGPL
Group:      Development/Tools/Version Control
Buildroot:  %{_tmppath}/%{name}-%{version}-root
Requires:   kdelibs3
Requires:   subversion
Packager:   Víctor Fernández <vfernandez@polinux.upv.es>

%description
KDESvn is a frontend to the subversion vcs. In difference to most other tools
it uses the subversion C-Api direct via a c++ wrapper made by Rapid SVN and
doesn't parse the output of the subversion client. So it is a real client
itself instead of a frontend to the command line tool.

It is hardly designed for the K-Desktop environment and uses all of the goodies
it has. It is planned for future that based on the native client some plugins
for Konqueror and/or Kate will made.

Author
---------------
Rajko Albrecht <ral@alwins-world.de>

%prep
%setup -q
. /etc/opt/kde3/common_options

%build
. /etc/opt/kde3/common_options
./configure --enable-final $configkde
make

%install
. /etc/opt/kde3/common_options
make DESTDIR=$RPM_BUILD_ROOT $INSTALL_TARGET
kde_post_install

# Desktop entry
mkdir -p $RPM_BUILD_ROOT/opt/kde3/share/applications/kde
cp -f src/kdesvn.desktop $RPM_BUILD_ROOT/opt/kde3/share/applications/kde
rm -Rf $RPM_BUILD_ROOT/opt/kde3/share/applnk
%suse_update_desktop_file kdesvn Development RevisionControl


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt/kde3/*
%doc COPYING NEWS README TODO AUTHORS

%changelog
* Sat Aug 13 2005 - vfernandez@polinux.upv.es
- first build
