# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   A subversion client for the KDE
Name:      kdesvn
Version:   0.6.0
Release:   1
License:   LGPL
Vendor:    Rajko Albrecht <ral@alwins-world.de>
Url:       http://www.alwins-world.de
Icon:      kdesvn_logo_klein.xpm
Packager:  Rajko Albrecht <ral@alwins-world.de>
Group:     Development/Tools
Source:    kdesvn-%version.tar.gz
BuildRoot: /tmp/kdebuild
BuildPreReq: apr-devel
BuildPreReq: apr-util-devel
BuildPreReq: neon-devel
BuildPreReq: subversion-devel >= 1.1.0
Requires: subversion

%description
Kdsvn is a subversion client for KDE

%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                --prefix=`kde-config --prefix` \
                --disable-no-exceptions \
                --disable-debug
%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.kdesvn
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.kdesvn
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.kdesvn

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/kdesvn
rm -rf ../file.list.kdesvn


%files -f ../file.list.kdesvn
