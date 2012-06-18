####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
 #   http://kdesvn.alwins-world.de/                                        #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 2 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, write to the                         #
 #   Free Software Foundation, Inc.,                                       #
 #   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
 ####

include (SvnqtMacros)

SET(SOURCES
    client_update_parameter.cpp
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
    repoparameter.cpp
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
    cache/DatabaseException.cpp
    cache/ReposConfig.cpp
    client_parameter.cpp
    client_commit_parameter.cpp
    client_annotate_parameter.cpp
    reposnotify.cpp
    annotate_line.cpp
    )

SET(CACHEINST_HEADERS
    cache/LogCache.h
    cache/ReposLog.h
    cache/DatabaseException.h
    cache/ReposConfig.h
    cache/DatabaseException.h
    cache/conversion_check.h
)

SET(INST_HEADERS
    annotate_line.h
    apr.h
    check.h
    client.h
    commititem.h
    context.h
    context_listener.h
    datetime.h
    dirent.h
    entry.h
    exception.h
    info_entry.h
    lock_entry.h
    log_entry.h
    path.h
    pool.h
    repository.h
    repositorylistener.h
    repoparameter.cpp
    revision.h
    smart_pointer.h
    shared_pointer.h
    status.h
    svnfilestream.h
    svnstream.h
    svnqttypes.h
    targets.h
    url.h
    version_check.h
    wc.h
    stringarray.h
    diffoptions.h
    conflictdescription.h
    conflictresult.h
    client_parameter.h
    repoparameter.h
    client_commit_parameter.h
    client_annotate_parameter.h
    client_update_parameter.h
    reposnotify.h
    )

FILE(GLOB svnhdr RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
SET(svnhdr ${svnhdr} ${CACHE_INST_HEADERS})

SET(QT_LIBRARIES "${QT_QTCORE_LIBRARY}")
MESSAGE(STATUS "Qt: ${QT_LIBRARIES}")
SET(TOUTF8 "toUtf8")
SET(FROMUTF8 "fromUtf8")
SET(QLIST "QList")
SET(TOASCII "toAscii")
SET(HOMEDIR "homePath")
IF(NOT svnqt-name)
SET(svnqt-name svnqt)
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

IF (QT_LUPDATE_EXECUTABLE)
    MESSAGE(STATUS "Setting up language target")

    FILE(GLOB _LANGFILES lang/*.ts)
    FOREACH(_p ${_LANGFILES})
        SET(LANGFILES "-ts ${_p} ${LANGFILES}")
    ENDFOREACH(_p)
    CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/update-language.sh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/update-language.sh"
    IMMEDIATE
    @ONLY)
    ADD_CUSTOM_TARGET(svnqt_update_translation
        "${CMAKE_CURRENT_BINARY_DIR}/update-language.sh"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    ADD_SUBDIRECTORY(lang)
ENDIF (QT_LUPDATE_EXECUTABLE)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/svnqt_defines.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.h"
    IMMEDIATE
    @ONLY)

SET(INST_HEADERS ${INST_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.h)
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

SET(LIB_MAJOR 7)
SET(LIB_MINOR 0)
SET(LIB_RELEASE 1)

SET(_soversion ${LIB_MAJOR}.${LIB_MINOR}.${LIB_RELEASE})

SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES
    SOVERSION ${LIB_MAJOR}
    VERSION ${_soversion})

SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")

IF(WIN32 OR APPLE)
    SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
ELSE(WIN32 OR APPLE)
    SET_TARGET_PROPERTIES(${svnqt-name} PROPERTIES LINK_FLAGS "-Wl,--as-needed ${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
ENDIF(WIN32 OR APPLE)

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
    INSTALL(TARGETS ${svnqt-name} LIBRARY DESTINATION ${LIB_INSTALL_DIR}
            RUNTIME DESTINATION ${BIN_INSTALL_DIR}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
    INSTALL(FILES ${INST_HEADERS} DESTINATION include/svnqt)
    INSTALL(FILES ${CACHEINST_HEADERS} DESTINATION include/svnqt/cache)
ENDIF(NOT WIN32)

IF (BUILD_TESTS)
  ADD_SUBDIRECTORY(tests)
  ADD_SUBDIRECTORY(cache/test)
ENDIF (BUILD_TESTS)
