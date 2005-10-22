INCLUDES = $(all_includes)
METASOURCES = AUTO
BUILT_SOURCES = settings.h
kde_kcfg_DATA = kdesvn_part.kcfg

noinst_LTLIBRARIES = libsvnfrontend_helpers.la

noinst_HEADERS = rangeinput_impl.h authdialogimpl.h 				logmsg_impl.h \
					ssltrustprompt_impl.h checkoutinfo_impl.h

libsvnfrontend_helpers_la_SOURCES = rangeinput.ui rangeinput_impl.cpp \
	authdlg.ui 		authdialogimpl.cpp ssltrustprompt.ui ssltrustprompt_impl.cpp \
			logmessage.ui logmsg_impl.cpp settings.kcfgc checkoutinfo_impl.cpp checkoutinfo.ui
