SET(kded_kdesvnd_src
   kdesvnd_dcop.cpp)

KDE3_ADD_DCOP_SKELS(kded_kdesvnd_src
 kdesvnd_dcop.h)

KDE3_ADD_KPART(kded_kdesvnd ${kded_kdesvnd_src})
TARGET_LINK_LIBRARIES(kded_kdesvnd
 svnqt
 ksvnwidgets
 kdesvnhelpers
 kdesvncfgreader
 ${QT_AND_KDECORE_LIBS} ${KDE3_KIO_LIBRARY} ${KDE3_DCOP_LIBRARY} ${KDE3_UI_LIBRARY}
 )

SET_TARGET_PROPERTIES(kded_kdesvnd
	PROPERTIES
	LINK_FLAGS "${APR_EXTRA_LIBFLAGS} ${APU_EXTRA_LIBFLAGS} -avoid-version -module")

INSTALL(TARGETS kded_kdesvnd
    RUNTIME DESTINATION ${KDE3_BINDIR}
    LIBRARY DESTINATION lib/kde3
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)

SET(deskfile "kdesvn_subversion.desktop")
SET(templatedesk "${CMAKE_CURRENT_SOURCE_DIR}/${deskfile}.template")

ADD_CUSTOM_COMMAND(TARGET kded_kdesvnd
    POST_BUILD
	COMMAND "${kdesvn_SOURCE_DIR}/src/kdesvnd/test.kde.sh"
	ARGS "${templatedesk}" "${CMAKE_CURRENT_BINARY_DIR}/${deskfile}"
	DEPENDS "${templatedesk}"
	)
KDESVN_GENERATE_LIBTOOL_FILE(kded_kdesvnd)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${deskfile}
    DESTINATION ${KDE3_APPSDIR}/konqueror/servicemenus)
