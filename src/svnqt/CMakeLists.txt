SET(SOURCES
    apr.cpp
    client_annotate.cpp
    client_cat.cpp
    client.cpp
    client_diff.cpp
    client_ls.cpp
    client_modify.cpp
    client_property.cpp
    client_status.cpp
    context.cpp
    datetime.cpp
    dirent.cpp
    entry.cpp
    exception.cpp
    log_entry.cpp
    path.cpp
    pool.cpp
    revision.cpp
    status.cpp
    targets.cpp
    url.cpp
    version_check.cpp
    wc.cpp
    lock_entry.cpp
    client_lock.cpp
    info_entry.cpp
    client_impl.cpp
    contextdata.cpp
    commititem.cpp
    repository.cpp
    repositorydata.cpp
    repositorylistener.cpp
    svnstream.cpp
    diff_data.cpp
    svnfilestream.cpp
    stringarray.cpp
    diffoptions.cpp
    conflictdescription.cpp
    conflictresult.cpp
    client_merge.cpp
    cache/LogCache.cpp
    cache/ReposLog.cpp
    cache/DatabaseException.hpp
    cache/DatabaseException.cpp
    )

SET(CACHEINST_HEADERS
    cache/LogCache.hpp
    cache/ReposLog.hpp
    cache/DatabaseException.hpp
)

SET(INST_HEADERS
    annotate_line.hpp
    apr.hpp
    check.hpp
    client.hpp
    commititem.hpp
    context.hpp
    context_listener.hpp
    datetime.hpp
    dirent.hpp
    entry.hpp
    exception.hpp
    info_entry.hpp
    lock_entry.hpp
    log_entry.hpp
    path.hpp
    pool.hpp
    repository.hpp
    repositorylistener.hpp
    revision.hpp
    smart_pointer.hpp
    shared_pointer.hpp
    status.hpp
    svnfilestream.hpp
    svnstream.hpp
    svnqttypes.hpp
    targets.hpp
    url.hpp
    version_check.hpp
    wc.hpp
    stringarray.hpp
    diffoptions.hpp
    conflictdescription.hpp
    conflictresult.hpp
    )

FILE(GLOB svnhdr RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} "*.hpp")
SET(svnhdr ${svnhdr} ${CACHE_INST_HEADERS})

SET(QT_LIBRARIES "${QT_QTCORE_LIBRARY}")
MESSAGE(STATUS "Qt: ${QT_LIBRARIES}")
SET(TOUTF8 "toUtf8")
SET(FROMUTF8 "fromUtf8")
SET(QLIST "QList")
SET(TOASCII "toAscii")
SET(HOMEDIR "homePath")
IF(NOT svnqt-name)
SET(svnqt-name svnqt4)
ENDIF(NOT svnqt-name)

SET(QDATABASE "QSqlDatabase")
SET(QLONG "qlonglong")

# must set from caller before - here no checks of compiler will done!
IF (HAVE_GCC_VISIBILITY)
    SET(_SVNQT_EXPORT "__attribute__ ((visibility(\"default\")))")
    SET(_SVNQT_NOEXPORT "__attribute__ ((visibility(\"hidden\")))")
ENDIF (HAVE_GCC_VISIBILITY)

IF (WIN32)
    SET(_SVNQT_EXPORT "__declspec(dllexport)")
ENDIF (WIN32)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/svnqt_defines.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.hpp"
    IMMEDIATE
    @ONLY)

SET(INST_HEADERS ${INST_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.hpp)
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(${svnqt-name} SHARED ${SOURCES} ${svnhdr})
IF(WIN32)
    ADD_DEFINITIONS(-D_USE_32BIT_TIME_T)
ELSE(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX})
    SET_TARGET_PROPERTIES(${svnqt-name}
        PROPERTIES
        COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
ENDIF(WIN32)

SET(LIB_MAJOR 5)
SET(LIB_MINOR 0)
SET(LIB_RELEASE 1)

SET(_soversion ${LIB_MAJOR}.${LIB_MINOR}.${LIB_RELEASE})

SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES
    SOVERSION ${LIB_MAJOR}
    VERSION ${_soversion})

SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
IF(WIN32)
    SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
ELSE(WIN32)
    SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES LINK_FLAGS "-Wl,--as-needed ${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
ENDIF(WIN32)
TARGET_LINK_LIBRARIES(${svnqt-name} ${QT_LIBRARIES} ${SUBVERSION_ALL_LIBS} ${QT_QTSQL_LIBRARY})

IF (WIN32)
    TARGET_LINK_LIBRARIES( ${svnqt-name} wsock32.lib  )
ENDIF (WIN32)

# Just a small linking test
IF (BUILD_TESTS)
    ADD_EXECUTABLE(testlink testmain.cpp)
    TARGET_LINK_LIBRARIES(testlink ${svnqt-name})
ENDIF (BUILD_TESTS)

# install rules
# in win32 we don't install it
IF(NOT WIN32)
    INSTALL(TARGETS ${svnqt-name} DESTINATION ${LIB_INSTALL_DIR})
    INSTALL(FILES ${INST_HEADERS} DESTINATION include/svnqt)
    INSTALL(FILES ${CACHEINST_HEADERS} DESTINATION include/svnqt/cache)
ENDIF(NOT WIN32)

IF (BUILD_TESTS)
  ADD_SUBDIRECTORY(tests)
  ADD_SUBDIRECTORY(cache/test)
ENDIF (BUILD_TESTS)
