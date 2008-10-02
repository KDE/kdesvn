FILE(GLOB pofiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po)

# disable unfinished/ancient translations
LIST(REMOVE_ITEM pofiles "cs.po")

GETTEXT_CREATE_TRANSLATIONS("kdesvn.pot" "ALL" ${pofiles} )
