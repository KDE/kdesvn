INCLUDE(TestCXXAcceptsFlag)

CHECK_CXX_ACCEPTS_FLAG("-fexceptions" CXX_EXCEPTION)
IF (CXX_EXCEPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
ENDIF (CXX_EXCEPTION)

# check if we can use setenv
TRY_COMPILE(HAS_SETENV
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/cmakemodules/TestSetenv.cxx
        OUTPUT_VARIABLE OUTPUT
)

IF (HAS_SETENV)
    MESSAGE(STATUS "Checking for setenv - yes")
ELSE (HAS_SETENV)
    MESSAGE(STATUS "Checking for setenv - no")
ENDIF (HAS_SETENV)
