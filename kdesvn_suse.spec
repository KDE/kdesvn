#
# spec file for package kdesvn
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           kdesvn
Version:        2.1.0
Release:        0
Summary:        KDE Subversion Client
License:        GPL-2.0+
Group:          Development/Tools/Version Control
Url:            http://projects.kde.org/kdesvn
Source:         %{name}-%{version}.tar.xz
BuildRequires:  extra-cmake-modules
BuildRequires:  subversion-devel
BuildRequires:  kbookmarks-devel
BuildRequires:  kconfig-devel
BuildRequires:  kconfigwidgets-devel
BuildRequires:  kcoreaddons-devel
BuildRequires:  kdbusaddons-devel
BuildRequires:  kdoctools-devel
BuildRequires:  ki18n-devel
BuildRequires:  kiconthemes-devel
BuildRequires:  kitemviews-devel
BuildRequires:  kjobwidgets-devel
BuildRequires:  kio-devel
BuildRequires:  knotifications-devel
BuildRequires:  kservice-devel
BuildRequires:  ktexteditor-devel
BuildRequires:  kwallet-devel
BuildRequires:  kwidgetsaddons-devel
BuildRequires:  pkgconfig(Qt5Sql)

Obsoletes:      %{name}5 < %{version}
Provides:       %{name}5 = %{version}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
kdesvn is a GUI client for subversion repositories.

%prep
%setup -q

%build
%cmake_kf5
%make_jobs

%install
%kf5_makeinstall
rm -rf %{buildroot}%{_kf5_servicesdir}/svn*.protocol
%kf5_post_install

%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING COPYING.OpenSSL
%{_kf5_bindir}/kdesvn
%{_kf5_bindir}/kdesvnaskpass
%{_kf5_plugindir}/kdesvnpart.so
%{_kf5_plugindir}/kio_ksvn.so
%{_kf5_plugindir}/kf5/kded/kdesvnd.so
%{_kf5_applicationsdir}/org.kde.kdesvn.desktop
%{_kf5_configkcfgdir}/
%{_kf5_dbusinterfacesdir}/kf5_org.kde.kdesvnd.xml
%{_kf5_sharedir}/dbus-1/services/org.kde.kdesvnd.service
%{_kf5_htmldir}/en/kdesvn
%{_kf5_iconsdir}/hicolor/*/actions/kdesvn*
%{_kf5_iconsdir}/hicolor/*/apps/kdesvn*
%{_kf5_iconsdir}/hicolor/*/places/kdesvn*
%{_kf5_sharedir}/kconf_update/
%{_kf5_sharedir}/kdesvn/
%{_kf5_servicesdir}/
%{_kf5_kxmlguidir}/kdesvn/
%{_kf5_sharedir}/man/man1/kdesvn*
# try to make brb happy ...
%dir %{_kf5_iconsdir}/hicolor/96x96
%dir %{_kf5_iconsdir}/hicolor/*/actions
%dir %{_kf5_iconsdir}/hicolor/*/apps
%dir %{_kf5_iconsdir}/hicolor/*/places


%changelog
* Sun Apr 17 2016 ch.ehrlicher@gmx.de
- Initial spec for opensuse / kf5
