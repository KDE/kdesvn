#first straigth forward until we have a doc translation
IF (KDE3_MEINPROC_EXECUTABLE)
    FILE(GLOB docimages en/*.png)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/en_index.cache.bz2
        COMMAND
        ${KDE3_MEINPROC_EXECUTABLE}
        ARGS "--check" "--cache" "${CMAKE_CURRENT_BINARY_DIR}/en_index.cache.bz2"
        ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook
        DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook
        ${docimages}
    )
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/common
        COMMAND
        ${CMAKE_CURRENT_SOURCE_DIR}/makecommon.sh
        ARGS
        "${CMAKE_CURRENT_BINARY_DIR}/common"
        "${CMAKE_INSTALL_PREFIX}/${KDE3_DATADIR}/doc/HTML/en/common"
    )
    INSTALL(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/en_index.cache.bz2
        ${CMAKE_CURRENT_SOURCE_DIR}/en/index.docbook
        ${docimages}
        DESTINATION ${KDE3_DATADIR}/doc/HTML/en/kdesvn
    )
    ADD_CUSTOM_TARGET(doc ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/en_index.cache.bz2 ${CMAKE_CURRENT_BINARY_DIR}/common)

ENDIF (KDE3_MEINPROC_EXECUTABLE)
