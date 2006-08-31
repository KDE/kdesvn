include(FindQt3)
include(FindKDE3)
include(kdesvnMacros)

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

FIND_PROGRAM(KDE3_KDECONFIG_EXECUTABLE NAME kde-config PATHS
    $ENV{KDEDIR}/bin
    /opt/kde/bin
    /opt/kde3/bin
)

FIND_PROGRAM(MSGFMT
    NAMES gmsgfmt msgfmt)

FIND_PROGRAM(STRIP
    NAMES strip)

FIND_PROGRAM(KDE3_MEINPROC_EXECUTABLE NAME meinproc PATHS
     ${KDE3_BIN_INSTALL_DIR}
     $ENV{KDEDIR}/bin
     /opt/kde/bin
     /opt/kde3/bin
)

IF(KDE3_MEINPROC_EXECUTABLE)
    MESSAGE(STATUS "Found meinproc: ${KDE3_MEINPROC_EXECUTABLE}")
ELSE(KDE3_MEINPROC_EXECUTABLE)
    MESSAGE(STATUS "Didn't find meinproc!")
ENDIF(KDE3_MEINPROC_EXECUTABLE)

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

# applicationsdir for kde has an own subdir! (where the desktop-files goes)
IF (NOT KDE3_APPLICATIONSDIR)
    SET(KDE3_APPLICATIONSDIR ${KDE3_DATADIR}/applications/kde CACHE STRING
        "KDE3-applications dir (relative to install-prefix when not starting with \"/\")" )
ENDIF (NOT KDE3_APPLICATIONSDIR)

# linker flags - must get checked
SET(LINK_NO_UNDEFINED "")
SET(lundef "-Wl,--no-undefined")
KDESVN_CHECK_LINK_FLAG(${lundef} _NO_UNDEFINED)
IF (_NO_UNDEFINED)
    SET(LINK_NO_UNDEFINED "${lundef}")
ENDIF (_NO_UNDEFINED)

SET(lundef "-Wl,--allow-shlib-undefined")
KDESVN_CHECK_LINK_FLAG(${lundef} _ALLOW_SHLIB)
IF (_ALLOW_SHLIB)
    SET(LINK_NO_UNDEFINED "${LINK_NO_UNDEFINED} ${lundef}")
ENDIF (_ALLOW_SHLIB)


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
		set(_sizestring "scalable")
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

MACRO(ADD_POFILES packagename)
    SET(_gmofiles)
    FILE(GLOB _pofiles *.po)

    FOREACH(_current_po ${_pofiles})
        GET_FILENAME_COMPONENT(_name ${_current_po} NAME_WE)
        STRING(REGEX REPLACE "^.*/([a-zA-Z]+)(\\.po)" "\\1" _lang "${_current_po}")
        SET(_gmofile "${CMAKE_CURRENT_BINARY_DIR}/${_name}.gmo")
        ADD_CUSTOM_COMMAND(OUTPUT ${_gmofile}
            COMMAND ${MSGFMT}
            ARGS "-o" "${_gmofile}" "${_current_po}"
            DEPENDS ${_current_po}
            )
        SET(_gmofiles ${_gmofiles} ${_gmofile})
        INSTALL(FILES ${_gmofile}
            DESTINATION ${KDE3_LOCALEDIR}/${_lang}/LC_MESSAGES
            RENAME ${packagename}.mo)
    ENDFOREACH(_current_po ${_pofiles})

    ADD_CUSTOM_TARGET(translations ALL
        DEPENDS ${_gmofiles})

ENDMACRO(ADD_POFILES)
