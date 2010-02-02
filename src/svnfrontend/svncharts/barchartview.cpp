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

#include "barchartview.h"
#include "barchartviewdata.h"

#include <QScrollBar>

#define KDESVN_DEFINE_SETGET(preg,pres)\
    QVariant BarChartView::preg(int role)const\
    {\
        return _data->preg(role);\
    }\
    void BarChartView::pres(const QVariant&value,int role)\
    {\
        _data->pres(value,role);\
    }

BarChartView::BarChartView(QWidget *parent)
    :ChartBaseView(parent),_data(new BarChartViewData)
{
    _displayFlags.set();
    updateGeometries();
}

BarChartView::~BarChartView()
{
}

void BarChartView::updateGeometries()
{
    // x-direction
    _data->_seriesWidth=calcColumnSeriesWidth();
    _data->_diagramWidth=calcColumnDiagramWidth();
    _data->_diagramXOffset=calcLeftOffset();

    int viewWidth = _data->_diagramWidth+_data->_diagramXOffset+_canvasHMargin;

    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, viewWidth-viewport()->width()));

    // y-direction
    _data->_diagramBottomOffset = calcBottomOffset();
    _data->_diagramTopOffset = calcTopOffset();
    int chartHeight = qMax(viewport()->height()-_data->_diagramBottomOffset-_data->_diagramTopOffset,_data->_minChartHeight+_data->_maxValueOffset);
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0,qMax(0,chartHeight-viewport()->height()));
}

void BarChartView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {
        for (int col = minColumn(); col<model()->columnCount(rootIndex());++col) {
            QModelIndex index = model()->index(row, col, rootIndex());
            int value = model()->data(index).toInt();
            if (value>maximumValue()) {
                setMaximumValue(qRound((double)value/50.0+0.5)*50);
            }
        }
    }
    QAbstractItemView::rowsInserted(parent, start, end);
    updateGeometries();
}

uint BarChartView::minColumn()const
{
    return firstColumnIsLegend()?1:0;
}

uint BarChartView::minimumBarWidth()const
{
    return _data->_minimumBarWidth;
}

void BarChartView::setMinimumBarWidth(uint width)
{
    _data->_minimumBarWidth=width;
}

int BarChartView::verticalScaleWidth()const
{
    return _data->_verticalScaleWidth;
}

void BarChartView::setVerticalScaleWidth(int value)
{
    _data->_verticalScaleWidth=value;
}

int BarChartView::columnSeries()const
{
    if (!model()) {
        return 0;
    }
    return model()->columnCount(rootIndex())-minColumn();
}

int BarChartView::columnSeries(const QModelIndex & index)const
{
    return (index.isValid()?index.model()->columnCount()-minColumn():0);
}

/*
 * Setters / Getters for Data
 */

KDESVN_DEFINE_SETGET(yTitle,setYTitle);
KDESVN_DEFINE_SETGET(xTitle,setXTitle);
KDESVN_DEFINE_SETGET(chartTitle,setChartTitle);

QSize BarChartView::yTitleSize()const
{
    QSize s(0,0);
    if (!_displayFlags.test(ShowYTitle)) {
        return s;
    }
    QFont _f;
    QVariant v = yTitle(Qt::FontRole);
    if(v.isValid()) {
        _f = qvariant_cast<QFont>(v);
    } else {
        _f = KGlobalSettings::generalFont();
    }
    s.setHeight(QFontMetrics(_f).height());

    ///@todo use the height of the chart for the width of the line
    v = yTitle(Qt::DisplayRole);
    if (v.isValid()) {
        s.setWidth(QFontMetrics(_f).width(qvariant_cast<QString>(v)));
    }
    return s;
}

QSize BarChartView::xTitleSize()const
{
    QSize s(0,0);
    if (!_displayFlags.test(ShowXTitle)) {
        return s;
    }
    QFont _f;
    QVariant v = xTitle(Qt::FontRole);
    if(v.isValid()) {
        _f = qvariant_cast<QFont>(v);
    } else {
        _f = KGlobalSettings::generalFont();
    }
    s.setHeight(QFontMetrics(_f).height());
    s.setWidth(_data->_diagramWidth);
    return s;
}

QSize BarChartView::chartTitleSize()const
{
    QSize s(0,0);
    if (!_displayFlags.test(ShowChartTitle)) {
        return s;
    }
    QFont _f;
    QVariant v = chartTitle(Qt::FontRole);
    if(v.isValid()) {
        _f = qvariant_cast<QFont>(v);
    } else {
        _f = KGlobalSettings::generalFont();
    }
    s.setHeight(QFontMetrics(_f).height());
    s.setWidth(_data->_diagramWidth-barBorder());
    return s;
}

/*
 * Setters / Getters for Data end
 */

/*
 * general calculations
 */

uint BarChartView::calcLeftOffset()const
{
    return _canvasHMargin+yTitleSize().height()+_displayFlags.test(ShowYLegend)*valueSize().width()+_displayFlags.test(ShowYTitle)*_data->_ytitleBorder+_displayFlags.test(ShowYLegend)*_data->_ytitleBorder;
}

uint BarChartView::calcBottomOffset()const
{
    return _canvasVMargin+xTitleSize().height()+_displayFlags.test(ShowXTitle)*_data->_xtitleBorder+_displayFlags.test(ShowXLegend)*_data->_xtitleBorder;
}

uint BarChartView::calcTopOffset()const
{
    return _canvasVMargin+chartTitleSize().height()+_displayFlags.test(ShowChartTitle)*_data->_chartTitleYBorder;
}
/*
 * calculations for series in colums begin
 */
uint BarChartView::calcColumnSeriesWidth()const
{
    if (!model()||model()->rowCount(rootIndex())==0) {
        return 0;
    }
    int rows = model()->rowCount(rootIndex());
    return qMax(0,rows*((int)minimumBarWidth())+(rows-1)*((int)barBorder()));
}

uint BarChartView::calcColumnDiagramWidth()const
{
    return columnSeries()*_data->_seriesWidth+(columnSeries()-1)*_data->_seriesBorder+2*barBorder();
}

uint BarChartView::columnSeriesStart(const QModelIndex&index)const
{
    if (!index.isValid()) {
        return 0;
    }
    return qMax(0,(int)(_data->_diagramXOffset+barBorder()+index.column()-minColumn()*(_data->_seriesWidth+_data->_seriesBorder)));
}

uint BarChartView::columnItemBarLeft(const QModelIndex&index)const
{
    if (!index.isValid()) {
        return 0;
    }
    return columnSeriesStart(index)+index.row()*(minimumBarWidth()+barBorder());
}
/*
 * calculations for series in colums end
 */


#include "barchartview.moc"
