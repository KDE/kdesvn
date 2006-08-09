
MACRO(KDESVN_CHECK_LINK_FLAG FLAGS VARIABLE)
    IF(NOT DEFINED ${VARIABLE})
        TRY_COMPILE(${VARIABLE}
            ${CMAKE_BINARY_DIR}
            ${CMAKE_ROOT}/Modules/DummyCXXFile.cxx
            CMAKE_FLAGS -DCMAKE_EXE_LINKER_FLAGS="${FLAGS}"
            OUTPUT_VARIABLE OUTPUT)
        IF (${VARIABLE})
            MESSAGE(STATUS "Checking to see if linker accepts flag ${FLAGS} - yes")
            FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
                "Determining if the linker accepts the flag ${FLAGS} passed with "
                "the following output:\n${OUTPUT}\n\n")
        ELSE (${VARIABLE})
            MESSAGE(STATUS "Checking to see if linker accepts flag ${FLAGS} - no")
            FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
              "Determining if the linker accepts the flag ${FLAGS} failed with "
                "the following output:\n${OUTPUT}\n\n")
        ENDIF (${VARIABLE})
    ENDIF(NOT DEFINED ${VARIABLE})
ENDMACRO(KDESVN_CHECK_LINK_FLAG)
