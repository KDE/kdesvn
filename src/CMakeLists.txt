ADD_EXECUTABLE(kdesvnaskpass kdesvn-askpass.cpp)

SET(kdesvnsrc
    main.cpp
    kdesvn.cpp
    urldlg.cpp
    commandline.cpp)

SET(partsrc
    kdesvn_part.cpp
    kdesvnview.cpp
    commandline_part.cpp)

KDE3_AUTOMOC(${kdesvnsrc})
KDE3_AUTOMOC(${partsrc})

ADD_SUBDIRECTORY(svnqt)
ADD_SUBDIRECTORY(ksvnwidgets)
ADD_SUBDIRECTORY(settings)
ADD_SUBDIRECTORY(svnfrontend)
ADD_SUBDIRECTORY(helpers)
ADD_SUBDIRECTORY(kdesvnd)
ADD_SUBDIRECTORY(kiosvn)
ADD_SUBDIRECTORY(pics)
ADD_SUBDIRECTORY(icons)

ADD_EXECUTABLE(kdesvn ${kdesvnsrc})

KDE3_ADD_KPART(kdesvnpart WITH_PREFIX ${partsrc})
TARGET_LINK_LIBRARIES(kdesvnpart svnfrontend kdesvnhelpers ksvnwidgets settingsdlgs kdesvncfgreader svnqt
    ${QT_AND_KDECORE_LIBS} ${KDE3_KIO_LIBRARY} ${KDE3_PART_LIBRARY} ${KDE3_UI_LIBRARY})
SET_TARGET_PROPERTIES(kdesvnpart
    PROPERTIES
    LINK_FLAGS "${APR_EXTRA_LIBFLAGS} ${APU_EXTRA_LIBFLAGS} ${LINK_NO_UNDEFINED}")

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt)

KDESVN_GENERATE_LIBTOOL_FILE(kdesvnpart)


TARGET_LINK_LIBRARIES(kdesvnaskpass ${QT_AND_KDECORE_LIBS} ${KDE3_UI_LIBRARY})
TARGET_LINK_LIBRARIES(kdesvn ${QT_AND_KDECORE_LIBS} ${KDE3_UI_LIBRARY} ${KDE3_PART_LIBRARY})

INSTALL(TARGETS kdesvn kdesvnaskpass kdesvnpart
    RUNTIME DESTINATION ${KDE3_BINDIR}
    LIBRARY DESTINATION lib/kde3)

# the resources
INSTALL(FILES kdesvnui.rc DESTINATION ${KDE3_APPSDIR}/kdesvn)
INSTALL(FILES kdesvn_part.rc DESTINATION ${KDE3_APPSDIR}/kdesvnpart)
INSTALL(FILES kdesvn.desktop DESTINATION ${KDE3_APPLICATIONSDIR})
