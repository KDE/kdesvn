INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/src/svnqt)

KDE_ADD_UI_FILES(
    kdesvn_shared_ui
    logmessage.ui
    ssltrustprompt.ui
    authdlg.ui
)

SET(ksvnwidgetsrc
    authdialogimpl.cpp
    logmsg_impl.cpp
    ssltrustprompt_impl.cpp
    pwstorage.cpp
    diffbrowser.cpp
    diffbrowserdata.cpp
    diffsyntax.cpp
    )

SET(ksvnwidgethdr
    authdialogimpl.h
    logmsg_impl.h
    ssltrustprompt_impl.h
    pwstorage.h
    diffbrowser.h
    diffbrowserdata.h
    diffsyntax.h
)

KDE_AUTOMOC(${ksvnwidgetsrc})

ADD_LIBRARY(ksvnwidgets STATIC ${ksvnwidgetsrc} ${kdesvn_shared_ui} ${ksvnwidgethdr})
SET_TARGET_PROPERTIES(ksvnwidgets
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(ksvnwidgets kdesvncfgreader)
