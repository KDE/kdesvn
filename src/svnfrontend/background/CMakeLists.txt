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
############################################################################

INCLUDE_DIRECTORIES(
                    ${CMAKE_BINARY_DIR}/src/svnqt
                    )

set (backgroundsrc
    fillcachethread.cpp
    fillcachethread.h
    modifiedthread.cpp
    modifiedthread.h
    svnthread.cpp
    svnthread.h
    tcontextlistener.cpp
    tcontextlistener.h
    threadcontextlistenerdata.cpp
    threadcontextlistenerdata.h
    getinfothread.cpp
    getinfothread.h
    )

KDE_AUTOMOC(${backgroundsrc})

ADD_LIBRARY(svnbackground STATIC
    ${backgroundsrc}
)

SET_TARGET_PROPERTIES(svnbackground
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

ADD_DEPENDENCIES(svnbackground ksvnwidgets)
