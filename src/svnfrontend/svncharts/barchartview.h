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

#ifndef BARCHART_VIEW_H
#define BARCHART_VIEW_H

#include "src/svnqt/shared_pointer.h"

#include <QAbstractItemView>
#include <QFlags>
#include "chartbaseview.h"

class BarChartViewData;

class BarChartView : public ChartBaseView
{
    Q_OBJECT
    public:
        BarChartView(QWidget *parent = 0);
        virtual ~BarChartView();

    public Q_SLOTS:
        void rowsInserted(const QModelIndex &parent, int start, int end);
    protected Q_SLOTS:
        virtual void updateGeometries();

    protected:
        uint minColumn()const;

        uint minimumBarWidth()const;
        void setMinimumBarWidth(uint);

        int columnSeries()const;
        int columnSeries(const QModelIndex & index)const;

        int verticalScaleWidth()const;
        void setVerticalScaleWidth(int);

    private:
        svn::SharedPointer<BarChartViewData> _data;
        uint calcColumnSeriesWidth()const;
        uint calcColumnDiagramWidth()const;
        uint columnSeriesStart(const QModelIndex&index)const;
        uint columnItemBarLeft(const QModelIndex&index)const;
        uint calcLeftOffset()const;

        QSize yTitleSize()const;
};

#endif
