#first straigth forward until we have a doc translation

MACRO (GENBOOK _language_code)
    FILE(GLOB docimages RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code} ${_language_code}/hi128-filesys-*.png)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}_index.cache.bz2
        COMMAND
        ${KDE3_MEINPROC_EXECUTABLE}
        ARGS "--check" "--cache" "${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}_index.cache.bz2"
        ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}/index.docbook
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}/index.docbook
    )
    INSTALL(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}_index.cache.bz2
        DESTINATION ${HTML_INSTALL_DIR}/${_language_code}/kdesvn
        RENAME
        index.cache.bz2
    )
    INSTALL(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}/index.docbook
        DESTINATION ${HTML_INSTALL_DIR}/${_language_code}/kdesvn
    )

    FOREACH(_img ${docimages})
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}/${_img}
                        ${CMAKE_CURRENT_BINARY_DIR}/${_language_code}/${_img}
                        COPYONLY
                        )
        INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_language_code}/${_img}
                DESTINATION ${HTML_INSTALL_DIR}/${_language_code}/kdesvn
                )
    ENDFOREACH(_img)

    ADD_CUSTOM_TARGET(${_language_code}_doc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_language_code}_index.cache.bz2
    )
    ADD_DEPENDENCIES(doc ${_language_code}_doc)

ENDMACRO (GENBOOK)

IF (KDE3_MEINPROC_EXECUTABLE)
    ADD_CUSTOM_TARGET(doc ALL)
    GENBOOK(en)
    GENBOOK(nl)
    # creates the link to the common-section during install
    INSTALL(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/makelink.cmake)

#${CMAKE_CURRENT_BINARY_DIR}/common)

ENDIF (KDE3_MEINPROC_EXECUTABLE)

ADD_SUBDIRECTORY(man)

