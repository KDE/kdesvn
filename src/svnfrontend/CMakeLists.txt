####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
 #   http://kdesvn.alwins-world.de/                                        #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 2 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, write to the                         #
 #   Free Software Foundation, Inc.,                                       #
 #   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
 ####

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/svnfrontend/fronthelpers
                    ${CMAKE_BINARY_DIR}/src/svnqt
                    ${CMAKE_BINARY_DIR}/src/ksvnwidgets)

SET(svnfrontendsrc
    ccontextlistener.cpp
    commandexec.cpp
    copymoveview_impl.cpp
    createrepo_impl.cpp
    dummydisplay.cpp
    dumprepo_impl.cpp
    editproperty_impl.cpp
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
    fronthelpers/kmultilinedelegate.cpp
    fronthelpers/kmultilinedelegate.h
    fronthelpers/watchedprocess.cpp
    fronthelpers/watchedprocess.h
    fronthelpers/fronthelpers.h
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
    models/logitemmodel.cpp
    models/logitemmodel.h
    models/logmodelhelper.cpp
    models/logmodelhelper.h
    models/svnitemnode.cpp
    models/svnitemnode.h
    models/svnitemmodelfwd.h
    models/svnitemmodel.h
    models/svnitemmodel.cpp
    models/svnsortfilter.cpp
    models/svnsortfilter.h
    models/svndirsortfilter.cpp
    models/svndirsortfilter.h
    models/getinfothread.cpp
    models/getinfothread.h
    hotcopydlg_impl.cpp
    importdir_logmsg.cpp
    itemdisplay.cpp
    loaddmpdlg_impl.cpp
    mergedlg_impl.cpp
    modifiedthread.cpp
    opencontextmenu.cpp
    propertiesdlg.cpp
    stopdlg.cpp
    svnactions.cpp
    svnitem.cpp
    svnlogdlgimp.cpp
    tcontextlistener.cpp
    blamedisplay_impl.cpp
    threadcontextlistenerdata.cpp
    fillcachethread.cpp
    fillcachethread.h
    maintreewidget.h
    maintreewidget.cpp
    svntreeview.cpp
    svntreeview.h
    svnthread.cpp
    svnthread.h
    database/dboverview.cpp
    database/dboverview.h
    database/dbsettings.cpp
    database/dbsettings.h
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
    treeWidget.ui
    database/dboverview.ui
    database/dbsettings.ui
    )

FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

KDE_AUTOMOC(${svnfrontendsrc})
KDE_ADD_UI_FILES(frontend_ui_sources ${svnfrontendui})

ADD_LIBRARY(svnfrontend STATIC ${svnfrontendsrc} ${frontend_ui_sources} ${hdr})
SET_TARGET_PROPERTIES(svnfrontend
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(svnfrontend ksvnwidgets)
