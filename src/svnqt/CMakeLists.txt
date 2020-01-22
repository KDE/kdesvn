####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
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

option(OVERRIDE_SVN_API_VERSION "Override the current API version (e.g. to compile against 1.6 api when 1.8 is installed)" OFF)
if(OVERRIDE_SVN_API_VERSION)
  set(OVERRIDE_SVN_API_VERSION_STR 0x10600 CACHE STRING "svn api version to use, hex format, two digits per major/minor/patch")
endif()

set(SOURCES
    annotate_line.cpp
    apr.cpp
    cache/DatabaseException.cpp
    cache/LogCache.cpp
    cache/ReposConfig.cpp
    cache/ReposLog.cpp
    client_annotate.cpp
    client_annotate_parameter.cpp
    client_cat.cpp
    client_commit_parameter.cpp
    client.cpp
    client_diff.cpp
    client_impl.cpp
    client_lock.cpp
    client_ls.cpp
    client_merge.cpp
    client_modify.cpp
    client_parameter.cpp
    client_property.cpp
    client_status.cpp
    client_update_parameter.cpp
    commititem.cpp
    conflictdescription.cpp
    conflictresult.cpp
    context.cpp
    contextdata.cpp
    datetime.cpp
    diff_data.cpp
    diffoptions.cpp
    dirent.cpp
    entry.cpp
    exception.cpp
    info_entry.cpp
    lock_entry.cpp
    log_entry.cpp
    path.cpp
    pool.cpp
    repository.cpp
    repositorydata.cpp
    repositorylistener.cpp
    reposnotify.cpp
    revision.cpp
    status.cpp
    stringarray.cpp
    svnfilestream.cpp
    svnstream.cpp
    targets.cpp
    url.cpp
    version_check.cpp
    )


configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/svnqt_defines.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/svnqt_defines.h"
    IMMEDIATE
    @ONLY)

add_library(svnqt STATIC ${SOURCES})
if(WIN32)
    target_compile_definitions(svnqt PUBLIC _USE_32BIT_TIME_T)
endif()
if(OVERRIDE_SVN_API_VERSION)
    target_compile_definitions(svnqt PUBLIC OVERRIDE_SVN_API_VERSION=${OVERRIDE_SVN_API_VERSION_STR})
endif()

set(ALL_LINKFLAGS ${APR_EXTRA_LDFLAGS})
set(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LDFLAGS}")

if(WIN32 OR APPLE)
    set_property(TARGET svnqt APPEND PROPERTY LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
else()
    set_property(TARGET svnqt APPEND PROPERTY LINK_FLAGS "-Wl,--as-needed ${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
endif()

target_link_libraries(svnqt
    PUBLIC
        ${SUBVERSION_ALL_LIBS}
        KF5::ConfigCore
        Qt5::Core
        Qt5::Sql
)
target_include_directories(svnqt
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_BINARY_DIR}/../
)
set_property(TARGET svnqt APPEND PROPERTY COMPILE_FLAGS ${APR_CPP_FLAGS})
set_property(TARGET svnqt PROPERTY CXX_VISIBILITY_PRESET hidden)

if(Backtrace_FOUND)
     target_include_directories(svnqt PUBLIC SYSTEM ${Backtrace_INCLUDE_DIRS})
     target_link_libraries(svnqt PUBLIC ${Backtrace_LIBRARY})
endif()

if(WIN32)
    target_link_libraries(svnqt wsock32.lib)
endif()

# Just a small linking test
if(KDE4_BUILD_TESTS)
    add_executable(testlink testmain.cpp)
    target_link_libraries(testlink svnqt)
endif()

if(KDE4_BUILD_TESTS)
  add_subdirectory(tests)
  add_subdirectory(cache/test)
endif()
