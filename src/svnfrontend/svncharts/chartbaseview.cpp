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

#include "chartbaseview.h"

ChartBaseView::ChartBaseView(QWidget *parent)
:QAbstractItemView(parent),_firstColIsLegend(true)
{
    _minVal = 0;
    _maxVal = 100;
    _canvasHMargin = 30;
    _canvasVMargin = 30;

    _barBorder = 10;
}

ChartBaseView::~ChartBaseView()
{
}

void ChartBaseView::setMaximumValue( const int & theValue )
{
    _maxVal = theValue;
    viewport()->update();
}
int ChartBaseView::maximumValue( ) const
{
    return _maxVal;
}

void ChartBaseView::setMinimumValue( const int & theValue )
{
    _minVal = theValue;
    viewport()->update();
}

int ChartBaseView::minimumValue( ) const
{
    return _minVal;
}
QSize ChartBaseView::canvasMargins( )
{
    return QSize(_canvasHMargin, _canvasVMargin);
}

void ChartBaseView::setCanvasMargins( QSize s )
{
    _canvasHMargin = s.width();
    _canvasVMargin = s.height();
}

uint ChartBaseView::barBorder()const
{
    return _barBorder;
}

void ChartBaseView::setBarBorder(uint value)
{
    _barBorder=value;
}

QSize ChartBaseView::valueSize()const
{
    QString s = QString::number(maximumValue());
    int m = QFontMetrics(font()).width(s);
    int h = QFontMetrics(font()).height();
    return QSize(m+4, h+4);
}

#include "chartbaseview.moc"
