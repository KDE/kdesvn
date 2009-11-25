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

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt
${CMAKE_BINARY_DIR}/src/ksvnwidgets)

SET(kiosvn_src kiosvn.cpp kiolistener.cpp kiobytestream.cpp ../ksvnwidgets/pwstorage.cpp)
FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
qt4_add_dbus_interfaces( kiosvn_src ../kdesvnd/org.kde.kdesvnd.xml )

KDE_ADD_KPART(kio_ksvn ${kiosvn_src} ${hdr})

TARGET_LINK_LIBRARIES(kio_ksvn
    ${svnqt-name}
    kdesvnhelpers
    kdesvncfgreader
    ${KDE4_KIO_LIBS}
)

SET_TARGET_PROPERTIES(kio_ksvn
    PROPERTIES
    LINK_FLAG "${LINK_NO_UNDEFINED}  ${_BASE_LDADD}")

INSTALL(TARGETS kio_ksvn
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )

FILE(GLOB PROTFILES *.protocol)

INSTALL(FILES ${PROTFILES}
    DESTINATION  ${SERVICES_INSTALL_DIR} )
