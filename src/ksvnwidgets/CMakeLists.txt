INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/src/svnqt)

KDE_ADD_UI_FILES(
    kdesvn_shared_ui
    commitmessage.ui
    ssltrustprompt.ui
    authdlg.ui
    encodingselector.ui
    depthform.ui
    revertform.ui
)

SET(ksvnwidgetsrc
    authdialogimpl.cpp
    authdialogwidget.cpp
    commitmsg_impl.cpp
    ssltrustprompt_impl.cpp
    pwstorage.cpp
    diffbrowser.cpp
    diffbrowserdata.cpp
    diffsyntax.cpp
    encodingselector_impl.cpp
    depthselector.cpp
    revertform_impl.cpp
    models/commitmodelhelper.cpp
    models/commitmodel.cpp
    )

SET(ksvnwidgethdr
    authdialogimpl.h
    authdialogwidget.h
    commitmsg_impl.h
    ssltrustprompt_impl.h
    pwstorage.h
    diffbrowser.h
    diffbrowserdata.h
    diffsyntax.h
    encodingselector_impl.h
    depthselector.h
    revertform_impl.h
    models/commitmodelhelper.h
    models/commitmodel.h
)

KDE_AUTOMOC(${ksvnwidgetsrc})

ADD_LIBRARY(ksvnwidgets STATIC ${kdesvn_shared_ui} ${ksvnwidgetsrc} ${ksvnwidgethdr})
SET_TARGET_PROPERTIES(ksvnwidgets
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(ksvnwidgets kdesvncfgreader)
