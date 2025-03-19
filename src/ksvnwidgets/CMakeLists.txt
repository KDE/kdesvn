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

qt_add_dbus_interfaces(ksvnwidgetsrc ${KJOBWIDGETS_DBUS_INTERFACES_DIR}/kf6_org.kde.JobViewServer.xml)
qt_add_dbus_interfaces(ksvnwidgetsrc ${KJOBWIDGETS_DBUS_INTERFACES_DIR}/kf6_org.kde.JobView.xml)

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
        KF6::Wallet
    PUBLIC
        svnqt
        Qt::Widgets
        Qt::Core5Compat
        KF6::Codecs
        KF6::Completion
        KF6::ConfigWidgets
        KF6::I18n
        KF6::KIOWidgets
        KF6::Parts
        KF6::TextWidgets
        KF6::XmlGui
)
