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

kconfig_add_kcfg_files(cfgsrc
    kdesvnsettings.kcfgc)
add_library(kdesvncfgreader STATIC ${cfgsrc})
target_link_libraries(kdesvncfgreader
    KF5::ConfigGui
)

ki18n_wrap_ui(settings_ui_sources
    cmdexecsettings.ui
    diffmergesettings.ui
    dispcolor_settings.ui
    display_settings.ui
    polling_settings.ui
    revisiontree_settings.ui
    subversion_settings.ui
)

set(settings_sources
    cmdexecsettings_impl.cpp
    diffmergesettings_impl.cpp
    dispcolorsettings_impl.cpp
    displaysettings_impl.cpp
    polling_settings_impl.cpp
    revisiontreesettingsdlg_impl.cpp
    subversionsettings_impl.cpp
    )

file(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

add_library(settingsdlgs STATIC ${settings_ui_sources} ${settings_sources} ${hdr})
target_link_libraries(settingsdlgs
    PRIVATE
        kdesvncfgreader
    PUBLIC
        KF5::WidgetsAddons
        KF5::Completion
        KF5::I18n
)
target_include_directories(settingsdlgs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_BINARY_DIR}/../
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_part.kcfg DESTINATION ${KCFG_INSTALL_DIR})

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnpartrc-use-external.upd
    DESTINATION
    ${DATA_INSTALL_DIR}/kconf_update
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn-use-external-update.sh
    DESTINATION
    ${DATA_INSTALL_DIR}/kconf_update
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)
