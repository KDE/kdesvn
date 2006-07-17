INCLUDES = $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libsvnqt.la

libsvnqt_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 4:0:2

svnqtinclude_HEADERS = annotate_line.hpp apr.hpp check.hpp client.hpp \
			context.hpp datetime.hpp dirent.hpp entry.hpp log_entry.hpp path.hpp pool.hpp \
			revision.hpp status.hpp targets.hpp url.hpp wc.hpp version_check.hpp lock_entry.hpp \
			info_entry.hpp svnqt_defines.hpp context_listener.hpp exception.hpp commititem.hpp \
			repository.hpp repositorylistener.hpp svnstream.hpp svnfilestream.hpp
svnqtincludedir = $(includedir)/svnqt

noinst_HEADERS = client_impl.hpp contextdata.h repositorydata.hpp
	

libsvnqt_la_SOURCES = apr.cpp client_annotate.cpp client_cat.cpp client.cpp \
		client_diff.cpp client_ls.cpp client_modify.cpp client_property.cpp client_status.cpp \
		context.cpp datetime.cpp dirent.cpp entry.cpp exception.cpp log_entry.cpp path.cpp \
		pool.cpp revision.cpp status.cpp targets.cpp url.cpp wc.cpp version_check.cpp \
		lock_entry.cpp client_lock.cpp info_entry.cpp client_impl.cpp contextdata.cpp \
		commititem.cpp repository.cpp repositorydata.cpp repositorylistener.cpp svnstream.cpp \
	svnfilestream.cpp
libsvnqt_la_LIBADD = $(APR_LIBS) $(SVN_LIBS) $(LIB_QT)
