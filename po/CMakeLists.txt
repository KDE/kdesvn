FILE(GLOB pofiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po)

# disable unfinished/ancient translations
LIST(REMOVE_ITEM pofiles "nl.po")
LIST(REMOVE_ITEM pofiles "ru.po")

GETTEXT_CREATE_TRANSLATIONS("kdesvn.pot" "ALL" ${pofiles} )
