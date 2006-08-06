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

# we require some defines
SET(KDE3_DATADIR share/apps CACHE STRING "KDE3-datadir (relative to install-prefix when not starting with \"/\")" )
SET(KDE3_BINDIR bin CACHE STRING "KDE3-binarydir (relative to install-prefix when not starting with \"/\")" )

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

      INSTALL(FILES ${_current_ICON} DESTINATION ${_ICON_INSTALL_DIR} RENAME
	  ${_name})

   ENDFOREACH (_current_ICON)
ENDMACRO(KDESVN_INSTALL_ICONS)

