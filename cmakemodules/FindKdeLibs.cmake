include(${CMAKE_ROOT}/Modules/FindQt3.cmake)
include(${CMAKE_ROOT}/Modules/FindKDE3.cmake)

FIND_LIBRARY(KDE3_UI_LIBRARY NAMES kdeui
  PATHS
  ${KDE3_LIB_DIR}
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(KDE3_PART_LIBRARY NAMES kparts
  PATHS
  ${KDE3_LIB_DIR}
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(KDE3_KIO_LIBRARY NAMES kio
  PATHS
  ${KDE3_LIB_DIR}
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  /usr/lib
  /usr/local/lib
)

FIND_LIBRARY(KDE3_DCOP_LIBRARY NAMES DCOP
  PATHS
  ${KDE3_LIB_DIR}
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  /usr/lib
  /usr/local/lib
)

FIND_PROGRAM(MSGFMT
    NAMES gmsgfmt msgfmt)

IF(MSGFMT)
    EXECUTE_PROCESS(COMMAND ${MSGFMT} "--version" "2>&1"
    OUTPUT_VARIABLE _msgout)
    STRING(REGEX MATCH "GNU[\t ]gettext" _isgnu "${_msgout}")
    IF (NOT _isgnu)
        MESSAGE(STATUS "No gnu msgfmt found!")
        SET(MSGFMT ":" CACHE STRING "Msgfmt program")
    ELSE(NOT _isgnu)
        MESSAGE(STATUS "Found gnu msgfmt: ${MSGFMT}")
    ENDIF (NOT _isgnu)
ELSE(MSGFMT)
    SET(MSGFMT ":" CACHE STRING "Msgfmt program")
ENDIF(MSGFMT)

# we require some defines
IF (NOT KDE3_DATADIR)
    SET(KDE3_DATADIR share CACHE STRING
        "KDE3-datadir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_DATADIR)
IF (NOT KDE3_BINDIR)
    SET(KDE3_BINDIR bin CACHE STRING
        "KDE3-binarydir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_BINDIR)
IF (NOT KDE3_APPSDIR)
    SET(KDE3_APPSDIR ${KDE3_DATADIR}/apps CACHE STRING
        "KDE3-application dir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_APPSDIR)
IF (NOT KDE3_SERVICESDIR)
    SET(KDE3_SERVICESDIR ${KDE3_DATADIR}/services CACHE STRING
        "KDE3-services dir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_SERVICESDIR)
IF (NOT KDE3_LOCALEDIR)
    SET(KDE3_LOCALEDIR ${KDE3_DATADIR}/locale CACHE STRING
        "KDE3-locale dir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_LOCALEDIR)
IF (NOT KDE3_ICONDIR)
    SET(KDE3_ICONDIR ${KDE3_DATADIR}/icons CACHE STRING
        "KDE3-icons dir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_ICONDIR)

# linker flags - must get checked
SET(LINK_NO_UNDEFINED "-Wl,--no-undefined -Wl,--allow-shlib-undefined")

# own macros
MACRO(KDESVN_INSTALL_ICONS _theme)

   FILE(GLOB _icons *.png)
   FILE(GLOB _svg *svgz)
   SET(_icons ${_icons} ${_svg})
   FOREACH(_current_ICON ${_icons} )
      GET_FILENAME_COMPONENT(_ctype ${_current_ICON} EXT)

	  if (${_ctype} STREQUAL ".png")
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\1" _size "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\2" _group "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/[a-zA-Z]+([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\3" _name "${_current_ICON}")
	  set(_sizestring "${_size}x${_size}")
	  endif (${_ctype} STREQUAL ".png")

	  if (${_ctype} STREQUAL ".svgz")
        STRING(REGEX REPLACE "^.*/[a-zA-Z]+\\-([a-z]+)\\-(.+\\.svgz)$" "\\1" _group "${_current_ICON}")
        STRING(REGEX REPLACE "^.*/[a-zA-Z]+\\-([a-z]+)\\-(.+\\.svgz)$" "\\2" _name "${_current_ICON}")
		set(_sizestring "scaleable")
	  endif (${_ctype} STREQUAL ".svgz")

      SET(_icon_GROUP "actions")

      IF(${_group} STREQUAL "mime")
         SET(_icon_GROUP  "mimetypes")
      ENDIF(${_group} STREQUAL "mime")

      IF(${_group} STREQUAL "filesys")
         SET(_icon_GROUP  "filesystems")
      ENDIF(${_group} STREQUAL "filesys")

      IF(${_group} STREQUAL "device")
         SET(_icon_GROUP  "devices")
      ENDIF(${_group} STREQUAL "device")

      IF(${_group} STREQUAL "app")
         SET(_icon_GROUP  "apps")
      ENDIF(${_group} STREQUAL "app")

      IF(${_group} STREQUAL "action")
         SET(_icon_GROUP  "actions")
      ENDIF(${_group} STREQUAL "action")

      #message(STATUS "icon: ${_current_ICON} size: ${_sizestring} group: ${_group} name: ${_name}" )
      SET(_ICON_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/icons/${_theme}/${_sizestring}/${_icon_GROUP})

      INSTALL(FILES ${_current_ICON} DESTINATION ${_ICON_INSTALL_DIR} RENAME ${_name})

   ENDFOREACH (_current_ICON)
ENDMACRO(KDESVN_INSTALL_ICONS)

MACRO(ADD_POFILES packagename _molistvar)
    FILE(GLOB _pofiles *.po)
    FOREACH(_current_po ${_pofiles})
        GET_FILENAME_COMPONENT(_name ${_current_po} NAME_WE)
        STRING(REGEX REPLACE "^.*/([a-zA-Z]+)(\\.po)" "\\1" _lang "${_current_po}")
        SET(_gmofile "${_name}.gmo")
        ADD_CUSTOM_COMMAND(OUTPUT ${_gmofile}
            COMMAND ${MSGFMT}
            ARGS "-o" "${_gmofile}" "${_current_po}"
            DEPENDS ${_current_po}
            )
        SET(${_molistvar} ${${_molistvar}} ${_gmofile})
        INSTALL(FILES ${_gmofile}
            DESTINATION ${KDE3_LOCALEDIR}/${_lang}/LC_MESSAGES
            RENAME ${packagename}.mo)
    ENDFOREACH(_current_po ${_pofiles})
ENDMACRO(ADD_POFILES)
