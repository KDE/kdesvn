INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt)
FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

SET(kded_kdesvnd_src
    kdesvnd.cpp
    ${hdr}
   )

qt4_add_dbus_adaptor( kded_kdesvnd_src org.kde.kdesvnd.xml kdesvnd.h kdesvnd)

KDE4_ADD_PLUGIN(kded_kdesvnd ${kded_kdesvnd_src})


KDE_ADD_MOC_FILES(kded_kdesvnd_src kdesvnd_dcop.h)

TARGET_LINK_LIBRARIES(kded_kdesvnd
    ${svnqt-name}
    ksvnwidgets
    kdesvnhelpers
    kdesvncfgreader
    ${KDE4_KIO_LIBS}
)

INSTALL(TARGETS kded_kdesvnd
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvn_subversion.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnd.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kded)

install( FILES org.kde.kdesvnd.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
