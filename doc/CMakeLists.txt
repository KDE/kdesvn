kdoctools_create_handbook(index.docbook INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en SUBDIR kdesvn)
kdoctools_create_manpage(man-kdesvn.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
kdoctools_create_manpage(man-kdesvnaskpass.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
