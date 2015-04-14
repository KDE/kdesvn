kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR kdesvn)
kdoctools_create_manpage(man-kdesvn.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
kdoctools_create_manpage(man-kdesvnaskpass.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
