INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt
${CMAKE_BINARY_DIR}/src/ksvnwidgets)

SET(kiosvn_src kiosvn.cpp kiolistener.cpp kiobytestream.cpp ../ksvnwidgets/pwstorage.cpp)
FILE(GLOB hdr RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
qt4_add_dbus_interfaces( kiosvn_src ../kdesvnd/org.kde.kdesvnd.xml )

KDE_ADD_KPART(kio_ksvn ${kiosvn_src} ${hdr})

TARGET_LINK_LIBRARIES(kio_ksvn
    svnqt4
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
