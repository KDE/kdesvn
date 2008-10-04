PROJECT(kdesvn)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(BASE_VERSION 1.0.4)

OPTION(BUILD_TESTS "Building the tests yes or no (default off)")
OPTION(DAILY_BUILD "Generate version number for a daily build (default off)")

SET(TESTINGBUILD OFF CACHE BOOL "Enable/Disable special testing flags")
IF (TESTINGBUILD)
   ADD_DEFINITIONS("-DTESTING_RC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvnui.rc\\\"\" -DTESTING_PARTRC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvn_part.rc\\\"\"")
   ADD_DEFINITIONS("-DFORCE_ASKPASS=\"\\\"\${CMAKE_BINARY_DIR}/bin/kdesvnaskpass\\\"\"")
ENDIF (TESTINGBUILD)

IF(BUILD_TESTS)
  ENABLE_TESTING()
ENDIF(BUILD_TESTS)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakemodules ${CMAKE_SOURCE_DIR}/src/svnqt/cmakemodules)

INCLUDE(FindKdeLibs)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX}/kde3)

INCLUDE(MacroEnsureVersion)
INCLUDE(FindSubversion)
INCLUDE(SetupCompiler)
INCLUDE(FindSqlite)

IF (HAVE_GCC_VISIBILITY)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    SET(_KDESVN_EXPORT "__attribute__ ((visibility(\"default\")))")
    SET(_KDESVN_NOEXPORT "__attribute__ ((visibility(\"hidden\")))")
ENDIF (HAVE_GCC_VISIBILITY)

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
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
)

ADD_DEFINITIONS(${KDE3_DEFINITIONS} ${QT_DEFINITIONS} ${APR_CPP_FLAGS})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  # FindKde3 always make a "-O2" into it. That isn't nice.
  STRING(REGEX REPLACE "-O2 " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
STRING(REGEX REPLACE "-fno-exceptions " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib${LIB_SUFFIX}/kde3)
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

CONFIGURE_FILE(kdesvn-config.h.in kdesvn-config.h @ONLY)
CONFIGURE_FILE(kdesvn.spec.in kdesvn.spec @ONLY)
CONFIGURE_FILE(Helperstuff/BuildService/kdesvn.spec.in Helperstuff/BuildService/kdesvn.spec @ONLY)
CONFIGURE_FILE(Helperstuff/BuildService/kdesvn.dsc.in Helperstuff/BuildService/kdesvn.dsc @ONLY)
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


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(doc)

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
