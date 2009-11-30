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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/src/svnqt)

KDE_ADD_UI_FILES(
    kdesvn_shared_ui
    commitmessage.ui
    ssltrustprompt.ui
    authdlg.ui
    encodingselector.ui
    depthform.ui
    revertform.ui
    deleteform.ui
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
    deleteform_impl.cpp
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
    deleteform_impl.h
    models/commitmodelhelper.h
    models/commitmodel.h
)

qt4_add_dbus_interfaces(ksvnwidgetsrc ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewServer.xml)
qt4_add_dbus_interfaces(ksvnwidgetsrc ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobView.xml)

KDE_AUTOMOC(${ksvnwidgetsrc})

ADD_LIBRARY(ksvnwidgets STATIC ${kdesvn_shared_ui} ${ksvnwidgetsrc} ${ksvnwidgethdr})
SET_TARGET_PROPERTIES(ksvnwidgets
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(ksvnwidgets kdesvncfgreader)
