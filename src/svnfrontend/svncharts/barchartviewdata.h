/***************************************************************************
 *   Copyright (C) 2006-2010 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef BARCHARTVIEW_DATA_H
#define BARCHARTVIEW_DATA_H

#include <QSize>

class BarChartViewData
{
    public:
        BarChartViewData()
        : _chartSize(QSize()),_minimumBarWidth(40),_seriesWidth(0),_seriesBorder(20),_diagramWidth(0),_diagramOffset(0),_ytitleBorder(10),_verticalScaleWidth(20)
        {
        }

    QSize _chartSize;
    uint _minimumBarWidth;
    uint _seriesWidth;
    uint _seriesBorder;
    uint _diagramWidth;
    uint _diagramOffset;
    uint _ytitleBorder;
    int _verticalScaleWidth;
};

#endif
