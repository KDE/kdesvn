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

cmake_minimum_required(VERSION 3.16.0)

project(kdesvn)
set(PACKAGENAME "${CMAKE_PROJECT_NAME}")

set(QT_MIN_VERSION "6.5")
set(KF_MIN_VERSION "6.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84.0")

# Where to look for Findxxx.cmake modules
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/src/svnqt/cmake)
add_definitions(-DTRANSLATION_DOMAIN="kdesvn")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Bookmarks Codecs Completion Config
    ConfigWidgets CoreAddons DBusAddons DocTools
    I18n IconThemes ItemViews JobWidgets KIO
    Notifications Parts Service TextWidgets
    Wallet WidgetsAddons XmlGui
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core DBus Gui Sql Widgets Xml Core5Compat
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
add_definitions(-DQT_NO_KEYWORDS)

# include(MacroEnsureVersion)
include(FindSubversion)
include(kdesvn-macros)

# Enable exceptions
kde_enable_exceptions()

include_directories(
    ${CMAKE_BINARY_DIR}
)
include_directories(SYSTEM
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
)

add_definitions(${APR_CPP_FLAGS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(doc)
kdoctools_install(po)

configure_file(kdesvn-config.h.in kdesvn-config.h @ONLY)
configure_file(Doxyfile.in Doxyfile @ONLY)
configure_file(Doxyfile.svnqt.in Doxyfile.svnqt @ONLY)

set(DIST_EXCLUDE "Helperstuff/pic_design")

set(MESSAGES_DIR ${CMAKE_SOURCE_DIR}/src)

add_custom_target(apidox
    "doxygen" "${CMAKE_BINARY_DIR}/Doxyfile"
    DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile")

ki18n_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
