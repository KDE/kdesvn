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

ki18n_wrap_ui(kdesvn_shared_ui
    authdlg.ui
    commitmessage.ui
    deleteform.ui
    depthselector.ui
    encodingselector.ui
    revertform.ui
    ssltrustprompt.ui
)

set(ksvnwidgetsrc
    authdialogimpl.cpp
    authdialogwidget.cpp
    commitmsg_impl.cpp
    deleteform.cpp
    depthselector.cpp
    diffbrowser.cpp
    diffsyntax.cpp
    encodingselector_impl.cpp
    ksvndialog.cpp
    models/commitmodel.cpp
    models/commitmodelhelper.cpp
    pwstorage.cpp
    revertform.cpp
    ssltrustprompt.cpp
    )

set(ksvnwidgethdr
    authdialogimpl.h
    authdialogwidget.h
    commitmsg_impl.h
    deleteform.h
    depthselector.h
    diffbrowser.h
    diffsyntax.h
    encodingselector_impl.h
    ksvndialog.h
    models/commitmodel.h
    models/commitmodelhelper.h
    pwstorage.h
    revertform.h
    ssltrustprompt.h
)

qt5_add_dbus_interfaces(ksvnwidgetsrc ${KJOBWIDGETS_DBUS_INTERFACES_DIR}/kf5_org.kde.JobViewServer.xml)
qt5_add_dbus_interfaces(ksvnwidgetsrc ${KJOBWIDGETS_DBUS_INTERFACES_DIR}/kf5_org.kde.JobView.xml)

add_library(ksvnwidgets STATIC ${kdesvn_shared_ui} ${ksvnwidgetsrc} ${ksvnwidgethdr})
target_include_directories(ksvnwidgets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_BINARY_DIR}/../
)
target_link_libraries(ksvnwidgets
    PRIVATE
        kdesvnhelpers
        kdesvncfgreader
        KF5::Wallet
    PUBLIC
        svnqt
        Qt5::Widgets
        KF5::Completion
        KF5::ConfigWidgets
        KF5::I18n
        KF5::KIOWidgets
        KF5::Parts
        KF5::TextWidgets
        KF5::XmlGui
)
