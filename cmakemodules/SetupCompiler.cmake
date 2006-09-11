INCLUDE(TestCXXAcceptsFlag)
INCLUDE(MacroEnsureVersion)

CHECK_CXX_ACCEPTS_FLAG("-fexceptions" CXX_EXCEPTION)
IF (CXX_EXCEPTION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
ENDIF (CXX_EXCEPTION)

IF (CMAKE_COMPILER_IS_GNUCXX)

CHECK_CXX_ACCEPTS_FLAG("-fstack-protector" CXX_PROTECTOR)
IF (CXX_PROTECTOR)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
ENDIF (CXX_PROTECTOR)

# some code from KDE4 trunk cmake - all stuff under LGPL I think
# check for visibility patch in libstdc++ below written by me
CHECK_CXX_ACCEPTS_FLAG("-fvisibility=hidden" HAVE_GCC_VISIBILITY)

# get the gcc version
exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE _gcc_version_info)
string (REGEX MATCH " [34]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
# gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
if (NOT _gcc_version)
    string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
endif (NOT _gcc_version)

macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)

if (GCC_IS_NEWER_THAN_4_1)
    exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
    string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
    IF (_GCC_COMPILED_WITH_BAD_ALLOCATOR)
        SET(HAVE_GCC_VISIBILITY FALSE)
    ENDIF (_GCC_COMPILED_WITH_BAD_ALLOCATOR)
else (GCC_IS_NEWER_THAN_4_1)
    set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
    CHECK_CXX_ACCEPTS_FLAG("-E" HAVE_PREPROCESSOR_FLAG)
    IF (HAVE_PREPROCESSOR_FLAG)
        FILE(WRITE ${CMAKE_BINARY_DIR}/dummy.cpp "#include <exception>\n")
        EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -E ${CMAKE_BINARY_DIR}/dummy.cpp OUTPUT_VARIABLE _gcc_hidden_push_out RESULT_VARIABLE _gcc_hidden_push_err ERROR_QUIET)
        FILE(REMOVE ${CMAKE_BINARY_DIR}/dummy.cpp)
        IF (NOT ${_gcc_hidden_push_err})
            STRING(REGEX MATCH "GCC visibility push" _gcc_has_hidden_push ${_gcc_hidden_push_out})
        ENDIF (NOT ${_gcc_hidden_push_err})
        IF(_gcc_has_hidden_push)
            MESSAGE(STATUS "libstdc++ is patched for visibility support.")
        ELSE(_gcc_has_hidden_push)
            MESSAGE(STATUS "libstdc++ isn't patched for visibility support. Disabling -fvisibility=hidden")
            SET(HAVE_GCC_VISIBILITY FALSE)
        ENDIF(_gcc_has_hidden_push)
    ELSE (HAVE_PREPROCESSOR_FLAG)
        # should never happen but who knows
        SET(HAVE_GCC_VISIBILITY FALSE)
    ENDIF (HAVE_PREPROCESSOR_FLAG)
endif (GCC_IS_NEWER_THAN_4_1)

message(STATUS "have_visibility: ${HAVE_GCC_VISIBILITY} version>=4.1: ${GCC_IS_NEWER_THAN_4_1} bad alloctor: ${_GCC_COMPILED_WITH_BAD_ALLOCATOR}")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)
