INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = kio_ksvn.la

kio_ksvn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN)
kio_ksvn_la_LIBADD = \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la $(top_builddir)/src/svncpp/libsvnwrapper.la \
	$(top_builddir)/src/helpers/libhelper.la $(LIB_KFILE) $(LIB_KPARTS) $(APR_LIBS) $(NEON_LIBS) $(SVN_LIBS) -lkio
noinst_HEADERS = kiosvn.h kiolistener.h
kio_ksvn_la_SOURCES = kiosvn.cpp kiolistener.cpp

protocol_DATA = ksvn+http.protocol ksvn+https.protocol ksvn+file.protocol ksvn+ssh.protocol ksvn.protocol \
	svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol
protocoldir = $(kde_servicesdir)

