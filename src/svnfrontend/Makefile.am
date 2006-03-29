INCLUDES = -I$(top_srcdir)/src/svnfrontend/fronthelpers $(all_includes)
METASOURCES = AUTO
SUBDIRS = fronthelpers graphtree

noinst_LTLIBRARIES = libsvnfrontend.la

noinst_HEADERS = kdesvnfilelist.h filelistviewitem.h svnlogdlgimp.h \
		svnactions.h ccontextlistener.h propertiesdlg.h editproperty_impl.h stopdlg.h \
		importdir_logmsg.h mergedlg_impl.h copymoveview_impl.h itemdisplay.h svnfiletip.h \
		dummydisplay.h commandexec.h displaysettings_impl.h subversionsettings_impl.h \
		modifiedthread.h tcontextlistener.h threadcontextlistenerdata.h keystatus.h \
		dispcolorsettings_impl.h cacheentry.h revisiontreesettingsdlg_impl.h

libsvnfrontend_la_SOURCES = filelistviewitem.cpp svnlogdlgimp.cpp svnlogdlg.ui \
		svnactions.cpp ccontextlistener.cpp propertiesdlg.cpp editpropsdlg.ui \
		editproperty_impl.cpp stopdlg.cpp importdir_logmsg.cpp merge_dlg.ui mergedlg_impl.cpp \
		copymoveview.ui copymoveview_impl.cpp svnitem.cpp kdesvnfilelist.cpp itemdisplay.cpp \
		svnfiletip.cpp dummydisplay.cpp commandexec.cpp subversion_settings.ui \
		display_settings.ui displaysettings_impl.cpp subversionsettings_impl.cpp modifiedthread.cpp \
		tcontextlistener.cpp threadcontextlistenerdata.cpp keystatus.cpp dispcolor_settings.ui \
		dispcolorsettings_impl.cpp cacheentry.cpp revisiontree_settings.ui \
	revisiontreesettingsdlg_impl.cpp
>>>>>>> .merge-rechts.r725

libsvnfrontend_la_LIBADD = \
	$(top_builddir)/src/svnfrontend/fronthelpers/libsvnfrontend_helpers.la $(top_builddir)/src/svnfrontend/graphtree/libgraphtree.la
