INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/svnfrontend/fronthelpers ${CMAKE_BINARY_DIR}/src/svnqt)

SET(svnfrontendsrc
    ccontextlistener.cpp
    commandexec.cpp
    copymoveview_impl.cpp
    createrepo_impl.cpp
    dummydisplay.cpp
    dumprepo_impl.cpp
    editproperty_impl.cpp
    filelistviewitem.cpp
    fronthelpers/checkoutinfo_impl.cpp
    fronthelpers/checkoutinfo_impl.h
    fronthelpers/rangeinput_impl.cpp
    fronthelpers/rangeinput_impl.h
    fronthelpers/revisionbuttonimpl.cpp
    fronthelpers/revisionbuttonimpl.h
    fronthelpers/cursorstack.h
    fronthelpers/widgetblockstack.cpp
    fronthelpers/widgetblockstack.h
    fronthelpers/propertylist.cpp
    fronthelpers/propertylist.h
    fronthelpers/propertyitem.cpp
    fronthelpers/propertyitem.h
    graphtree/drawparams.cpp
    graphtree/drawparams.h
    graphtree/elogentry.cpp
    graphtree/elogentry.h
    graphtree/graphtreelabel.cpp
    graphtree/graphtreelabel.h
    graphtree/pannerview.cpp
    graphtree/pannerview.h
    graphtree/revgraphview.cpp
    graphtree/revgraphview.h
    graphtree/revisiontree.cpp
    graphtree/revisiontree.h
    graphtree/revtreewidget.cpp
    graphtree/revtreewidget.h
    hotcopydlg_impl.cpp
    importdir_logmsg.cpp
    itemdisplay.cpp
    kdesvnfilelist.cpp
    keystatus.cpp
    loaddmpdlg_impl.cpp
    mergedlg_impl.cpp
    modifiedthread.cpp
    opencontextmenu.cpp
    propertiesdlg.cpp
    stopdlg.cpp
    svnactions.cpp
    svnfiletip.cpp
    svnitem.cpp
    svnlogdlgimp.cpp
    tcontextlistener.cpp
    blamedisplay_impl.cpp
    threadcontextlistenerdata.cpp
    leftpane/leftpane_impl.cpp
    leftpane/leftpane_impl.h
    )

SET(svnfrontendui
    copymoveview.ui
    createrepo_dlg.ui
    dumprepo_dlg.ui
    editpropsdlg.ui
    fronthelpers/checkoutinfo.ui
    fronthelpers/rangeinput.ui
    fronthelpers/revisionbutton.ui
    hotcopydlg.ui
    loaddmpdlg.ui
    merge_dlg.ui
    blamedisplay.ui
    svnlogdlg.ui
    leftpane/leftpane.ui
    )

FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

KDE3_AUTOMOC(${svnfrontendsrc})
KDE3_ADD_UI_FILES(frontend_ui_sources ${svnfrontendui})

ADD_LIBRARY(svnfrontend STATIC ${svnfrontendsrc} ${frontend_ui_sources} ${hdr})
SET_TARGET_PROPERTIES(svnfrontend
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(svnfrontend ksvnwidgets)
