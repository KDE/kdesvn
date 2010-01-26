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

#include "simplechartmodel.h"

#include <QList>
#include <QMap>
#include <QVariant>
#include <QColor>

class SimpleChartModelData
{
    public:
        SimpleChartModelData()
        :a(color_begin,color_begin,color_begin),r(0),g(0),b(0),colinc(0)
        {}

        QVariant value(int row,int column,int role)const
        {
            return _data[row][column][role];
        }

        void setValue(int row,int column, int role,QVariant value)
        {
            _data[row][column][role]=value;
        }
        QVariant countColor(int row)const;

    protected:
        typedef QMap<int,QVariant> valuesmap;
        typedef QMap<int,valuesmap> columnslist;
        typedef QMap<int,columnslist> rowslist;

        mutable rowslist _data;
        mutable QColor a;
        mutable int r,g,b;
        mutable uint colinc;

        static const int color_offset = 30;
        static const int color_begin = 50;
        static const int maxc=255-color_offset;
};

QVariant SimpleChartModelData::countColor(int row)const
{
    QVariant _v = value(row,1,Qt::DecorationRole);
    if (_v.isValid()) {
        return _v;
    }

    // generating new value
    a.setRgb(a.red()+color_offset,a.green()+color_offset,a.blue()+color_offset);
    // setting color value
    _v=qVariantFromValue(a);
    _data[row][1][Qt::DecorationRole]=_v;

    if( a.red()>maxc||a.green()>maxc||a.blue()>maxc ) {
        if (colinc==0) {
            ++colinc;
        } else if (r>=50||g>=50||b>=50) {
            if (++colinc>6) {
                colinc = 0;
                r=g=b=0;
            } else {
                r=g=b=-10;
            }
        }
        if (colinc & 0x1) {
            r+=10;
        }
        if (colinc & 0x2) {
            g+=10;
        }
        if (colinc & 0x4) {
            b+=10;
        }
        a.setRgb(color_begin+r,color_begin+g,color_begin+b);
    }
    return _v;
}

SimpleChartModel::SimpleChartModel(QObject*parent)
:QSqlQueryModel(parent),m_Data(new SimpleChartModelData)
{
}

SimpleChartModel::~SimpleChartModel()
{
}

bool SimpleChartModel::setData(const QModelIndex&index,const QVariant&value,int role)
{
    if (!index.isValid()||role==Qt::DisplayRole) {
        return QSqlQueryModel::setData(index,value,role);
    }
    m_Data->setValue(index.row(),index.column(),role,value);
    return true;
}

QVariant SimpleChartModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }
    if (role==Qt::DisplayRole) {
        return QSqlQueryModel::data(item,role);
    }
    // checking the color marker of the count column
    if (item.column()==1 && role==Qt::DecorationRole) {
        return m_Data->countColor(item.row());
    }
    return m_Data->value(item.row(),item.column(),role);
}

#include "simplechartmodel.moc"
