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

MACRO(LRELEASE_TRANSLATION lrelease)
    SET(_qmFiles)
    FOREACH(tsfile ${ARGN})
        SET(_qmFile)
        GET_FILENAME_COMPONENT(_absFile ${tsfile} ABSOLUTE)
        GET_FILENAME_COMPONENT(_lang ${tsfile} NAME_WE)
        SET(_qmFile "${_lang}.qm")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${_qmFile}
            COMMAND ${lrelease} ${tsfile} -qm ${_qmFile}
            DEPENDS ${_absFile}
        )
        INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_qmFile} DESTINATION share/svnqt/i18n)
        SET(_qmFiles ${_qmFile} ${_qmFiles})
    ENDFOREACH(tsfile)
    ADD_CUSTOM_TARGET(svnqt-translations ALL DEPENDS ${_qmFiles})
ENDMACRO(LRELEASE_TRANSLATION)
