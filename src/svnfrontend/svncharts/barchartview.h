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

#include <bitset>

class BarChartViewData;

class BarChartView : public ChartBaseView
{
    Q_OBJECT
    public:
        enum DisplayFlag {
            ShowYTitle = 0,
            ShowXTitle = 1,
            ShowChartTitle=2,
            ShowHGrid=3,
            ShowVGrid=4,
            ShowXLegend=5,
            ShowYLegend=6,

            NotValid=7
        };
        // the c++ bitset is more usefull than the QFlags
        typedef std::bitset<NotValid> DisplayFlags;

        BarChartView(QWidget *parent = 0);
        virtual ~BarChartView();

        QVariant yTitle(int role = Qt::DisplayRole) const;
        QVariant xTitle(int role = Qt::DisplayRole) const;
        QVariant chartTitle(int role = Qt::DisplayRole) const;

        void setYTitle(const QVariant&,int role = Qt::DisplayRole);
        void setXTitle(const QVariant&,int role = Qt::DisplayRole);
        void setChartTitle(const QVariant&,int role = Qt::DisplayRole);

        void setDisplayFlag(DisplayFlag);
        void eraseDisplayFlag(DisplayFlag);
        const DisplayFlags&flags()const;

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
        uint calcBottomOffset()const;
        uint calcTopOffset()const;

        QSize yTitleSize()const;
        QSize xTitleSize()const;
        QSize chartTitleSize()const;

        DisplayFlags _displayFlags;
};

inline void BarChartView::setDisplayFlag(DisplayFlag fl)
{
    if (fl<NotValid) _displayFlags.set(fl);
}

inline void BarChartView::eraseDisplayFlag(DisplayFlag fl)
{
    if (fl<NotValid) _displayFlags.reset(fl);
}

inline const BarChartView::DisplayFlags& BarChartView::flags()const
{
    return _displayFlags;
}

#endif
