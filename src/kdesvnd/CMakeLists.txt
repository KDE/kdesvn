INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/svnqt)

SET(kded_kdesvnd_src
    kdesvnd_dcop.cpp)

KDE3_ADD_MOC_FILES(kded_kdesvnd_src kdesvnd_dcop.h)

KDE3_ADD_DCOP_SKELS(kded_kdesvnd_src
    kdesvnd_dcop.h)

KDE3_ADD_KPART(kded_kdesvnd ${kded_kdesvnd_src})

TARGET_LINK_LIBRARIES(kded_kdesvnd
    svnqt
    ksvnwidgets
    kdesvnhelpers
    kdesvncfgreader
    ${QT_AND_KDECORE_LIBS} ${KDE3_KIO_LIBRARY} ${KDE3_DCOP_LIBRARY} ${KDE3_UI_LIBRARY})

SET_TARGET_PROPERTIES(kded_kdesvnd
    PROPERTIES
    LINK_FLAGS "${APR_EXTRA_LIBFLAGS} ${APU_EXTRA_LIBFLAGS} -avoid-version -module")

INSTALL(TARGETS kded_kdesvnd
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})

KDESVN_GENERATE_LIBTOOL_FILE(kded_kdesvnd)

IF(KDE3_KDECONFIG_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND ${KDE3_KDECONFIG_EXECUTABLE} "--version" OUTPUT_VARIABLE output)
    STRING(REGEX MATCH "KDE: [0-9]+\\.[0-9]+\\.[0-9]+" KDEVERSION "${output}")
    STRING(REGEX REPLACE "^KDE: " "" KDEVERSION "${KDEVERSION}")
ENDIF(KDE3_KDECONFIG_EXECUTABLE)
SET(KDE_MIN_VERSION "3.5.0")
MACRO_ENSURE_VERSION(${KDE_MIN_VERSION} ${KDEVERSION} VERSION_OK)

IF(VERSION_OK)
    SET(ACTION_MENU "X-KDE-GetActionMenu=kded kdesvnd getActionMenu(KURL::List)")
ELSE(VERSION_OK)
    SET(ACTION_MENU "Actions=Update;Commit;Add;Delete;Blame;Log;Diff;Checkout;Export;Info;Addnew;Switch;Revert;Rename;Tree")
ENDIF(VERSION_OK)

CONFIGURE_FILE(kdesvn_subversion.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/kdesvn_subversion.desktop @ONLY)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdesvn_subversion.desktop
    DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kdesvnd.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kded)
