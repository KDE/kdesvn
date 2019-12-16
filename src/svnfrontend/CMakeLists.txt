####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
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

include_directories(${CMAKE_SOURCE_DIR}/src/svnfrontend/fronthelpers
                    ${CMAKE_SOURCE_DIR}/src/svnfrontend/background)

set(svnfrontendsrc
    EditIgnorePattern.cpp
    EditIgnorePattern.h
    blamedisplay.cpp
    ccontextlistener.cpp
    commandexec.cpp
    copymoveview_impl.cpp
    createrepodlg.cpp
    database/dboverview.cpp
    database/dboverview.h
    database/dbsettings.cpp
    database/dbsettings.h
    dummydisplay.cpp
    dumprepo_impl.cpp
    editpropsdlg.cpp
    fronthelpers/checkoutinfo_impl.cpp
    fronthelpers/checkoutinfo_impl.h
    fronthelpers/cursorstack.h
    fronthelpers/fronthelpers.h
    fronthelpers/kmultilinedelegate.cpp
    fronthelpers/kmultilinedelegate.h
    fronthelpers/propertyitem.cpp
    fronthelpers/propertyitem.h
    fronthelpers/propertylist.cpp
    fronthelpers/propertylist.h
    fronthelpers/rangeinput_impl.cpp
    fronthelpers/rangeinput_impl.h
    fronthelpers/revisionbuttonimpl.cpp
    fronthelpers/revisionbuttonimpl.h
    fronthelpers/watchedprocess.cpp
    fronthelpers/watchedprocess.h
    fronthelpers/widgetblockstack.cpp
    fronthelpers/widgetblockstack.h
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
    loaddmpdlg_impl.cpp
    maintreewidget.cpp
    maintreewidget.h
    mergedlg_impl.cpp
    models/logitemmodel.cpp
    models/logitemmodel.h
    models/logmodelhelper.cpp
    models/logmodelhelper.h
    models/svndirsortfilter.cpp
    models/svndirsortfilter.h
    models/svnitemmodel.cpp
    models/svnitemmodel.h
    models/svnitemmodelfwd.h
    models/svnitemnode.cpp
    models/svnitemnode.h
    models/svnsortfilter.cpp
    models/svnsortfilter.h
    opencontextmenu.cpp
    propertiesdlg.cpp
    stopdlg.cpp
    svnactions.cpp
    svnitem.cpp
    svnlogdlgimp.cpp
    svntreeview.cpp
    svntreeview.h
    )

set(svnfrontendui
    blamedisplay.ui
    copymoveview.ui
    createrepodlg.ui
    database/dboverview.ui
    database/dbsettings.ui
    dumprepo_dlg.ui
    editignorepattern.ui
    editpropsdlg.ui
    fronthelpers/checkoutinfo.ui
    fronthelpers/rangeinput.ui
    fronthelpers/revisionbutton.ui
    hotcopydlg.ui
    loaddmpdlg.ui
    merge_dlg.ui
    propertiesdlg.ui
    svnlogdlg.ui
    treeWidget.ui
    )

file(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

ki18n_wrap_ui(frontend_ui_sources ${svnfrontendui})


add_subdirectory(background)

add_library(svnfrontend STATIC ${svnfrontendsrc} ${frontend_ui_sources} ${hdr})

target_link_libraries(svnfrontend
    PRIVATE
        KF5::IconThemes
    PUBLIC
        KF5::ItemViews
        KF5::JobWidgets
        KF5::Service
        ksvnwidgets
        svnqt
)
target_include_directories(svnfrontend
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)
