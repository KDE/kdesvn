MACRO (GENBOOK _language_code)
    FILE(GLOB docimages RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} hi128-filesys-*.png)
    kde4_create_handbook(index.docbook
        INSTALL_DESTINATION ${HTML_INSTALL_DIR}/${_language_code} SUBDIR kdesvn)

    FOREACH(_img ${docimages})
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${_img}
                        ${CMAKE_CURRENT_BINARY_DIR}/${_img}
                        COPYONLY
                        )
        INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_img}
                DESTINATION ${HTML_INSTALL_DIR}/${_language_code}/kdesvn
                )
    ENDFOREACH(_img)
    ADD_DEPENDENCIES(doc ${_language_code}-handbook)

ENDMACRO (GENBOOK)

ADD_CUSTOM_TARGET(doc ALL)

ADD_SUBDIRECTORY(en)
ADD_SUBDIRECTORY(nl)
INSTALL(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/makelink.cmake)

ADD_SUBDIRECTORY(man)

