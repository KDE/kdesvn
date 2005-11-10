INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/helpers \
	-I$(top_srcdir)/src/svnfrontend/fronthelpers $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = kded_kdesvnd.la
kded_kdesvnd_la_LDFLAGS = -avoid-version -module $(all_libraries)
kded_kdesvnd_la_LIBADD = \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la $(top_builddir)/src/helpers/libhelper.la \
	$(top_builddir)/src/svncpp/libsvnwrapper.la $(LIB_KIO) $(SVN_LIBS)
kded_kdesvnd_la_SOURCES = kdesvnd_dcop.cpp kdesvnd_dcop.skel
noinst_HEADERS = kdesvnd_dcop.h

kded_DATA = kdesvnd.desktop
kdeddir = $(kde_servicesdir)/kded

servicemenudir = $(kde_datadir)/konqueror/servicemenus
servicemenu_DATA = kdesvn_subversion.desktop kdesvn_subversion_toplevel.desktop
