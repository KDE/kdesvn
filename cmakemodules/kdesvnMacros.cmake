
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

# this macro is required to add libtools to normal build so
# we may debug our part from within build dir
# output     name      where
# _laname    _soname   ${LIBRARY_OUTPUT_PATH}/kde3
MACRO(KDESVN_GENERATE_LIBTOOL_FILE _target)
   GET_TARGET_PROPERTY(_target_location ${_target} LOCATION)
   GET_FILENAME_COMPONENT(_laname ${_target_location} NAME_WE)
   GET_FILENAME_COMPONENT(_soname ${_target_location} NAME)
   IF(LIBRARY_OUTPUT_PATH)
    SET(_laname ${LIBRARY_OUTPUT_PATH}/${_laname}.la)
   ELSE(LIBRARY_OUTPUT_PATH)
    SET(_laname ${CMAKE_CURRENT_BINARY_DIR}/${_laname}.la)
   ENDIF(LIBRARY_OUTPUT_PATH)
   IF(LIBRARY_OUTPUT_PATH)
    SET(_libdir "'${LIBRARY_OUTPUT_PATH}/kde3'")
   ELSE(LIBRARY_OUTPUT_PATH)
    SET(_libdir "'${CMAKE_CURRENT_BUILD_DIR}/kde3'")
   ENDIF(LIBRARY_OUTPUT_PATH)
    ADD_CUSTOM_COMMAND(TARGET ${_target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS
    -DOUTPUTFILE:FILEPATH=${_laname}
    -DSONAME:STRING=${_soname}
    -DLIBDIR:STRING=${_libdir}
    -P ${CMAKE_SOURCE_DIR}/cmakemodules/generatelibtoolfile.cmake
    )
ENDMACRO(KDESVN_GENERATE_LIBTOOL_FILE)
