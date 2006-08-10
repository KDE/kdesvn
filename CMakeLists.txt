PROJECT(kdesvn)
SET(CMAKE_COLOR_MAKEFILE ON CACHE STRING "Enable/Disable color output during build.")
include(${CMAKE_ROOT}/Modules/TestCXXAcceptsFlag.cmake)

SET(CMAKE_CXX_FLAGS_DEBUG
"${CMAKE_CXX_FLAGS_DEBUG} -DTESTING_RC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvnui.rc\\\"\" -DTESTING_PARTRC=\"\\\"\${CMAKE_SOURCE_DIR}/src/kdesvn_part.rc\\\"\"")

CHECK_CXX_ACCEPTS_FLAG("-fexceptions" CXX_EXCEPTION)
CHECK_CXX_ACCEPTS_FLAG("-fstack-protector" CXX_PROTECTOR)

INCLUDE(${kdesvn_SOURCE_DIR}/cmakemodules/FindKdeLibs.cmake)
INCLUDE(${kdesvn_SOURCE_DIR}/cmakemodules/MacroEnsureVersion.cmake)
INCLUDE(${kdesvn_SOURCE_DIR}/src/svnqt/cmakemodules/FindSubversion.cmake)

INCLUDE_DIRECTORIES(${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/svnfrontend
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/svnfrontend
    ${CMAKE_BINARY_DIR})

if (CXX_EXCEPTION)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif (CXX_EXCEPTION)
if (CXX_PROTECTOR)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif (CXX_PROTECTOR)

ADD_DEFINITIONS(${KDE3_DEFINITIONS} ${APR_CPP_FLAGS} ${QT_DEFINITIONS})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#die Version
SET(VERSION 0.9.2)
SET(KDESVN_VERSION \"${VERSION}\")

CONFIGURE_FILE(kdesvn-config.h.in config.h @ONLY)
CONFIGURE_FILE(kdesvn.spec.in kdesvn.spec @ONLY)
CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
CONFIGURE_FILE(Doxyfile.svnqt.in Doxyfile.svnqt @ONLY)
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(doc)
