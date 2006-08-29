INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/svnfrontend/fronthelpers)

SET(svnfrontendsrc
    cacheentry.cpp
    ccontextlistener.cpp
    commandexec.cpp
    copymoveview_impl.cpp
    createrepo_impl.cpp
    dummydisplay.cpp
    dumprepo_impl.cpp
    editproperty_impl.cpp
    filelistviewitem.cpp
    fronthelpers/checkoutinfo_impl.cpp
    fronthelpers/oimagescrollview.cpp
    fronthelpers/oimagezoomer.cpp
    fronthelpers/rangeinput_impl.cpp
    graphtree/drawparams.cpp
    graphtree/elogentry.cpp
    graphtree/graphtreelabel.cpp
    graphtree/pannerview.cpp
    graphtree/revgraphview.cpp
    graphtree/revisiontree.cpp
    graphtree/revtreewidget.cpp
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
    threadcontextlistenerdata.cpp)

SET(svnfrontendui
    copymoveview.ui
    createrepo_dlg.ui
    dumprepo_dlg.ui
    editpropsdlg.ui
    fronthelpers/checkoutinfo.ui
    fronthelpers/rangeinput.ui
    hotcopydlg.ui
    loaddmpdlg.ui
    merge_dlg.ui
    svnlogdlg.ui)

KDE3_AUTOMOC(${svnfrontendsrc})
KDE3_ADD_UI_FILES(frontend_ui_sources ${svnfrontendui})

ADD_LIBRARY(svnfrontend STATIC ${svnfrontendsrc} ${frontend_ui_sources})
SET_TARGET_PROPERTIES(svnfrontend
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(svnfrontend ksvnwidgets)
