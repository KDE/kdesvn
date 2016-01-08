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

kde4_add_executable(kdesvnaskpass askpass/kdesvn-askpass.cpp)

set(kdesvnsrc
    commandline.cpp
    kdesvn.cpp
    main.cpp
    urldlg.cpp
)

set(partsrc
    commandline_part.cpp
    kdesvn_part.cpp
    kdesvnview.cpp
)

set(partsrc ${partsrc} kdesvn_part.h kdesvnview.h commandline_part.h)
set(kdesvnsrc ${kdesvnsrc} kdesvn.h urldlg.h commandline.h)

add_subdirectory(svnqt)
add_subdirectory(ksvnwidgets)
add_subdirectory(settings)
add_subdirectory(svnfrontend)
add_subdirectory(helpers)
add_subdirectory(kdesvnd)
add_subdirectory(kiosvn)
add_subdirectory(pics)
add_subdirectory(icons)

kde4_add_executable(kdesvn ${kdesvnsrc})
kde4_add_plugin(kdesvnpart ${partsrc})

target_link_libraries(kdesvnpart svnfrontend kdesvnhelpers ksvnwidgets settingsdlgs kdesvncfgreader svnbackground svnqt
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
    )

set_target_properties(kdesvnpart
    PROPERTIES
    LINK_FLAGS "${APR_EXTRA_LDFLAGS} ${APU_EXTRA_LDFLAGS} ${LINK_NO_UNDEFINED}")

include_directories(${CMAKE_BINARY_DIR}/src/svnqt)

target_link_libraries(kdesvnaskpass #${QT_QTCORE_LIBRARY} ${QT_AND_KDECORE_LIBS} ${KDE4_KDEUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
)
target_link_libraries(kdesvn #${QT_AND_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
)

install(TARGETS kdesvn kdesvnaskpass kdesvnpart
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

# the resources
install(FILES kdesvnui.rc DESTINATION ${DATA_INSTALL_DIR}/kdesvn)
install(FILES kdesvn_part.rc DESTINATION ${DATA_INSTALL_DIR}/kdesvnpart)
install(FILES kdesvn.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kdesvnpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdesvn.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdesvn)
