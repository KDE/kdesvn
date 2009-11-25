####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
 #   http://kdesvn.alwins-world.de/                                        #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 2 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, write to the                         #
 #   Free Software Foundation, Inc.,                                       #
 #   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         #
 ####

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

