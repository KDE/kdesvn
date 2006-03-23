INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = kio_ksvn.la

kio_ksvn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN)
kio_ksvn_la_LIBADD = \
	$(top_builddir)/src/svnqt/libsvnqt.la \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la \
	$(top_builddir)/src/helpers/libhelper.la \
	$(APR_LIBS) $(SVN_LIBS) \
	$(LIB_KFILE) $(LIB_KPARTS) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO) -lDCOP

kio_ksvn_la_SOURCES = kiosvn.cpp kiolistener.cpp

noinst_HEADERS = kiosvn.h kiolistener.h

protocoldir = $(kde_servicesdir)
protocol_DATA = ksvn+http.protocol ksvn+https.protocol ksvn+file.protocol ksvn+ssh.protocol ksvn.protocol \
	svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol

