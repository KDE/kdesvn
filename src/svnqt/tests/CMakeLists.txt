####
 #   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        #
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

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h
)
add_test(rmrepo "/bin/rm" "-rvf" "${CMAKE_CURRENT_BINARY_DIR}/repo")
add_test(rmco "/bin/rm" "-rvf" "${CMAKE_CURRENT_BINARY_DIR}/co")
build_test(crepo)
build_test(lsdir)
build_test(ckpath)
