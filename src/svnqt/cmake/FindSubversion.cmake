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

set(SUBVERSIONFOUND)
set(SUBVERSION_ALL_LIBS)

INCLUDE (CheckIncludeFiles)

#search libraries for UNIX
if(UNIX)

  MACRO(FIND_SUB_LIB targetvar libname)
    if(SUBVERSION_INSTALL_PATH)
        find_library(${targetvar} ${libname}
            PATHS
            ${SUBVERSION_INSTALL_PATH}/lib
            NO_DEFAULT_PATH
        )
    endif(SUBVERSION_INSTALL_PATH)
    find_library(${targetvar} ${libname}
        PATHS
        /usr/lib
        /usr/local/lib
        )
  endmacro()

  if(SUBVERSION_INSTALL_PATH)
    find_path(SUBVERSION_INCLUDE_DIR svn_client.h
        PATHS
        ${SUBVERSION_INSTALL_PATH}/include/subversion-1
        NO_DEFAULT_PATH
    )
  endif(SUBVERSION_INSTALL_PATH)
  find_path(SUBVERSION_INCLUDE_DIR svn_client.h
        /usr/include/subversion-1
        /usr/local/include/subversion-1)

  find_sub_lib(SUBVERSION_CLIENTLIB svn_client-1)
  find_sub_lib(SUBVERSION_REPOSITORYLIB svn_repos-1)
  find_sub_lib(SUBVERSION_WCLIB svn_wc-1)
  find_sub_lib(SUBVERSION_FSLIB svn_fs-1)
  find_sub_lib(SUBVERSION_SUBRLIB svn_subr-1)
  find_sub_lib(SUBVERSION_RALIB svn_ra-1)
  find_sub_lib(SUBVERSION_DIFFLIB svn_diff-1)

  find_program(APR_CONFIG NAMES apr-config apr-1-config
    PATHS
    /usr/local/apr/bin
  )

  find_program(APU_CONFIG NAMES apu-config apu-1-config
    PATHS
    /usr/local/apr/bin
  )

  if(NOT APR_CONFIG)
    message(SEND_ERROR "Error: no apr-config found")
  endif()

  if(NOT APU_CONFIG)
    message(SEND_ERROR "Error: no apu-config found")
  endif()

  execute_process(COMMAND ${APR_CONFIG} "--includedir" OUTPUT_VARIABLE APR_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Found apr include: ${APR_INCLUDE_DIR}")
  execute_process(COMMAND ${APU_CONFIG} "--includedir" OUTPUT_VARIABLE APU_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Found apu include: ${APU_INCLUDE_DIR}")

  execute_process(COMMAND ${APR_CONFIG} "--cppflags" OUTPUT_VARIABLE APR_CPP_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Found apr cppflags: ${APR_CPP_FLAGS}")
  #execute_process(COMMAND ${APU_CONFIG} "--cppflags" OUTPUT_VARIABLE APU_CPP_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  #message(STATUS "Found apu cppflags: ${APU_CPP_FLAGS}")

  execute_process(COMMAND ${APR_CONFIG} "--link-ld" "--libs" OUTPUT_VARIABLE APR_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^ +" "" APR_LIBS "${APR_LIBS}")
  message(STATUS "Found apr libs: ${APR_LIBS}")
  set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APR_LIBS})

  execute_process(COMMAND ${APR_CONFIG} "--ldflags" OUTPUT_VARIABLE APR_EXTRA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^ +" "" APR_EXTRA_LDFLAGS "${APR_EXTRA_LDFLAGS}")
  message(STATUS "Found apr extra ldflags: ${APR_EXTRA_LDFLAGS}")

  execute_process(COMMAND ${APU_CONFIG} "--link-ld" "--libs" OUTPUT_VARIABLE APU_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^ +" "" APU_LIBS "${APU_LIBS}")
  message(STATUS "Found apu libs: ${APU_LIBS}")
  set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_LIBS})

  execute_process(COMMAND ${APU_CONFIG} "--ldflags" OUTPUT_VARIABLE APU_EXTRA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^ +" "" APU_EXTRA_LDFLAGS "${APU_EXTRA_LDFLAGS}")
  message(STATUS "Found apu extra ldflags: ${APU_EXTRA_LDFLAGS}")

  find_package(Backtrace)
  if(Backtrace_FOUND)
    set(HAS_BACKTRACE_H true)
    option(USE_BACKTRACE "Generate a backtrace when a svnclient exception is thrown" OFF)
  endif()

endif()

#search libaries for Windows
if(WIN32)

  # search for pathes
  find_path(SUBVERSION_BIN_DIR svn.exe
    "${SUBVERSION_INSTALL_PATH}/bin"
    "$ENV{ProgramFiles}/Subversion/bin"
  )

  find_path(SUBVERSION_INCLUDE_DIR svn_client.h
    "${SUBVERSION_INSTALL_PATH}/include"
    "$ENV{ProgramFiles}/Subversion/include"
  )

  find_path(APR_INCLUDE_DIR apr.h
    "${SUBVERSION_INSTALL_PATH}/include/apr"
    "$ENV{ProgramFiles}/Subversion/include/apr"
  )

  find_path(APU_INCLUDE_DIR apu.h
    "${SUBVERSION_INSTALL_PATH}/include/apr-util"
    "$ENV{ProgramFiles}/Subversion/include/apr-util"
  )

  # search for libraries
  find_library(APR_LIB libapr-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr"
    "$ENV{ProgramFiles}/Subversion/lib/apr"
  )

  find_library(APRICONV_LIB libapriconv-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr-iconv"
    "$ENV{ProgramFiles}/Subversion/lib/apr-iconv"
  )

  find_library(APU_LIB libaprutil-1
    "${SUBVERSION_INSTALL_PATH}/lib/apr-util"
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  find_library(APU_XMLLIB xml
    "${SUBVERSION_INSTALL_PATH}/lib/apr-util"
    "$ENV{ProgramFiles}/Subversion/lib/apr-util"
  )

  find_library(NEON_LIB libneon
    "${SUBVERSION_INSTALL_PATH}/lib/neon"
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  find_library(NEON_ZLIBSTATLIB zlibstat
    "${SUBVERSION_INSTALL_PATH}/lib/neon"
    "$ENV{ProgramFiles}/Subversion/lib/neon"
  )

  find_library(INTL3LIB intl3_svn
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(DB44_LIB libdb44
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_CLIENTLIB libsvn_client-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_DELTALIB libsvn_delta-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_DIFFLIB libsvn_diff-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_FSBASELIB libsvn_fs_base-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_FSFSLIB libsvn_fs_fs-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_FSUTILLIB libsvn_fs_util-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_FSLIB libsvn_fs-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_RALOCALLIB libsvn_ra_local-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_RANEONLIB libsvn_ra_neon-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_RASVNLIB libsvn_ra_svn-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_RALIB libsvn_ra-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_REPOSITORYLIB libsvn_repos-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_SUBRLIB libsvn_subr-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  find_library(SUBVERSION_WCLIB libsvn_wc-1
    "${SUBVERSION_INSTALL_PATH}/lib"
    "$ENV{ProgramFiles}/Subversion/lib"
  )

  set(APR_EXTRA_LIBS )
  set(APR_EXTRA_LDFLAGS )
  set(APU_EXTRA_LIBS )
  set(APU_EXTRA_LDFLAGS )


  # check found libraries
  if(NOT APR_LIB)
    message(SEND_ERROR "No apr lib found!")
  else()
    message(STATUS "Found apr lib: ${APR_LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APR_LIB} ${APR_EXTRA_LIBS})
  endif()

  if(NOT APRICONV_LIB)
    message(SEND_ERROR "No apriconv lib found!")
  else()
    message(STATUS "Found apriconv lib: ${APRICONV_LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APRICONV_LIB})
  endif()

  if(NOT APU_LIB)
    message(SEND_ERROR "No aprutil lib found!")
  else()
    message(STATUS "Found aprutil lib: ${APU_LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_LIB} ${APU_EXTRA_LIBS})
  endif()

  if(NOT APU_XMLLIB)
    message(SEND_ERROR "No xml lib found!")
  else()
    message(STATUS "Found xml lib: ${APU_XMLLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${APU_XMLLIB})
  endif()

  if(NOT NEON_LIB)
    message(SEND_ERROR "No neon lib found!")
  else()
    message(STATUS "Found neon lib: ${NEON_LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_LIB})
  endif()

  if(NOT NEON_ZLIBSTATLIB)
    message(SEND_ERROR "No zlibstat lib found!")
  else()
    message(STATUS "Found zlibstat lib: ${NEON_ZLIBSTATLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${NEON_ZLIBSTATLIB})
  endif()

  if(NOT INTL3LIB)
    message(SEND_ERROR "No intl3 lib found!")
  else()
    message(STATUS "Found intl3 lib: ${INTL3LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${INTL3LIB})
  endif()

  if(NOT DB44_LIB)
    message(SEND_ERROR "No db44 lib found!")
  else()
    message(STATUS "Found db44 lib: ${DB44_LIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${DB44_LIB})
  endif()

  if(NOT SUBVERSION_DELTALIB)
    message(SEND_ERROR "No subversion delta lib found!")
  else()
    message(STATUS "Found subversion delta lib: ${SUBVERSION_DELTALIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DELTALIB})
  endif()

  if(NOT SUBVERSION_FSBASELIB)
    message(SEND_ERROR "No subversion fs base lib found!")
  else()
    message(STATUS "Found subversion fs base lib: ${SUBVERSION_FSBASELIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSBASELIB})
  endif()

  if(NOT SUBVERSION_FSFSLIB)
    message(SEND_ERROR "No subversion fs fs lib found!")
  else()
    message(STATUS "Found subversion fs fs lib: ${SUBVERSION_FSFSLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSFSLIB})
  endif()

  if(NOT SUBVERSION_FSUTILLIB)
    message(SEND_ERROR "No subversion fs util lib found!")
  else()
    message(STATUS "Found subversion fs util lib: ${SUBVERSION_FSUTILLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSUTILLIB})
  endif()

  if(NOT SUBVERSION_RALOCALLIB)
    message(SEND_ERROR "No subversion ra local lib found!")
  else()
    message(STATUS "Found subversion ra local lib: ${SUBVERSION_RALOCALLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALOCALLIB})
  endif()

  if(NOT SUBVERSION_RANEONLIB)
    message(SEND_ERROR "No subversion ra neon lib found!")
  else()
    message(STATUS "Found subversion ra neon lib: ${SUBVERSION_RANEONLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RANEONLIB})
  endif()

  if (NOT SUBVERSION_RASVNLIB)
    message(SEND_ERROR "No subversion ra svn lib found!")
  else()
    message(STATUS "Found subversion ra svn lib: ${SUBVERSION_RASVNLIB}")
    set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RASVNLIB})
  endif()
endif()

if(NOT SUBVERSION_INCLUDE_DIR)
  message(SEND_ERROR "No subversion includes found!")
else()
  message(STATUS "Found subversion include: ${SUBVERSION_INCLUDE_DIR}")
endif()

if(NOT SUBVERSION_CLIENTLIB)
 message(SEND_ERROR "No subversion client libs found!")
else()
 message(STATUS "Found subversion client lib: ${SUBVERSION_CLIENTLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_CLIENTLIB})
endif()

if(NOT SUBVERSION_DIFFLIB)
 message(SEND_ERROR "No subversion diff lib found!")
else()
 message(STATUS "Found subversion diff lib: ${SUBVERSION_DIFFLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_DIFFLIB})
endif()

if(NOT SUBVERSION_FSLIB)
 message(SEND_ERROR "No subversion fs lib found!")
else()
 message(STATUS "Found subversion fs lib: ${SUBVERSION_FSLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_FSLIB})
endif()

if(NOT SUBVERSION_RALIB)
 message(SEND_ERROR "No subversion ra lib found!")
else()
 message(STATUS "Found subversion ra lib: ${SUBVERSION_RALIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_RALIB})
endif()

if(NOT SUBVERSION_REPOSITORYLIB)
 message(SEND_ERROR "No subversion repository lib found!")
else()
 message(STATUS "Found subversion repository lib: ${SUBVERSION_REPOSITORYLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_REPOSITORYLIB})
endif()

if(NOT SUBVERSION_SUBRLIB)
 message(SEND_ERROR "No subversion subr lib found!")
else()
 message(STATUS "Found subversion subr lib: ${SUBVERSION_SUBRLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_SUBRLIB})
endif()

if(NOT SUBVERSION_WCLIB)
 message(SEND_ERROR "No subversion wc lib found!")
else()
 message(STATUS "Found subversion wc lib: ${SUBVERSION_WCLIB}")
 set(SUBVERSION_ALL_LIBS ${SUBVERSION_ALL_LIBS} ${SUBVERSION_WCLIB})
endif()

set(SUBVERSIONFOUND true)
