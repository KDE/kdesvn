INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

KDE3_ADD_UI_FILES(
    kdesvn_shared_ui
    logmessage.ui
    ssltrustprompt.ui
    authdlg.ui
)

SET(ksvnwidgetsrc
    authdialogimpl.cpp
    logmsg_impl.cpp
    ssltrustprompt_impl.cpp)

KDE3_AUTOMOC(${ksvnwidgetsrc})

ADD_LIBRARY(ksvnwidgets STATIC ${ksvnwidgetsrc} ${kdesvn_shared_ui})
SET_TARGET_PROPERTIES(ksvnwidgets
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(ksvnwidgets kdesvncfgreader)
