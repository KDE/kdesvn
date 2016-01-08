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
file(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

set(kded_kdesvnd_src
    kdesvnd.cpp
    kdesvnd_listener.cpp
    ksvnjobview.cpp
    ${hdr}
   )

qt4_add_dbus_adaptor( kded_kdesvnd_src org.kde.kdesvnd.xml kdesvnd.h kdesvnd)

kde4_add_plugin(kded_kdesvnd ${kded_kdesvnd_src})

kde4_moc_headers(kded_kdesvnd_src kdesvnd_dcop.h)

target_link_libraries(kded_kdesvnd
    ksvnwidgets
    kdesvnhelpers
    kdesvncfgreader
    svnqt
    ${KDE4_KIO_LIBS}
)

install(TARGETS kded_kdesvnd
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_subversion.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_subversion_toplevel.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnd.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kded)

install(FILES org.kde.kdesvnd.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
