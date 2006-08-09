
SET(QT_QMAKE_EXECUTABLE qmake-qt4)
SET(QT_MOC_EXECUTABLE moc-qt4)
SET(QT_UIC_EXECUTABLE uic-qt4)
SET(CMAKE_INSTALL_PREFIX /usr)

include(${CMAKE_ROOT}/Modules/FindQt4.cmake)
include(${CMAKE_ROOT}/Modules/UseQt4.cmake)
include(../cmakemodules/FindSubversion.cmake)

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
    ../client_property.cpp ../client_status.cpp
    ../context.cpp ../datetime.cpp ../dirent.cpp ../entry.cpp ../exception.cpp ../log_entry.cpp ../path.cpp
    ../pool.cpp ../revision.cpp ../status.cpp ../targets.cpp ../url.cpp ../wc.cpp ../version_check.cpp
    ../lock_entry.cpp ../client_lock.cpp ../info_entry.cpp ../client_impl.cpp ../contextdata.cpp
    ../commititem.cpp ../repository.cpp ../repositorydata.cpp
    ../repositorylistener.cpp ../svnstream.cpp ../svnfilestream.cpp)

FILE(GLOB HEADERS ../*.hpp)

INCLUDE_DIRECTORIES( ../ ${QT_INCLUDES} ${APU_INCLUDE_DIR} ${APR_INCLUDE_DIR} ${SUBVERSION_INCLUDE_DIR} )

ADD_LIBRARY(svnqt-qt4 SHARED ${SOURCES} )
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES SOVERSION 2 VERSION 2.2.2)
SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
target_link_libraries(svnqt-qt4 ${QT_LIBRARIES} ${SUBVESION_ALL_LIBS})

# Just a small linking test
ADD_EXECUTABLE(testlink ../testmain.cpp)
target_link_libraries(testlink svnqt-qt4)
IF (WIN32)
  target_link_libraries( svnqt-qt4 wsock32.lib )
ENDIF (WIN32)

# install rules
INSTALL(TARGETS svnqt-qt4
    LIBRARY DESTINATION lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
install_files(/include/svnqt-qt4 FILES ${HEADERS})
