PROJECT(kdesvn)
include(${CMAKE_ROOT}/Modules/TestCXXAcceptsFlag.cmake)
CHECK_CXX_ACCEPTS_FLAG("-fexceptions" CXX_EXCEPTION)
CHECK_CXX_ACCEPTS_FLAG("-fstack-protector" CXX_PROTECTOR)

include(${kdesvn_SOURCE_DIR}/cmakemodules/FindKdeLibs.cmake)
include(${kdesvn_SOURCE_DIR}/src/svnqt/cmakemodules/FindSubversion.cmake)

INCLUDE_DIRECTORIES(${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${APU_INCLUDE_DIR}
    ${SUBVERSION_INCLUDE_DIR}
    ${kdesvn_SOURCE_DIR}/src
    ${kdesvn_SOURCE_DIR}/src/svnfrontend
    ${kdesvn_SOURCE_DIR})

if (CXX_EXCEPTION)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif (CXX_EXCEPTION)
if (CXX_PROTECTOR)
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif (CXX_PROTECTOR)

ADD_DEFINITIONS(${KDE3_DEFINITIONS} ${APR_CPP_FLAGS} ${QT_DEFINITIONS})
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Builds/${CMAKE_BUILD_TYPE}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Builds/${CMAKE_BUILD_TYPE}/lib)

#die Version
SET(VERSION 0.9.2)
SET(KDESVN_VERSION \"${VERSION}\")

CONFIGURE_FILE(kdesvn-config.h.in config.h @ONLY)
CONFIGURE_FILE(kdesvn.spec.in kdesvn.spec @ONLY)

SUBDIRS(src)

