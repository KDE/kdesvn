
IF (UNIX)
  SET(QT_QMAKE_EXECUTABLE qmake-qt4)
  SET(QT_MOC_EXECUTABLE moc-qt4)
  SET(QT_UIC_EXECUTABLE uic-qt4)
  SET(CMAKE_INSTALL_PREFIX /usr)
ENDIF (UNIX)

INCLUDE(${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE(../cmakemodules/FindSubversion.cmake)

MESSAGE("Qt: ${QT_LIBRARY_DIR}")
MESSAGE("Qt: ${QT_QTCORE_LIBRARY}")

SET(SOURCES
    ../apr.cpp
    ../client_annotate.cpp
    ../client_cat.cpp
    ../client.cpp
    ../client_diff.cpp
    ../client_ls.cpp
    ../client_modify.cpp
    ../client_property.cpp
    ../client_status.cpp
    ../context.cpp
    ../datetime.cpp
    ../dirent.cpp
    ../entry.cpp
    ../exception.cpp
    ../log_entry.cpp
    ../path.cpp
    ../pool.cpp
    ../revision.cpp
    ../status.cpp
    ../targets.cpp
    ../url.cpp
    ../wc.cpp
    ../version_check.cpp
    ../lock_entry.cpp
    ../client_lock.cpp
    ../info_entry.cpp
    ../client_impl.cpp
    ../contextdata.cpp
    ../commititem.cpp
    ../repository.cpp
    ../repositorydata.cpp
    ../repositorylistener.cpp
    ../svnstream.cpp
    ../svnfilestream.cpp)

FILE(GLOB HEADERS ../*.hpp)

INCLUDE_DIRECTORIES( ../ ${QT_INCLUDES} ${APU_INCLUDE_DIR} ${APR_INCLUDE_DIR} ${SUBVERSION_INCLUDE_DIR} )

IF (WIN32)
    ADD_LIBRARY(svnqt-qt4 STATIC ${SOURCES} )
ELSE (WIN32)
    ADD_LIBRARY(svnqt-qt4 SHARED ${SOURCES} )
ENDIF (WIN32)


SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES SOVERSION 2 VERSION 2.2.2)
SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
TARGET_LINK_LIBRARIES(svnqt-qt4 ${QT_LIBRARIES} ${SUBVESION_ALL_LIBS})
IF (WIN32)
    TARGET_LINK_LIBRARIES( svnqt-qt4 wsock32.lib )
ENDIF (WIN32)

# Just a small linking test
ADD_EXECUTABLE(testlink ../testmain.cpp)
TARGET_LINK_LIBRARIES(testlink svnqt-qt4)

# install rules
INSTALL(TARGETS svnqt-qt4 DESTINATION lib)
INSTALL(FILES ${HEADERS} DESTINATION include/svnqt-qt4)
