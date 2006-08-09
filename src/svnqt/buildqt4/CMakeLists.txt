SET(CMAKE_INSTALL_PREFIX /usr)

include(${CMAKE_ROOT}/Modules/FindQt4.cmake)
include(../cmakemodules/FindSubversion.cmake)

MESSAGE("Qt: ${QT_LIBRARY_DIR}")
MESSAGE("Qt: ${QT_QTCORE_LIBRARY}")

FILE(GLOB SOURCES ../*.cpp)
FILE(GLOB HEADERS ../*.hpp)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/../ ${QT_INCLUDES} ${APU_INCLUDE_DIR} ${APR_INCLUDE_DIR} ${SUBVERSION_INCLUDE_DIR} )

ADD_LIBRARY(svnqt-qt4 SHARED ${SOURCES} )
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES SOVERSION 2 VERSION 2.2.1)
SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS}")
target_link_libraries(svnqt-qt4 ${QT_QTCORE_LIBRARY} ${SUBVESION_ALL_LIBS})
IF (WIN32)
  target_link_libraries( svnqt-qt4 wsock32.lib )
ENDIF (WIN32)

# Just a small linking test
ADD_EXECUTABLE(testlink testmain.cpp)
target_link_libraries(testlink svnqt-qt4)

# install rules
install_targets(/lib svnqt-qt4 svnqt-qt4)
install_files(/include/svnqt FILES ${HEADERS})
