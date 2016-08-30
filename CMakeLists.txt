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
project(kdesvn)
set(PACKAGENAME "${CMAKE_PROJECT_NAME}")
cmake_minimum_required(VERSION 2.8.12)

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Honor visibility properties for all target types (since cmake 3.3)
endif()

# Where to look for Findxxx.cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/src/svnqt/cmake)
# Automatically include the current source and binary dirs
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(KDE4 4.9.0 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

# Set the version
set(KDESVN_VERSION_MAJOR 1)
set(KDESVN_VERSION_MINOR 7)
set(KDESVN_VERSION_PATCH 1)
set(KDESVN_VERSION_STRING ${KDESVN_VERSION_MAJOR}.${KDESVN_VERSION_MINOR}.${KDESVN_VERSION_PATCH})

# Append the git revision if available
# If Git is installed and a '.git' directory is found,
# we append the Git revision to KDESVN_VERSION_STRING
if(EXISTS "${kdesvn_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    message(STATUS "Found git: ${GIT_EXECUTABLE}")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${kdesvn_SOURCE_DIR}
                    OUTPUT_VARIABLE kdesvn_git_revision
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(KDESVN_VERSION_STRING "${KDESVN_VERSION_STRING} (rev: ${kdesvn_git_revision})")
  endif()
endif()

option(TESTINGBUILD "Enable/Disable special testing flags" OFF)
if(TESTINGBUILD)
    add_definitions(-DTESTING_RC=\"\\\"${CMAKE_SOURCE_DIR}/src/kdesvnui.rc\\\"\" -DTESTING_PARTRC=\"\\\"${CMAKE_SOURCE_DIR}/src/kdesvn_part.rc\\\"\")
    add_definitions(-DFORCE_ASKPASS=\"\\\"${CMAKE_BINARY_DIR}/bin/kdesvnaskpass\\\"\")
    add_definitions(-DEXTRA_KDE_LIBPATH=\"\\\"${PROJECT_BINARY_DIR}/lib\\\"\")
    add_definitions(-DDEBUG_TIMER=1)
endif()

add_definitions(-DQT_USE_FAST_CONCATENATION)
add_definitions(-DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_CAST_TO_ASCII)
# not (yet) possible
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQURL_NO_CAST_FROM_QSTRING)
add_definitions(-DQT_DEPRECATED_WARNINGS)
# not possible with kde 4.14 ...
#add_definitions(-DQT_NO_DEPRECATED)
add_definitions(-DQT_STRICT_ITERATORS)

include(MacroEnsureVersion)
include(FindSubversion)
include(kdesvn-macros)

# Enable exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

# from http://www.kdedevelopers.org/node/3171 and http://kdesvn.alwins-world.de/ticket/595#comment:5
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9510)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${QT_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
)

add_definitions(${APR_CPP_FLAGS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(doc)

configure_file(kdesvn-config.h.in kdesvn-config.h @ONLY)
configure_file(Doxyfile.in Doxyfile @ONLY)
configure_file(Doxyfile.svnqt.in Doxyfile.svnqt @ONLY)

set(DIST_EXCLUDE "Helperstuff/pic_design")

set(MESSAGES_DIR ${CMAKE_SOURCE_DIR}/src)

add_custom_target(apidox
    "doxygen" "${CMAKE_BINARY_DIR}/Doxyfile"
    DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile")
