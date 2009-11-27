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

KDE4_ADD_EXECUTABLE(kdesvnaskpass askpass/kdesvn-askpass.cpp)

SET(kdesvnsrc
    main.cpp
    kdesvn.cpp
    urldlg.cpp
    commandline.cpp
)

SET(partsrc
    kdesvn_events.cpp
    kdesvn_part.cpp
    kdesvnview.cpp
    commandline_part.cpp
)

KDE_AUTOMOC(${kdesvnsrc})
KDE_AUTOMOC(${partsrc})

SET(partsrc ${partsrc} kdesvn_part.h kdesvnview.h commandline_part.h)
SET(kdesvnsrc ${kdesvnsrc} kdesvn.h urldlg.h commandline.h)

ADD_SUBDIRECTORY(svnqt)
ADD_SUBDIRECTORY(ksvnwidgets)
ADD_SUBDIRECTORY(settings)
ADD_SUBDIRECTORY(svnfrontend)
ADD_SUBDIRECTORY(helpers)
ADD_SUBDIRECTORY(kdesvnd)
ADD_SUBDIRECTORY(kiosvn)
ADD_SUBDIRECTORY(pics)
ADD_SUBDIRECTORY(icons)

KDE4_ADD_EXECUTABLE(kdesvn ${kdesvnsrc})
KDE_ADD_KPART(kdesvnpart ${partsrc})

SET(QT_LIBRARIES "${QT_QTCORE_LIBRARY}" "${QT_QTGUI_LIBRARY}" )

TARGET_LINK_LIBRARIES(kdesvnpart svnfrontend kdesvnhelpers ksvnwidgets settingsdlgs kdesvncfgreader ${svnqt-name}
    ${QT_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
    )
SET_TARGET_PROPERTIES(kdesvnpart
    PROPERTIES
    LINK_FLAGS "${_BASE_LDADD} ${APR_EXTRA_LDFLAGS} ${APU_EXTRA_LDFLAGS} ${LINK_NO_UNDEFINED}")

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt)

TARGET_LINK_LIBRARIES(kdesvnaskpass #${QT_QTCORE_LIBRARY} ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBRARY}
    ${QT_LIBRARIES} ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY}
)
TARGET_LINK_LIBRARIES(kdesvn #${QT_AND_KDECORE_LIBS}
    ${QT_LIBRARIES} ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
)

INSTALL(TARGETS kdesvn kdesvnaskpass kdesvnpart
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

# the resources
INSTALL(FILES kdesvnui.rc DESTINATION ${DATA_INSTALL_DIR}/kdesvn)
INSTALL(FILES kdesvn_part.rc DESTINATION ${DATA_INSTALL_DIR}/kdesvnpart)
INSTALL(FILES kdesvn.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES kdesvnpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
INSTALL(FILES kdesvn.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdesvn)
