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

# code from KDE4 trunk cmake - all stuff under LGPL I think
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
else (GCC_IS_NEWER_THAN_4_1)
    set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
endif (GCC_IS_NEWER_THAN_4_1)

#if (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
#endif (HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

message(STATUS "have_visibility: ${HAVE_GCC_VISIBILITY} version>=4.1: ${GCC_IS_NEWER_THAN_4_1} bad alloctor: ${_GCC_COMPILED_WITH_BAD_ALLOCATOR}")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)
