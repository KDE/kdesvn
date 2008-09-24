INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

KDE_ADD_KCFG_FILES(cfgsrc
    kdesvnsettings.kcfgc)
ADD_LIBRARY(kdesvncfgreader STATIC ${cfgsrc})

KDE_ADD_UI_FILES(
    settings_ui_sources
    dispcolor_settings.ui
    display_settings.ui
    revisiontree_settings.ui
    subversion_settings.ui
    cmdexecsettings.ui
    diffmergesettings.ui
)

SET(settings_sources
    dispcolorsettings_impl.cpp
    displaysettings_impl.cpp
    revisiontreesettingsdlg_impl.cpp
    subversionsettings_impl.cpp
    cmdexecsettings_impl.cpp
    diffmergesettings_impl.cpp)

FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

KDE_AUTOMOC(${settings_sources})
ADD_LIBRARY(settingsdlgs STATIC ${settings_ui_sources} ${settings_sources} ${hdr})
ADD_DEPENDENCIES(settingsdlgs kdesvncfgreader)

SET_TARGET_PROPERTIES(kdesvncfgreader settingsdlgs
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_part.kcfg DESTINATION ${KCFG_INSTALL_DIR})

INSTALL(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnpartrc-use-external.upd
    DESTINATION
    ${DATA_INSTALL_DIR}/kconf_update
)

INSTALL(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn-use-external-update.sh
    DESTINATION
    ${DATA_INSTALL_DIR}/kconf_update
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)
