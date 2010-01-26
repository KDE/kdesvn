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

####
# The base code for bar graph is taken from http://www.wysota.eu.org/charts.html
# see copyright statement in header of source files which were taken
#####

INCLUDE_DIRECTORIES(
                    ${CMAKE_BINARY_DIR}/src/svnqt
                    )

SET(chartssrc
    chartbardelegate.cpp
    chartbardelegate.h
    columnchartview.cpp
    columnchartview.h
    columnchartview_p.h
    dbstatistic.cpp
    dbstatistic.h
    statisticview.cpp
    statisticview.h
    chartstandarditem.cpp
    chartstandarditem.h
    chartbaseview.cpp
    chartbaseview.h
    pieview.cpp
    pieview.h
    simplechartmodel.cpp
    simplechartmodel.h
)

set (chartsui
    statisticview.ui
    )
KDE_AUTOMOC(${chartssrc})
KDE_ADD_UI_FILES(chartsui_src ${chartsui})

ADD_LIBRARY(svncharts STATIC
    ${chartssrc}
    ${chartsui_src}
)

SET_TARGET_PROPERTIES(svncharts
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
