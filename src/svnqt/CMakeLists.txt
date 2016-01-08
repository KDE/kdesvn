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
    repoparameter.cpp
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

add_library(svnqt STATIC ${SOURCES})
if(WIN32)
    target_compile_definitions(svnqt PUBLIC _USE_32BIT_TIME_T)
endif()

set(ALL_LINKFLAGS ${APR_EXTRA_LDFLAGS})
set(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LDFLAGS}")

if(WIN32 OR APPLE)
    set_property(TARGET svnqt APPEND PROPERTY LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
else()
    set_property(TARGET svnqt APPEND PROPERTY LINK_FLAGS "-Wl,--as-needed ${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
endif()

target_link_libraries(svnqt
    ${SUBVERSION_ALL_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
)
set_property(TARGET svnqt APPEND PROPERTY COMPILE_FLAGS ${APR_CPP_FLAGS})
set_property(TARGET svnqt PROPERTY CXX_VISIBILITY_PRESET hidden)

if(WIN32)
    target_link_libraries(svnqt wsock32.lib)
endif()

# Just a small linking test
if(KDE4_BUILD_TESTS)
    add_executable(testlink testmain.cpp)
    target_link_libraries(testlink svnqt)
endif()

# install rules
install(TARGETS svnqt LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

if(KDE4_BUILD_TESTS)
  add_subdirectory(tests)
  add_subdirectory(cache/test)
endif()
