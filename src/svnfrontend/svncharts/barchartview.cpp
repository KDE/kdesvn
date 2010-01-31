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

BarChartView::BarChartView(QWidget *parent)
    :ChartBaseView(parent),_data(new BarChartViewData)
{
    updateGeometries();
}

BarChartView::~BarChartView()
{
}

void BarChartView::updateGeometries()
{
    _data->_seriesWidth=calcColumnSeriesWidth();
    _data->_diagramWidth=calcColumnDiagramWidth();
    _data->_diagramXOffset=calcLeftOffset();
    _data->_chartSize.setWidth(_data->_diagramWidth+yTitleSize().height());

    int viewWidth = _data->_diagramWidth+_data->_diagramXOffset+yTitleSize().height();

    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, viewWidth-viewport()->width()));
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
 * general calculations
 */

uint BarChartView::calcLeftOffset()const
{
    return _canvasHMargin+yTitleSize().height()+valueSize().width()+2*_data->_ytitleBorder;
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
