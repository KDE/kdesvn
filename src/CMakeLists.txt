IF (NOT BUILD_KDE4)
add_executable(kdesvnaskpass askpass/kdesvn-askpass.cpp)
ELSE (NOT BUILD_KDE4)
kde4_add_executable(kdesvnaskpass askpass/kdesvn-askpass.cpp)
ENDIF (NOT BUILD_KDE4)

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

IF (NOT BUILD_KDE4)
ADD_EXECUTABLE(kdesvn ${kdesvnsrc})
ELSE (NOT BUILD_KDE4)
KDE4_ADD_EXECUTABLE(kdesvn ${kdesvnsrc})
ENDIF (NOT BUILD_KDE4)

KDE_ADD_KPART(kdesvnpart ${partsrc})

SET(QT_LIBRARIES "${QT_QTCORE_LIBRARY}" "${QT_QTGUI_LIBRARY}" )

TARGET_LINK_LIBRARIES(kdesvnpart svnfrontend kdesvnhelpers ksvnwidgets settingsdlgs kdesvncfgreader svnqt4
    ${QT_LIBRARIES}
    ${KDE4_KDECORE_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_KIO_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
    )
SET_TARGET_PROPERTIES(kdesvnpart
    PROPERTIES
    LINK_FLAGS "${_BASE_LDADD} ${APR_EXTRA_LIBFLAGS} ${APU_EXTRA_LIBFLAGS} ${LINK_NO_UNDEFINED}")

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
install(FILES kdesvnpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
