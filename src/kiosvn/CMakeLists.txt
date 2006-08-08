SET(kiosvn_src kiosvn.cpp kiolistener.cpp)

KDE3_ADD_KPART(kio_ksvn ${kiosvn_src})

TARGET_LINK_LIBRARIES(kio_ksvn 
    svnqt
    kdesvnhelpers
    ${QT_AND_KDECORE_LIBS}
    ${KDE3_UI_LIBRARY} ${KDE3_DCOP_LIBRARY} ${KDE3_KIO_LIBRARY})

SET_TARGET_PROPERTIES(kio_ksvn
   PROPERTIES
   LINK_FLAG "${LINK_NO_UNDEFINED}")

INSTALL(TARGETS kio_ksvn
    LIBRARY DESTINATION lib/kde3
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)

SET(PROTFILES ksvn+http.protocol ksvn+https.protocol ksvn+file.protocol ksvn+ssh.protocol ksvn.protocol 
   svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol)

INSTALL(FILES ${PROTFILES}
   DESTINATION ${KDE3_SERVICESDIR})
