MACRO(LRELEASE_TRANSLATION lrelease)
    SET(_qmFiles)
    FOREACH(tsfile ${ARGN})
        SET(_qmFile)
        GET_FILENAME_COMPONENT(_absFile ${tsfile} ABSOLUTE)
        GET_FILENAME_COMPONENT(_lang ${tsfile} NAME_WE)
        SET(_qmFile "${_lang}.qm")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${_qmFile}
            COMMAND ${lrelease} ${tsfile} -qm ${_qmFile}
            DEPENDS ${_absFile}
        )
        INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_qmFile} DESTINATION share/svnqt/i18n)
        SET(_qmFiles ${_qmFile} ${_qmFiles})
    ENDFOREACH(tsfile)
    ADD_CUSTOM_TARGET(svnqt-translations ALL DEPENDS ${_qmFiles})
ENDMACRO(LRELEASE_TRANSLATION)
