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

file(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

set(kded_kdesvnd_src
    kdesvnd.cpp
    kdesvnd_listener.cpp
    ksvnjobview.cpp
    ${hdr}
   )

qt5_add_dbus_adaptor( kded_kdesvnd_src org.kde.kdesvnd.xml kdesvnd.h kdesvnd)

kcoreaddons_add_plugin(kded_kdesvnd INSTALL_NAMESPACE "kf5/kded" JSON ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnd.desktop SOURCES ${kded_kdesvnd_src})
kcoreaddons_desktop_to_json(kded_kdesvnd ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnd.desktop)
set_target_properties(kded_kdesvnd PROPERTIES
    OUTPUT_NAME kdesvnd
)

target_link_libraries(kded_kdesvnd
    ksvnwidgets
    kdesvnhelpers
    kdesvncfgreader
    svnqt
    Qt5::DBus
    KF5::DBusAddons
    KF5::Notifications
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_subversion.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_subversion_toplevel.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES org.kde.kdesvnd.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} RENAME kf5_org.kde.kdesvnd.xml)

configure_file(org.kde.kdesvnd.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kdesvnd.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kdesvnd.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
