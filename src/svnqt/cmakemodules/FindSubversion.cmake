FIND_PATH(SUBVERSION_INCLUDE_DIR svn_client.h
 /usr/include/subversion-1
 /usr/local/include/subversion-1
)

FIND_LIBRARY(SUBVERSION_CLIENTLIB svn_client-1
 /usr/lib
 /usr/local/lib
)

FIND_LIBRARY(SUBVERSION_REPOSITORYLIB svn_repos-1
 /usr/lib
 /usr/local/lib
)

FIND_PROGRAM(APR_CONFIG NAMES apr-config apr-1-config
 PATHS
 /usr/local/apr/bin
)

FIND_PROGRAM(APU_CONFIG NAMES apu-config apu-1-config
 PATHS
 /usr/local/apr/bin
)

IF(NOT SUBVERSION_INCLUDE_DIR)
  MESSAGE(SEND_ERROR "No subversion includes found!")
ENDIF(NOT SUBVERSION_INCLUDE_DIR)

if (NOT SUBVERSION_CLIENTLIB)
 MESSAGE(SEND_ERROR "No subversion client libs found!")
endif(NOT SUBVERSION_CLIENTLIB)

if (NOT SUBVERSION_REPOSITORYLIB)
 MESSAGE(SEND_ERROR "No subversion repository lib found!")
endif(NOT SUBVERSION_REPOSITORYLIB)

if(NOT APR_CONFIG)
 MESSAGE(SEND_ERROR "Error: no apr-config found")
endif(NOT APR_CONFIG)

if(NOT APU_CONFIG)
 MESSAGE(SEND_ERROR "Error: no apu-config found")
endif(NOT APU_CONFIG)

EXEC_PROGRAM(${APR_CONFIG} ARGS "--includedir" OUTPUT_VARIABLE APR_INCLUDE_DIR)
EXEC_PROGRAM(${APU_CONFIG} ARGS "--includedir" OUTPUT_VARIABLE APU_INCLUDE_DIR)
EXEC_PROGRAM(${APR_CONFIG} ARGS "--ldflags --libs --link-ld" OUTPUT_VARIABLE APR_EXTRA_LIBFLAGS)
EXEC_PROGRAM(${APU_CONFIG} ARGS "--ldflags --libs --link-ld" OUTPUT_VARIABLE APU_EXTRA_LIBFLAGS)



