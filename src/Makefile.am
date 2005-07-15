## Makefile.am for kdesvn

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kdesvn kdesvn_client

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/src $(all_includes)

# the library search path. 
kdesvn_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
kdesvn_LDADD = $(top_builddir)/src/listview/liblistview.a \
	$(top_builddir)/src/svncpp/libsvncpp.a $(APR_LIBS) $(NEON_LIBS) $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEPRINT) \
	$(SVN_LIBS)

# which sources should be compiled for kdesvn
kdesvn_SOURCES = main.cpp kdesvn.cpp kdesvnview.cpp pref.cpp kdesvniface.skel

# these are the headers for your project
noinst_HEADERS = kdesvn.h kdesvnview.h pref.h

# client stuff
kdesvn_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdesvn_client_LDADD = $(LIB_KDECORE)
kdesvn_client_SOURCES = kdesvn_client.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kdesvn.pot

KDE_ICON = AUTO

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/Utilities
kdelnk_DATA = kdesvn.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kdesvn
rc_DATA = kdesvnui.rc

SUBDIRS = listview svncpp
