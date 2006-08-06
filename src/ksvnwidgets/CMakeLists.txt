KDE3_ADD_UI_FILES(
    kdesvn_shared_ui
    logmessage.ui
    ssltrustprompt.ui
    authdlg.ui
)

SET(ksvnwidgetsrc
 authdialogimpl.cpp logmsg_impl.cpp ssltrustprompt_impl.cpp)

KDE3_AUTOMOC(${ksvnwidgetsrc})

ADD_LIBRARY(ksvnwidgets STATIC ${ksvnwidgetsrc} ${kdesvn_shared_ui})
ADD_DEPENDENCIES(ksvnwidgets kdesvncfgreader)
