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

SET(SUBVERSIONFOUND)
SET(SUBVERSION_ALL_LIBS)

INCLUDE (CheckIncludeFiles)

#search libraries for UNIX
IF (UNIX)

  MACRO(FIND_SUB_LIB targetvar libname)
    IF (SUBVERSION_INSTALL_PATH)
        FIND_LIBRARY(${targetvar} ${libname}
            PATHS
            ${SUBVERSION_INSTALL_PATH}/lib
            NO_DEFAULT_PATH
        )
    ENDIF(SUBVERSION_INSTALL_PATH)
    FIND_LIBRARY(${targetvar} ${libname}
        PATHS
        /usr/lib
        /usr/local/lib
        )
  ENDMACRO(FIND_SUB_LIB)

  IF (SUBVERSION_INSTALL_PATH)
    FIND_PATH(SUBVERSION_INCLUDE_DIR svn_client.h
        PATHS
        ${SUBVERSION_INSTALL_PATH}/include/subversion-1
        NO_DEFAULT_PATH
    )
  ENDIF (SUBVERSION_INSTALL_PATH)
  FIND_PATH(SUBVERSION_INCLUDE_DIR svn_client.h
        /usr/include/subversion-1
        /usr/local/include/subversion-1)

  FIND_SUB_LIB(SUBVERSION_CLIENTLIB svn_client-1)
  FIND_SUB_LIB(SUBVERSION_REPOSITORYLIB svn_repos-1)
  FIND_SUB_LIB(SUBVERSION_WCLIB svn_wc-1)
  FIND_SUB_LIB(SUBVERSION_FSLIB svn_fs-1)
  FIND_SUB_LIB(SUBVERSION_SUBRLIB svn_subr-1)
  FIND_SUB_LIB(SUBVERSION_RALIB svn_ra-1)
  FIND_SUB_LIB(SUBVERSION_DIFFLIB svn_diff-1)

  FIND_PROGRAM(APR_CONFIG NAMES apr-config apr-1-config
    PATHS
    /usr/local/apr/bin
  )

  FIND_PROGRAM(APU_CONFIG NAMES apu-config apu-1-config
    PATHS
    /usr/local/apr/bin
  )

  if(NOT APR_CONFIG)
    MESSAGE(SEND_ERROR "Error: no apr-config found")
  endif(NOT APR_CONFIG)

  if(NOT APU_CONFIG)
    MESSAGE(SEND_ERROR "Error: no apu-config found")
  endif(NOT APU_CONFIG)

  EXECUTE_PROCESS(COMMAND ${APR_CONFIG} "--includedir" OUTPUT_VARIABLE APR_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  MESSAGE(STATUS "Found apr include: ${APR_INCLUDE_DIR}")
  EXECUTE_PROCESS(COMMAND ${APU_CONFIG} "--includedir" OUTPUT_VARIABLE APU_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  MESSAGE(STATUS "Found apu include: ${APU_INCLUDE_DIR}")

  EXECUTE_PROCESS(COMMAND ${APR_CONFIG} "--cppflags" OUTPUT_VARIABLE APR_CPP_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  MESSAGE(STATUS "Found apr cppflags: ${APR_CPP_FLAGS}")
  #EXECUTE_PROCESS(COMMAND ${APU_CONFIG} "--cppflags" OUTPUT_VARIABLE APU_CPP_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  #MESSAGE(STATUS "Found apu cppflags: ${APU_CPP_FLAGS}")

  EXECUTE_PROCESS(COMMAND ${APR_CONFIG} "--link-ld" "--libs" OUTPUT_VARIABLE APR_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REGEX REPLACE "^ +" "" APR_LIBS "${APR_LIBS}")
  MESSAGE(STATUS "Found apr libs: ${APR_LIBS}")
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APR_LIBS})

  EXECUTE_PROCESS(COMMAND ${APR_CONFIG} "--ldflags" OUTPUT_VARIABLE APR_EXTRA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REGEX REPLACE "^ +" "" APR_EXTRA_LDFLAGS "${APR_EXTRA_LDFLAGS}")
  MESSAGE(STATUS "Found apr extra ldflags: ${APR_EXTRA_LDFLAGS}")

  EXECUTE_PROCESS(COMMAND ${APU_CONFIG} "--link-ld" "--libs" OUTPUT_VARIABLE APU_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REGEX REPLACE "^ +" "" APU_LIBS "${APU_LIBS}")
  MESSAGE(STATUS "Found apu libs: ${APU_LIBS}")
  SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_LIBS})

  EXECUTE_PROCESS(COMMAND ${APU_CONFIG} "--ldflags" OUTPUT_VARIABLE APU_EXTRA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(REGEX REPLACE "^ +" "" APU_EXTRA_LDFLAGS "${APU_EXTRA_LDFLAGS}")
  MESSAGE(STATUS "Found apu extra ldflags: ${APU_EXTRA_LDFLAGS}")

  CHECK_INCLUDE_FILES(execinfo.h HAS_BACKTRACE_H)

  CHECK_INCLUDE_FILES("svn_version.h" HAS_SVN_VERSION_H)


ENDIF (UNIX)

#search libaries for Windows
IF (WIN32)

  # search for pathes
  FIND_PATH (SUBVERSION_BIN_DIR svn.exe
    "${SUBVERSION_INSTALL_PATH}/bin"
    "$ENV{ProgramFiles}/Subversion/bin"
  )

  FIND_PATH (SUBVERSION_INCLUDE_DIR svn_client.h
    "${SUBVERSION_INSTALL_PATH}/include"
    "$ENV{ProgramFiles}/Subversion/include"
  )

  FIND_PATH(APR_INCLUDE_DIR apr.h
    "${SUBVERSION_INSTALL_PATH}/include/apr"
    "$ENV{ProgramFiles}/Subversion/include/apr"
  )

  FIND_PATH(APU_INCLUDE_DIR apu.h
    "${SUBVERSION_INSTALL_PATH}/include/apr-util"
    "$ENV{ProgramFiles}/Subversion/include/apr-util"
  )

  # search for libraries
  FIND_LIBRARY(APR_LIB libapr-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr"
    "$ENV{ProgramFiles}/Subversion/lib/apr"
  )

  FIND_LIBRARY(APRICONV_LIB libapriconv-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr-iconv"
    "$ENV{ProgramFiles}/Subversion/lib/apr-iconv"
  )

  FIND_LIBRARY(APU_LIB libaprutil-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr-util"
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  FIND_LIBRARY(APU_XMLLIB xml
    "${SUBVERSION_INSTALL_PATH}/lib/apr-util"
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  FIND_LIBRARY(NEON_LIB libneon
    "${SUBVERSION_INSTALL_PATH}/lib/neon"
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  FIND_LIBRARY(NEON_ZLIBSTATLIB zlibstat
    "${SUBVERSION_INSTALL_PATH}/lib/neon"
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  FIND_LIBRARY(INTL3LIB intl3_svn
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(DB44_LIB libdb44
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_CLIENTLIB libsvn_client-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_DELTALIB libsvn_delta-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_DIFFLIB libsvn_diff-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSBASELIB libsvn_fs_base-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSFSLIB libsvn_fs_fs-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSUTILLIB libsvn_fs_util-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_FSLIB libsvn_fs-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RALOCALLIB libsvn_ra_local-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RANEONLIB libsvn_ra_neon-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RASVNLIB libsvn_ra_svn-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_RALIB libsvn_ra-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_REPOSITORYLIB libsvn_repos-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_SUBRLIB libsvn_subr-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  FIND_LIBRARY(SUBVERSION_WCLIB libsvn_wc-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  SET(APR_EXTRA_LIBS )
  SET(APR_EXTRA_LDFLAGS )
  SET(APU_EXTRA_LIBS )
  SET(APU_EXTRA_LDFLAGS )


  # check found libraries
  if (NOT APR_LIB)
    MESSAGE(SEND_ERROR "No apr lib found!")
  ELSE (NOT APR_LIB)
    MESSAGE(STATUS "Found apr lib: ${APR_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APR_LIB} ${APR_EXTRA_LIBS})
  endif(NOT APR_LIB)

  if (NOT APRICONV_LIB)
    MESSAGE(SEND_ERROR "No apriconv lib found!")
  ELSE (NOT APRICONV_LIB)
    MESSAGE(STATUS "Found apriconv lib: ${APRICONV_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APRICONV_LIB})
  endif(NOT APRICONV_LIB)

  if (NOT APU_LIB)
    MESSAGE(SEND_ERROR "No aprutil lib found!")
  ELSE (NOT APU_LIB)
    MESSAGE(STATUS "Found aprutil lib: ${APU_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_LIB} ${APU_EXTRA_LIBS})
  endif(NOT APU_LIB)

  if (NOT APU_XMLLIB)
    MESSAGE(SEND_ERROR "No xml lib found!")
  ELSE (NOT APU_XMLLIB)
    MESSAGE(STATUS "Found xml lib: ${APU_XMLLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_XMLLIB})
  endif(NOT APU_XMLLIB)

  if (NOT NEON_LIB)
    MESSAGE(SEND_ERROR "No neon lib found!")
  ELSE (NOT NEON_LIB)
    MESSAGE(STATUS "Found neon lib: ${NEON_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_LIB})
  endif(NOT NEON_LIB)

  if (NOT NEON_ZLIBSTATLIB)
    MESSAGE(SEND_ERROR "No zlibstat lib found!")
  ELSE (NOT APRICONV_LIB)
    MESSAGE(STATUS "Found zlibstat lib: ${NEON_ZLIBSTATLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_ZLIBSTATLIB})
  endif(NOT NEON_ZLIBSTATLIB)

  if (NOT INTL3LIB)
    MESSAGE(SEND_ERROR "No intl3 lib found!")
  ELSE (NOT INTL3LIB)
    MESSAGE(STATUS "Found intl3 lib: ${INTL3LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${INTL3LIB})
  endif(NOT INTL3LIB)

  if (NOT DB44_LIB)
    MESSAGE(SEND_ERROR "No db44 lib found!")
  ELSE (NOT DB44_LIB)
    MESSAGE(STATUS "Found db44 lib: ${DB44_LIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${DB44_LIB})
  endif(NOT DB44_LIB)

  if (NOT SUBVERSION_DELTALIB)
    MESSAGE(SEND_ERROR "No subversion delta lib found!")
  ELSE (NOT SUBVERSION_DELTALIB)
    MESSAGE(STATUS "Found subversion delta lib: ${SUBVERSION_DELTALIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DELTALIB})
  endif(NOT SUBVERSION_DELTALIB)

  if (NOT SUBVERSION_FSBASELIB)
    MESSAGE(SEND_ERROR "No subversion fs base lib found!")
  ELSE (NOT SUBVERSION_FSBASELIB)
    MESSAGE(STATUS "Found subversion fs base lib: ${SUBVERSION_FSBASELIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSBASELIB})
  endif(NOT SUBVERSION_FSBASELIB)

  if (NOT SUBVERSION_FSFSLIB)
    MESSAGE(SEND_ERROR "No subversion fs fs lib found!")
  ELSE (NOT SUBVERSION_FSFSLIB)
    MESSAGE(STATUS "Found subversion fs fs lib: ${SUBVERSION_FSFSLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSFSLIB})
  endif(NOT SUBVERSION_FSFSLIB)

  if (NOT SUBVERSION_FSUTILLIB)
    MESSAGE(SEND_ERROR "No subversion fs util lib found!")
  ELSE (NOT SUBVERSION_FSUTILLIB)
    MESSAGE(STATUS "Found subversion fs util lib: ${SUBVERSION_FSUTILLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSUTILLIB})
  endif(NOT SUBVERSION_FSUTILLIB)

  if (NOT SUBVERSION_RALOCALLIB)
    MESSAGE(SEND_ERROR "No subversion ra local lib found!")
  ELSE (NOT SUBVERSION_RALOCALLIB)
    MESSAGE(STATUS "Found subversion ra local lib: ${SUBVERSION_RALOCALLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALOCALLIB})
  endif(NOT SUBVERSION_RALOCALLIB)

  if (NOT SUBVERSION_RANEONLIB)
    MESSAGE(SEND_ERROR "No subversion ra neon lib found!")
  ELSE (NOT SUBVERSION_RANEONLIB)
    MESSAGE(STATUS "Found subversion ra neon lib: ${SUBVERSION_RANEONLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RANEONLIB})
  endif(NOT SUBVERSION_RANEONLIB)

  if (NOT SUBVERSION_RASVNLIB)
    MESSAGE(SEND_ERROR "No subversion ra svn lib found!")
  ELSE (NOT SUBVERSION_RASVNLIB)
    MESSAGE(STATUS "Found subversion ra svn lib: ${SUBVERSION_RASVNLIB}")
    SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RASVNLIB})
  endif(NOT SUBVERSION_RASVNLIB)

ENDIF (WIN32)


IF(NOT SUBVERSION_INCLUDE_DIR)
  MESSAGE(SEND_ERROR "No subversion includes found!")
ELSE(NOT SUBVERSION_INCLUDE_DIR)
  MESSAGE(STATUS "Found subversion include: ${SUBVERSION_INCLUDE_DIR}")
ENDIF(NOT SUBVERSION_INCLUDE_DIR)

if (NOT SUBVERSION_CLIENTLIB)
 MESSAGE(SEND_ERROR "No subversion client libs found!")
ELSE (NOT SUBVERSION_CLIENTLIB)
 MESSAGE(STATUS "Found subversion client lib: ${SUBVERSION_CLIENTLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_CLIENTLIB})
endif(NOT SUBVERSION_CLIENTLIB)

if (NOT SUBVERSION_DIFFLIB)
 MESSAGE(SEND_ERROR "No subversion diff lib found!")
ELSE (NOT SUBVERSION_DIFFLIB)
 MESSAGE(STATUS "Found subversion diff lib: ${SUBVERSION_DIFFLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DIFFLIB})
endif(NOT SUBVERSION_DIFFLIB)

if (NOT SUBVERSION_FSLIB)
 MESSAGE(SEND_ERROR "No subversion fs lib found!")
ELSE (NOT SUBVERSION_FSLIB)
 MESSAGE(STATUS "Found subversion fs lib: ${SUBVERSION_FSLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSLIB})
endif(NOT SUBVERSION_FSLIB)

if (NOT SUBVERSION_RALIB)
 MESSAGE(SEND_ERROR "No subversion ra lib found!")
ELSE (NOT SUBVERSION_RALIB)
 MESSAGE(STATUS "Found subversion ra lib: ${SUBVERSION_RALIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALIB})
endif(NOT SUBVERSION_RALIB)

if (NOT SUBVERSION_REPOSITORYLIB)
 MESSAGE(SEND_ERROR "No subversion repository lib found!")
ELSE (NOT SUBVERSION_REPOSITORYLIB)
 MESSAGE(STATUS "Found subversion repository lib: ${SUBVERSION_REPOSITORYLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_REPOSITORYLIB})
endif(NOT SUBVERSION_REPOSITORYLIB)

if (NOT SUBVERSION_SUBRLIB)
 MESSAGE(SEND_ERROR "No subversion subr lib found!")
ELSE (NOT SUBVERSION_SUBRLIB)
 MESSAGE(STATUS "Found subversion subr lib: ${SUBVERSION_SUBRLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_SUBRLIB})
endif(NOT SUBVERSION_SUBRLIB)

if (NOT SUBVERSION_WCLIB)
 MESSAGE(SEND_ERROR "No subversion wc lib found!")
ELSE (NOT SUBVERSION_WCLIB)
 MESSAGE(STATUS "Found subversion wc lib: ${SUBVERSION_WCLIB}")
 SET(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_WCLIB})
endif(NOT SUBVERSION_WCLIB)


SET(SUBVERSIONFOUND true)
