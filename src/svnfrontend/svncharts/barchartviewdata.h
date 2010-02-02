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

#include <kglobalsettings.h>

#include <QSize>

class BarChartViewData
{
    public:
        BarChartViewData()
        : _minimumBarWidth(40),_seriesWidth(0),_seriesBorder(20),_diagramWidth(0),_diagramXOffset(0),_diagramBottomOffset(0),_diagramTopOffset(0),
        _ytitleBorder(10),_xtitleBorder(10),_verticalScaleWidth(20),_chartTitleYBorder(15),
        _maxValueOffset(5),_minChartHeight(100)
        {
            QFont _f = KGlobalSettings::generalFont();
            _f.setPointSize(_f.pointSize()+2);
            _xTitle[Qt::FontRole]=_f;
            _yTitle[Qt::FontRole]=_f;

            _f.setPointSize(_f.pointSize()+2);
            _chartTitle[Qt::FontRole]=_f;
            _chartTitle[Qt::TextAlignmentRole]=Qt::AlignLeft;
        }

        QVariant yTitle(int role = Qt::DisplayRole) const;
        QVariant xTitle(int role = Qt::DisplayRole) const;
        QVariant chartTitle(int role = Qt::DisplayRole) const;

        void setYTitle(const QVariant&,int role = Qt::DisplayRole);
        void setXTitle(const QVariant&,int role = Qt::DisplayRole);
        void setChartTitle(const QVariant&,int role = Qt::DisplayRole);


        uint _minimumBarWidth;
        uint _seriesWidth;
        uint _seriesBorder;
        uint _diagramWidth;
        uint _diagramXOffset;
        uint _diagramBottomOffset;
        uint _diagramTopOffset;
        uint _ytitleBorder;
        uint _xtitleBorder;
        int _verticalScaleWidth;
        uint _chartTitleYBorder;
        uint _maxValueOffset;
        int _minChartHeight;

        QMap<uint, QVariant> _chartTitle;
        QMap<uint, QVariant> _xTitle;
        QMap<uint, QVariant> _yTitle;
};

inline QVariant BarChartViewData::yTitle(int role) const
{
    return _yTitle[role];
}

inline QVariant BarChartViewData::xTitle(int role) const
{
    return _xTitle[role];
}

inline QVariant BarChartViewData::chartTitle(int role) const
{
    return _chartTitle[role];
}

inline void BarChartViewData::setYTitle(const QVariant&value,int role)
{
    _yTitle[role]=value;
}

inline void BarChartViewData::setXTitle(const QVariant&value,int role)
{
    _xTitle[role]=value;
}

inline void BarChartViewData::setChartTitle(const QVariant&value,int role)
{
    _chartTitle[role]=value;
}

#endif
