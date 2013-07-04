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

include(SvnqtMacros)

set(SOURCES
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

set(CACHEINST_HEADERS
    cache/LogCache.h
    cache/ReposLog.h
    cache/DatabaseException.h
    cache/ReposConfig.h
    cache/DatabaseException.h
    cache/conversion_check.h
)

set(INST_HEADERS
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

file(GLOB svnhdr RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
set(svnhdr ${svnhdr} ${CACHE_INST_HEADERS})

set(QT_LIBRARIES "${QT_QTCORE_LIBRARY}")
message(STATUS "Qt: ${QT_LIBRARIES}")
set(TOUTF8 "toUtf8")
set(FROMUTF8 "fromUtf8")
set(QLIST "QList")
set(TOASCII "toAscii")
set(HOMEDIR "homePath")
if(NOT svnqt-name)
  set(svnqt-name svnqt)
endif()

set(QDATABASE "QSqlDatabase")
set(QLONG "qlonglong")

# must set from caller before - here no checks of compiler will done!
if(HAVE_GCC_VISIBILITY)
    set(_SVNQT_EXPORT "__attribute__ ((visibility(\"default\")))")
    set(_SVNQT_NOEXPORT "__attribute__ ((visibility(\"hidden\")))")
endif()

if(WIN32)
    set(_SVNQT_EXPORT "__declspec(dllexport)")
endif()

if(QT_LUPDATE_EXECUTABLE)
    message(STATUS "Setting up language target")

    file(GLOB _LANGFILES lang/*.ts)
    foreach(_p ${_LANGFILES})
        set(LANGFILES "-ts ${_p} ${LANGFILES}")
    endforeach()
    configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/update-language.sh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/update-language.sh"
    IMMEDIATE
    @ONLY)
    add_custom_target(svnqt_update_translation
        "${CMAKE_CURRENT_BINARY_DIR}/update-language.sh"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    add_subdirectory(lang)
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/svnqt_defines.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.h"
    IMMEDIATE
    @ONLY)

set(INST_HEADERS ${INST_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.h)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_library(${svnqt-name} SHARED ${SOURCES} ${svnhdr})
if(WIN32)
    add_definitions(-D_USE_32BIT_TIME_T)
else()
    set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX})
    set_target_properties(${svnqt-name}
        PROPERTIES
        COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
endif()

set(LIB_MAJOR 7)
set(LIB_MINOR 0)
set(LIB_RELEASE 1)

set(_soversion ${LIB_MAJOR}.${LIB_MINOR}.${LIB_RELEASE})

set_target_properties(${svnqt-name} PROPERTIES
    SOVERSION ${LIB_MAJOR}
    VERSION ${_soversion})

set(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
set(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")

if(WIN32 OR APPLE)
    set_target_properties(${svnqt-name} PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
else()
    set_target_properties(${svnqt-name} PROPERTIES LINK_FLAGS "-Wl,--as-needed ${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
endif()

target_link_libraries(${svnqt-name} ${QT_LIBRARIES} ${SUBVERSION_ALL_LIBS} ${QT_QTSQL_LIBRARY})

if(WIN32)
    target_link_libraries(${svnqt-name} wsock32.lib)
endif()

# Just a small linking test
if(BUILD_TESTS)
    add_executable(testlink testmain.cpp)
    target_link_libraries(testlink ${svnqt-name})
endif()

# install rules
# in win32 we don't install it
if(NOT WIN32)
    install(TARGETS ${svnqt-name} LIBRARY DESTINATION ${LIB_INSTALL_DIR}
            RUNTIME DESTINATION ${BIN_INSTALL_DIR}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
    install(FILES ${INST_HEADERS} DESTINATION include/svnqt)
    install(FILES ${CACHEINST_HEADERS} DESTINATION include/svnqt/cache)
endif()

if(BUILD_TESTS)
  add_subdirectory(tests)
  add_subdirectory(cache/test)
endif()
