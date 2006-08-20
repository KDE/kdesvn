INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO
BUILT_SOURCES = kdesvnsettings.h
kde_kcfg_DATA = kdesvn_part.kcfg

noinst_LTLIBRARIES = libkdesvnsettings.la

noinst_HEADERS = displaysettings_impl.h dispcolorsettings_impl.h \
	revisiontreesettingsdlg_impl.h subversionsettings_impl.h \
	cmdexecsettings_impl.h

libkdesvnsettings_la_SOURCES = kdesvnsettings.kcfgc displaysettings_impl.cpp \
	display_settings.ui dispcolorsettings_impl.cpp dispcolor_settings.ui \
	revisiontreesettingsdlg_impl.cpp revisiontree_settings.ui subversionsettings_impl.cpp \
	subversion_settings.ui cmdexecsettings.ui
