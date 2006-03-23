INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/helpers \
	-I$(top_srcdir)/src/svnfrontend/fronthelpers $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kded_kdesvnd.la

kded_kdesvnd_la_LDFLAGS = -avoid-version -module $(all_libraries)
kded_kdesvnd_la_LIBADD = \
	$(top_builddir)/src/svnqt/libsvnqt.la \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la \
	$(top_builddir)/src/helpers/libhelper.la \
	$(LIB_KIO) $(SVN_LIBS)
	
kded_kdesvnd_la_SOURCES = kdesvnd_dcop.cpp kdesvnd_dcop.skel

noinst_HEADERS = kdesvnd_dcop.h

kdeddir = $(kde_servicesdir)/kded
kded_DATA = kdesvnd.desktop

servicemenudir = $(kde_datadir)/konqueror/servicemenus
servicemenu_DATA = kdesvn_subversion.desktop kdesvn_subversion_toplevel.desktop

kdesvn_subversion.desktop: $(top_srcdir)/src/kdesvnd/kdesvn_subversion.desktop.template
	sh $(top_srcdir)/src/kdesvnd/test.kde.sh $< $@

kdesvn_subversion_toplevel.desktop: $(top_srcdir)/src/kdesvnd/kdesvn_subversion_toplevel.desktop.template
	sh $(top_srcdir)/src/kdesvnd/test.kde.sh $< $@

EXTRA_DIST = kdesvn_subversion_toplevel.desktop.template kdesvn_subversion.desktop.template test.kde.sh

clean-generic:
	rm -f kdesvn_subversion_toplevel.desktop
	rm -f kdesvn_subversion.desktop
