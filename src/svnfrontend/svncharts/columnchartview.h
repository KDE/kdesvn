/***************************************************************************
 *   Copyright (C) 2006 by Witold Wysota                                   *
 *   wysota@qtcentre.org                                                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef COLUMNCHARTVIEW_H
#define COLUMNCHARTVIEW_H

#include <QAbstractItemView>
#include <QFlags>
#include "chartbardelegate.h"
#include "chartbaseview.h"

/**

@todo legenda
@todo warto¶ci ujemne
@todo obracanie podpisów

*/

class ChartLegend {
public:
    enum LegendFlag {
    	NoFlags=0, Shadow=1, SeriesClickable=2
    };
    Q_DECLARE_FLAGS(LegendFlags, LegendFlag)
    ChartLegend(LegendFlags f=NoFlags){ m_flags = f;}
    LegendFlags flags() const{ return m_flags;}
    void setFlags(LegendFlags f){ m_flags = f; }
protected:
    LegendFlags m_flags;
};


/**
    @author Witold Wysota <wysota@qtcentre.org>
*/
class ColumnChartView : public ChartBaseView
{
    Q_OBJECT
public:
    enum { Legend=1, ChartTitle=2, XTitle=4, YTitle=8, VerticalGrid=256, VerticalScale=512  };
    ColumnChartView(QWidget *parent = 0);

    virtual ~ColumnChartView();
    QModelIndex indexAt ( const QPoint & point ) const;
    void scrollTo ( const QModelIndex & index, ScrollHint hint = EnsureVisible );
    QRect visualRect ( const QModelIndex & index ) const;

    int itemWidth() const;
    QSize chartSize() const;
    quint32 flags() const;
    uint verticalGridLines() const;
    uint minorVerticalGridLines() const;
    QVariant chartTitle(int role = Qt::DisplayRole) const;
    QVariant xTitle(int role = Qt::DisplayRole) const;
    QVariant yTitle(int role = Qt::DisplayRole) const;
    inline ChartLegend legend() const { return _legend; }

    virtual void setVerticalGridLines(const uint& theValue);
    virtual void setChartSize(const QSize& theValue);
    virtual void setFlags(const quint32& theValue);
    virtual void setMinorVerticalGridLines(const uint& theValue);
    virtual void setChartTitle(const QVariant& theValue, int role = Qt::DisplayRole);
    virtual void setXTitle(const QVariant& theValue, int role = Qt::DisplayRole);
    virtual void setYTitle(const QVariant& theValue, int role = Qt::DisplayRole);
    virtual void setMinimumBarWidth(uint v);
    virtual uint minimumBarWidth()const{return _minimumBarWidth;}

    void setLegend(const ChartLegend &l){ _legend = l; }

    QString rowLegendName(int row)const;
    QVariant rowLegendValue(int row,int role=Qt::DisplayRole)const;

    int minColumn()const;
    int series(const QModelIndex&)const;
    int series()const;

    virtual int bartype()const{return BARVIEW;}

Q_SIGNALS:
    void xTitleChanged();
    void yTitleChanged();
    void chartTitleChanged();

public Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);

protected:
    void paintEvent(QPaintEvent *event);
    void updateGeometries();
    void scrollContentsBy(int dx, int dy);

    void resizeEvent(QResizeEvent * /* event */);
    void mouseReleaseEvent(QMouseEvent *event);
    bool event ( QEvent * e );
    void paintItem(QPainter *p, const QModelIndex &index);
    bool isIndexHidden ( const QModelIndex & index ) const;
    int horizontalOffset () const;
    int verticalOffset () const;
    QModelIndex moveCursor ( CursorAction cursorAction, Qt::KeyboardModifiers modifiers );
    void setSelection ( const QRect & rect, QItemSelectionModel::SelectionFlags flags);
    QRegion visualRegionForSelection ( const QItemSelection & selection ) const;

    uint _vertGridLines;
    uint _minorVertGridLines;
    uint _minimumBarWidth;
    QMap<uint, QVariant> _chartTitle;
    QMap<uint, QVariant> _xTitle;
    QMap<uint, QVariant> _yTitle;

    virtual void drawGridLines(QPainter *p);
    virtual void drawVerticalScale(QPainter *p);
    virtual void drawAxis(QPainter *p);
    virtual void drawLegend(QPainter *p);
    virtual void drawRowNames(QPainter *p);
    virtual void drawChartTitle(QPainter *p);
    virtual void drawXTitle(QPainter *p);
    virtual void drawYTitle(QPainter *p);
    virtual void drawCanvas(QPainter *p);

    QSize _chartSize;
    QRect selectionRect;
    ChartLegend _legend;

    uint horizontalMargins() const;
    virtual QSize chartTitleSize() const;
    virtual QSize xTitleSize() const;
    virtual QSize yTitleSize() const;
    virtual QSize rowNameSize() const;
    virtual QSize valueSize() const;

protected Q_SLOTS:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    Q_DISABLE_COPY(ColumnChartView);
    void setItemDelegate(QAbstractItemDelegate *){};
    uint _legendwidth;
    QStyleOptionViewItem getOptionsForRowLabel(int row);
    quint32 _flags;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(ChartLegend::LegendFlags)

#endif
