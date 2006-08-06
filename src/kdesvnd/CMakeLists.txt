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
    LIBRARY DESTINATION lib/kde3)

