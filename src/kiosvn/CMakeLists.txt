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

include_directories(
    ${CMAKE_BINARY_DIR}/src/svnqt
    ${CMAKE_BINARY_DIR}/src/ksvnwidgets
)

set(kiosvn_src
    kiosvn.cpp
    kiolistener.cpp
    kiobytestream.cpp
    ../ksvnwidgets/pwstorage.cpp
)
set(kiosvn_hdr
    kio_macros.h
    kiosvn.h
    kiolistener.h
    kiobytestream.h
    ../ksvnwidgets/pwstorage.h
)
qt4_add_dbus_interfaces( kiosvn_src ../kdesvnd/org.kde.kdesvnd.xml )

kde4_add_plugin(kio_ksvn ${kiosvn_src} ${kiosvn_hdr})

target_link_libraries(kio_ksvn
    svnqt
    kdesvnhelpers
    kdesvncfgreader
    ${KDE4_KIO_LIBS}
)

set_target_properties(kio_ksvn
    PROPERTIES
    LINK_FLAG "${LINK_NO_UNDEFINED}")

install(TARGETS kio_ksvn
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} )

file(GLOB PROTFILES *.protocol)

install(FILES ${PROTFILES}
    DESTINATION  ${SERVICES_INSTALL_DIR} )
