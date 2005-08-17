## Makefile.am for kdesvn

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kdesvn kdesvnaskpass

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/src $(all_includes)

# the library search path. 
kdesvn_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.


# which sources should be compiled for kdesvn
kdesvn_SOURCES = main.cpp kdesvn.cpp pref.cpp

# these are the headers for your project
noinst_HEADERS = kdesvn.h kdesvnview.h pref.h urldlg.h

# client stuff

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	find . -name '*.cpp' | xargs $(XGETTEXT) -o $(podir)/kdesvn.pot

KDE_ICON = AUTO

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/Development
kdelnk_DATA = kdesvn.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kdesvn
rc_DATA = kdesvnui.rc

SUBDIRS = svnfrontend svncpp helpers pics
kdesvnaskpass_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdesvnaskpass_SOURCES = kdesvn-askpass.cpp
kdesvnaskpass_LDADD = $(LIB_KDEUI)

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkdesvnpart.la

# the Part's source, library search path, and link libraries
libkdesvnpart_la_SOURCES = kdesvn_part.cpp urldlg.cpp kdesvnview.cpp
libkdesvnpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkdesvnpart_la_LIBADD = $(top_builddir)/src/svnfrontend/libsvnfrontend.la \
	$(top_builddir)/src/svncpp/libsvnwrapper.la $(top_builddir)/src/helpers/libhelper.la $(SVN_LIBS) $(NEON_LIBS) \
	$(APR_LIBS) $(LIB_KPARTS) $(LIB_KFILE)

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
# is set via the application file
# partdesktop_DATA = kdesvn_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kdesvnpart
partrc_DATA = kdesvn_part.rc

kdesvn_LDADD = $(LIB_KPARTS)
