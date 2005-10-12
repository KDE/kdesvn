INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = kio_svn.la

kio_svn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN)
kio_svn_la_LIBADD = $(top_builddir)/src/svncpp/libsvnwrapper.la \
	$(top_builddir)/src/helpers/libhelper.la $(top_builddir)/src/svnfrontend/libsvnfrontend.la $(LIB_KFILE) \
	$(LIB_KPARTS) $(APR_LIBS) $(NEON_LIBS) $(SVN_LIBS) -lkio
noinst_HEADERS = kiosvn.h
kio_svn_la_SOURCES = kiosvn.cpp
