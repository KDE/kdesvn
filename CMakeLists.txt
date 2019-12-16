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
project(kdesvn)
set(PACKAGENAME "${CMAKE_PROJECT_NAME}")
cmake_minimum_required(VERSION 3.1.0)
if(POLICY CMP0063)
  CMAKE_POLICY(SET CMP0063 NEW) # Honor visibility properties for all target types.
endif()

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW) # Honor visibility properties for all target types (since cmake 3.3)
endif()

# Where to look for Findxxx.cmake modules
find_package(ECM 5.10.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/src/svnqt/cmake)
# Automatically include the current source and binary dirs
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)
add_definitions(-DTRANSLATION_DOMAIN="kdesvn")

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)

find_package(KF5 REQUIRED COMPONENTS
    Bookmarks Codecs Completion Config
    ConfigWidgets CoreAddons DBusAddons DocTools
    I18n IconThemes ItemViews JobWidgets KIO
    Notifications Parts Service TextWidgets
    Wallet WidgetsAddons XmlGui
)
set(QT_MIN_VERSION 5.9.0)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core DBus Gui Sql Widgets Xml
)

# Set the version
set(KDESVN_VERSION_MAJOR 2)
set(KDESVN_VERSION_MINOR 1)
set(KDESVN_VERSION_PATCH 0)
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

add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_TO_ASCII)
# not (yet) possible
#add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
# kde headers still use Q_FOREACH... :(
# add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_STRICT_ITERATORS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050900)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

# include(MacroEnsureVersion)
include(FindSubversion)
include(kdesvn-macros)

# Enable exceptions
kde_enable_exceptions()

include_directories(
    ${CMAKE_BINARY_DIR}
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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
