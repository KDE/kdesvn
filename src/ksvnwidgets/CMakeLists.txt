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

include_directories(${CMAKE_BINARY_DIR}/src/svnqt)

kde4_add_ui_files(kdesvn_shared_ui
    authdlg.ui
    commitmessage.ui
    deleteform.ui
    depthform.ui
    encodingselector.ui
    revertform.ui
    ssltrustprompt.ui
)

set(ksvnwidgetsrc
    authdialogimpl.cpp
    authdialogwidget.cpp
    commitmsg_impl.cpp
    deleteform_impl.cpp
    depthselector.cpp
    diffbrowser.cpp
    diffbrowserdata.cpp
    diffsyntax.cpp
    encodingselector_impl.cpp
    models/commitmodel.cpp
    models/commitmodelhelper.cpp
    pwstorage.cpp
    revertform_impl.cpp
    ssltrustprompt_impl.cpp
    )

set(ksvnwidgethdr
    authdialogimpl.h
    authdialogwidget.h
    commitmsg_impl.h
    deleteform_impl.h
    depthselector.h
    diffbrowser.h
    diffbrowserdata.h
    diffsyntax.h
    encodingselector_impl.h
    models/commitmodel.h
    models/commitmodelhelper.h
    pwstorage.h
    revertform_impl.h
    ssltrustprompt_impl.h
)

qt4_add_dbus_interfaces(ksvnwidgetsrc ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobViewServer.xml)
qt4_add_dbus_interfaces(ksvnwidgetsrc ${KDE4_DBUS_INTERFACES_DIR}/org.kde.JobView.xml)

add_library(ksvnwidgets STATIC ${kdesvn_shared_ui} ${ksvnwidgetsrc} ${ksvnwidgethdr})
target_link_libraries(ksvnwidgets
    kdesvncfgreader
)
