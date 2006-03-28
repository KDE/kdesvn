/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#ifndef GRAPHTREELABEL_H
#define GRAPHTREELABEL_H

#include <treemap.h>
#include <revgraphitem.h>
#include <qcanvas.h>

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class GraphTreeLabel : public QCanvasRectangle,StoredDrawParams, public RevGraphItem
{
public:
    GraphTreeLabel(const QString&,const QString&,const QRect&r,QCanvas*c);
    virtual ~GraphTreeLabel();

    virtual int rtti()const;
    virtual void drawShape(QPainter& p);

    void setBgColor(const QColor&);

    const QString&nodename()const;

protected:
    QString m_Nodename;
};


class GraphEdge;

class GraphEdgeArrow:public QCanvasPolygon
{
public:
    GraphEdgeArrow(GraphEdge*,QCanvas*);
    GraphEdge*edge();
    virtual void drawShape(QPainter&);
    int rtti()const;

private:
    GraphEdge*_edge;
};

/* line */
class GraphEdge:public QCanvasSpline
{
public:
    GraphEdge(QCanvas*);
    virtual ~GraphEdge();

    virtual void drawShape(QPainter&);
    QPointArray areaPoints() const;
    virtual int rtti()const;
};

#endif
