/***************************************************************************
 *   Copyright (C) 2005-2010 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU General Public                     *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU General Public               *
 * License along with this program (in the file GPL.txt); if not,          *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

#ifndef CHARTBASEVIEW_H
#define CHARTBASEVIEW_H

#include <QAbstractItemView>
#include <QFlags>

class ChartBaseView : public QAbstractItemView
{
    Q_OBJECT

    public:
        enum {BASE=0, BARVIEW=1,PIEVIEW=2};
        ChartBaseView(QWidget *parent = 0);
        virtual ~ChartBaseView();

        virtual int bartype()const{return BASE;}

        virtual void setChartTitle(const QVariant& theValue, int role = Qt::DisplayRole)=0;
        virtual void setXTitle(const QVariant& theValue, int role = Qt::DisplayRole)=0;
        virtual void setYTitle(const QVariant& theValue, int role = Qt::DisplayRole)=0;
        virtual void setMinimumBarWidth(uint){};

        inline void setItemDelegate(QAbstractItemDelegate *dele){QAbstractItemView::setItemDelegate(dele);}

        virtual void setFirstColumnIsLegend(bool how){_firstColIsLegend=how;}
        virtual bool firstColumnIsLegend()const{return _firstColIsLegend;}

        virtual void setMaximumValue(const int& theValue);
        virtual int maximumValue() const;
        virtual void setMinimumValue(const int& theValue);
        virtual int minimumValue() const;
        virtual void setCanvasMargins(QSize s);
        virtual QSize canvasMargins();

        virtual uint barBorder()const;
        virtual void setBarBorder(uint);

        virtual QSize valueSize() const;

    protected:
        bool _firstColIsLegend;

        int _minVal;
        int _maxVal;
        uint _canvasHMargin;
        uint _canvasVMargin;

        uint _barBorder;
};

#endif
