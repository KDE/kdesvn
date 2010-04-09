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

PROJECT(kdesvn)
SET(PACKAGENAME "${CMAKE_PROJECT_NAME}")
IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
SET(BASE_VERSION 1.5.3)

OPTION(BUILD_TESTS "Building the tests yes or no (default off)")

SET(TESTINGBUILD OFF CACHE BOOL "Enable/Disable special testing flags")
IF (TESTINGBUILD)
    IF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
        ADD_DEFINITIONS("-DTESTING_RC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvnui.rc\\\"\" -DTESTING_PARTRC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvn_part.rc\\\"\"")
        ADD_DEFINITIONS("-DFORCE_ASKPASS=\"\\\"\${CMAKE_BINARY_DIR}/bin/kdesvnaskpass\\\"\"")
    ELSE (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
        ADD_DEFINITIONS(-DTESTING_RC=\"${CMAKE_SOURCE_DIR}/src/kdesvnui.rc\" -DTESTING_PARTRC=\"${CMAKE_SOURCE_DIR}/src/kdesvn_part.rc\")
        ADD_DEFINITIONS(-DFORCE_ASKPASS=\"${CMAKE_BINARY_DIR}/bin/kdesvnaskpass\")
    ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION EQUAL 4)
    ADD_DEFINITIONS(-DDEBUG_TIMER=1)
ENDIF (TESTINGBUILD)

IF(BUILD_TESTS)
  ENABLE_TESTING()
ENDIF(BUILD_TESTS)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakemodules ${CMAKE_SOURCE_DIR}/src/svnqt/cmakemodules)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX}/kde4)

MACRO(KDE_AUTOMOC)
 QT4_AUTOMOC(${ARGV})
ENDMACRO(KDE_AUTOMOC)

MACRO(KDE_ADD_UI_FILES)
 KDE4_ADD_UI_FILES(${ARGV})
ENDMACRO(KDE_ADD_UI_FILES)

MACRO(KDE_ADD_KCFG_FILES)
 KDE4_ADD_KCFG_FILES(${ARGV})
ENDMACRO(KDE_ADD_KCFG_FILES)

MACRO(KDE_ADD_KPART)
 kde4_add_plugin(${ARGV})
ENDMACRO(KDE_ADD_KPART)

MACRO(KDE_ADD_MOC_FILES)
 KDE4_MOC_HEADERS(${ARGV})
ENDMACRO(KDE_ADD_MOC_FILES)

MACRO(KDESVN_INSTALL_ICONS)
 kde4_install_icons( ${ICON_INSTALL_DIR}  )
ENDMACRO(KDESVN_INSTALL_ICONS)

MACRO(KDESVN_GENERATE_LIBTOOL_FILE _subdir _target)
 KDE4_INSTALL_LIBTOOL_FILE(${_subdir} ${_target})
ENDMACRO(KDESVN_GENERATE_LIBTOOL_FILE)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES}
${QT_QTDBUS_INCLUDE_DIR})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX}/kde4)

INCLUDE(MacroEnsureVersion)
INCLUDE(FindSubversion)
INCLUDE(SetupCompiler)
INCLUDE(FindGettext)

IF (__KDE_HAVE_GCC_VISIBILITY)
    SET(HAVE_GCC_VISIBILITY On)
    SET(_KDESVN_EXPORT "__attribute__ ((visibility(\"default\")))")
    SET(_KDESVN_NOEXPORT "__attribute__ ((visibility(\"hidden\")))")
ENDIF (__KDE_HAVE_GCC_VISIBILITY)

# from http://www.kdedevelopers.org/node/3171 and http://kdesvn.alwins-world.de/ticket/595#comment:5
ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=9510)

INCLUDE_DIRECTORIES (
    BEFORE
    ${CMAKE_SOURCE_DIR}/src/svnqt
)

INCLUDE_DIRECTORIES (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/svnfrontend
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/svnfrontend
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
)

ADD_DEFINITIONS(${APR_CPP_FLAGS})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  # FindKde3 always make a "-O2" into it. That isn't nice.
  STRING(REGEX REPLACE "-O2 " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

#we requires exeptions!!!!
STRING(REGEX REPLACE "-fno-exceptions " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(_BASE_LDADD "-L${CMAKE_BINARY_DIR}/lib${LIB_SUFFIX}")

# current version
IF (DAILY_BUILD)
    EXECUTE_PROCESS(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE BUILDDATE)
    STRING(REPLACE "\n" "" BUILDDATE ${BUILDDATE})
    SET(VERSION "${BASE_VERSION}.${BUILDDATE}")
    SET(KDESVN_VERSION "\"${BASE_VERSION}-${BUILDDATE}\"")
ELSE(DAILY_BUILD)
    SET(VERSION "$ENV{KDESVN_VERSION}")
    IF(NOT VERSION)
        SET(VERSION ${BASE_VERSION})
    ENDIF(NOT VERSION)
    SET(KDESVN_VERSION \"${VERSION}\")
ENDIF(DAILY_BUILD)

SET(svnqt-name svnqt)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(doc)

CONFIGURE_FILE(kdesvn-config.h.in kdesvn-config.h @ONLY)
CONFIGURE_FILE(kdesvn.spec.in ${PROJECT_NAME}.spec @ONLY)
CONFIGURE_FILE(Helperstuff/BuildService/kdesvn.spec.in Helperstuff/BuildService/${PROJECT_NAME}.spec @ONLY)
CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
CONFIGURE_FILE(Doxyfile.svnqt.in Doxyfile.svnqt @ONLY)

SET(DIST_EXCLUDE "Helperstuff/pic_design")
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/makedist.sh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/makedist.sh"
    IMMEDIATE
    @ONLY)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

ADD_CUSTOM_TARGET(dist
    "${CMAKE_BINARY_DIR}/makedist.sh"
    DEPENDS doc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

FILE(GLOB _POFILES po/*.po)
FOREACH(_p ${_POFILES})
  SET(POFILES "${_p} ${POFILES}")
ENDFOREACH(_p)

SET(MESSAGES_DIR ${CMAKE_SOURCE_DIR}/src)
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/package_messages.sh.in"
    "${CMAKE_CURRENT_BINARY_DIR}/package_messages.sh"
    IMMEDIATE
    @ONLY)
ADD_CUSTOM_TARGET(package-messages
    "${CMAKE_CURRENT_BINARY_DIR}/package_messages.sh")

ADD_CUSTOM_TARGET(apidox
    "doxygen" "${CMAKE_BINARY_DIR}/Doxyfile"
    DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile")
