INCLUDE(FindQt4)
INCLUDE(UseQt4)
INCLUDE(../cmakemodules/FindSubversion.cmake)

IF(QT4_FOUND)
    MESSAGE(STATUS "Qt: ${QT_LIBRARY_DIR}")
    MESSAGE(STATUS "Qt: ${QT_QTCORE_LIBRARY}")
    SET(TOUTF8 "toUtf8")
    SET(FROMUTF8 "fromUtf8")
    SET(QLIST "QList")
ELSE(QT4_FOUND)
    MESSAGE(FATAL_ERROR "No qt4 found!")
ENDIF(QT4_FOUND)

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

INCLUDE_DIRECTORIES( ../../
    ${QT_INCLUDES}
    ${APU_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
    )

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

SET(HEADERS ${HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/svnqt/svnqt_defines.hpp)

MESSAGE(STATUS "Current source: ${CMAKE_CURRENT_SOURCE_DIR}")
IF(QT4_FOUND)
    MESSAGE(STATUS "Qt: ${QT_LIBRARY_DIR}")
    MESSAGE(STATUS "Qt: ${QT_QTCORE_LIBRARY}")
    SET(TOUTF8 "toUtf8")
    SET(FROMUTF8 "fromUtf8")
    SET(QLIST "QList")
ELSE(QT4_FOUND)
    IF(QT_FOUND)
        SET(TOUTF8 "utf8")
        SET(FROMUTF8 "fromUtf8")
        SET(QLIST "QValueList")
    ENDIF(QT_FOUND)
ENDIF(QT4_FOUND)

IF (HAVE_GCC_VISIBILITY)
    SET(_SVNQT_EXPORT "__attribute__ ((visibility(\"default\")))")
    SET(_SVNQT_NOEXPORT "__attribute__ ((visibility(\"hidden\")))")
ENDIF (HAVE_GCC_VISIBILITY)
IF (WIN32)
    SET(_SVNQT_EXPORT "__declspec(dllexport)")
ENDIF (WIN32)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/../svnqt_defines.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/svnqt/svnqt_defines.hpp"
    IMMEDIATE
    @ONLY)

ADD_LIBRARY(svnqt-qt4 SHARED ${SOURCES} )
IF (WIN32)
    ADD_DEFINITIONS(-D_USE_32BIT_TIME_T)
ELSE (WIN32)
    SET_TARGET_PROPERTIES(svnqt-qt4
        PROPERTIES
        COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
ENDIF (WIN32)


SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES SOVERSION 3 VERSION 3.1.3)
SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
SET_TARGET_PROPERTIES(svnqt-qt4 PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
TARGET_LINK_LIBRARIES(svnqt-qt4 ${QT_LIBRARIES} ${SUBVERSION_ALL_LIBS})
IF (WIN32)
    TARGET_LINK_LIBRARIES( svnqt-qt4 wsock32.lib )
ENDIF (WIN32)

# Just a small linking test

IF (BUILD_TESTS)
    ADD_EXECUTABLE(testlink ../testmain.cpp)
    TARGET_LINK_LIBRARIES(testlink svnqt-qt4)
ENDIF (BUILD_TESTS)

# install rules
IF ( UNIX )
    INSTALL(TARGETS svnqt-qt4 DESTINATION lib)
    INSTALL(FILES ${HEADERS} DESTINATION include/svnqt-qt4)
ENDIF ( UNIX )
