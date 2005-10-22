INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/helpers \
	-I$(top_srcdir)/src/svnfrontend/fronthelpers $(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = kdesvnd
kdesvnd_LDFLAGS = $(all_libraries)
kdesvnd_LDADD = \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la $(top_builddir)/src/helpers/libhelper.la $(LIB_KIO)
kdesvnd_SOURCES = kdesvnd_dcop.cpp main.cpp kdesvnd_dcop.skel
noinst_HEADERS = kdesvnd_dcop.h

service_DATA = kdesvnd.desktop
servicedir = $(kde_servicesdir)
