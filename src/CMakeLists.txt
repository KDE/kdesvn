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

include(ECMMarkNonGuiExecutable)
include(ECMAddAppIcon)

add_executable(kdesvnaskpass askpass/kdesvn-askpass.cpp)
ecm_mark_nongui_executable(kdesvnaskpass)

ki18n_wrap_ui(kdesvn_ui
    urldlg.ui
)
set(kdesvnsrc
    commandline.cpp
    kdesvn.cpp
    main.cpp
    urldlg.cpp

    kdesvn.qrc
)

# Sets the icon on Windows and OSX
find_program(KSVG2ICNS NAMES ksvg2icns)
if(APPLE AND KSVG2ICNS)
    execute_process(COMMAND ${KSVG2ICNS} "${CMAKE_CURRENT_SOURCE_DIR}/icons/sc-apps-kdesvn.svgz"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} RESULT_VARIABLE KSVG2ICNS_ERROR)
    if(${KSVG2ICNS_ERROR})
        message(SEND_ERROR "Could not generate OS X application icon")
    else()
        set(MACOSX_BUNDLE_ICON_FILE sc-apps-kdesvn.icns)
        set(kdesvnsrc "${kdesvnsrc};${CMAKE_CURRENT_BINARY_DIR}/sc-apps-kdesvn.icns")
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/sc-apps-kdesvn.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()
else()
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kdesvn.png")
    ecm_add_app_icon(kdesvnsrc ICONS ${ICONS_SRCS})
endif()

set(partsrc
    kdesvn_part.cpp
    kdesvnview.cpp

    kdesvn_part.qrc
)

set(partsrc ${partsrc} kdesvn_part.h kdesvnview.h)
set(kdesvnsrc ${kdesvnsrc} ${kdesvn_ui} kdesvn.h urldlg.h commandline.h)

add_subdirectory(svnqt)
add_subdirectory(ksvnwidgets)
add_subdirectory(settings)
add_subdirectory(svnfrontend)
add_subdirectory(helpers)
add_subdirectory(kdesvnd)
add_subdirectory(kiosvn)
add_subdirectory(pics)
add_subdirectory(icons)
add_subdirectory(fileitemactions)

add_executable(kdesvn ${kdesvnsrc})

kcoreaddons_add_plugin(kdesvnpart SOURCES ${partsrc} INSTALL_NAMESPACE "kf6/parts")

# TODO: Re-enable the desktop icon.
# See also https://phabricator.kde.org/T14564 for some hints
# And also re-enable the installation of the icon below.
#kcoreaddons_desktop_to_json(kdesvnpart kdesvnpart.desktop)

target_link_libraries(kdesvnpart
    svnfrontend
    kdesvnhelpers
    ksvnwidgets
    settingsdlgs
    kdesvncfgreader
    svnbackground
    svnqt
)

set_target_properties(kdesvnpart
    PROPERTIES
    LINK_FLAGS "${APR_EXTRA_LDFLAGS} ${APU_EXTRA_LDFLAGS} ${LINK_NO_UNDEFINED}")

target_link_libraries(kdesvnaskpass
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
    KF6::Wallet
)
target_link_libraries(kdesvn
    KF6::Parts
    KF6::Bookmarks
    KF6::BookmarksWidgets
    kdesvnhelpers
    svnfrontend
    svnbackground
)

install(TARGETS kdesvn kdesvnaskpass
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# the resources
install(FILES org.kde.kdesvn.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.kdesvn.desktop DESTINATION ${KDE_INSTALL_APPDIR})
# TODO See the TODO above...
#install(FILES kdesvnpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICESDIR})
install(FILES kdesvn.notifyrc DESTINATION ${KDE_INSTALL_DATADIR}/kdesvn)
