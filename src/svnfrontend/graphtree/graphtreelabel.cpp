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
#include "graphtreelabel.h"
#include "graphtree_defines.h"
#include <qpainter.h>

#if 0
GraphTreeLabel::GraphTreeLabel(const QString&name,const QString&action,const svn::LogEntry&entry
    ,const QRect&r,QCanvas*c)
 : QCanvasRectangle(r,c),StoredDrawParams(), RevGraphItem(name,action,entry)
{
    setText(0,name);
    setPosition(0, DrawParams::BottomCenter);
}
#endif

GraphTreeLabel::GraphTreeLabel(const QString&text, const QString&_nodename,const QRect&r,QCanvas*c)
    : QCanvasRectangle(r,c),StoredDrawParams(),RevGraphItem()
{
    m_Nodename = _nodename;
    setText(0,text);
    setPosition(0, DrawParams::TopCenter);
    drawFrame(true);
}

GraphTreeLabel::~GraphTreeLabel()
{
}

const QString&GraphTreeLabel::nodename()const
{
    return m_Nodename;
}

int GraphTreeLabel::rtti()const
{
    return GRAPHTREE_LABEL;
}

void GraphTreeLabel::setBgColor(const QColor&c)
{
    _backColor=c;
}

void GraphTreeLabel::drawShape(QPainter& p)
{
    QRect r = rect();
/*
    p.setPen(blue);
    p.drawRect(r);
*/
    RectDrawing d(r);
    d.drawBack(&p,this);
    d.drawField(&p, 0, this);
    d.drawField(&p, 1, this);
}

GraphEdge::GraphEdge(QCanvas*c)
    : QCanvasSpline(c)
{
}

GraphEdge::~GraphEdge()
{
}

void GraphEdge::drawShape(QPainter& p)
{
    p.drawPolyline(poly);
}

QPointArray GraphEdge::areaPoints() const
{
  int minX = poly[0].x(), minY = poly[0].y();
  int maxX = minX, maxY = minY;
  int i;

  if (0) qDebug("GraphEdge::areaPoints\n  P 0: %d/%d", minX, minY);
  int len = poly.count();
  for (i=1;i<len;i++) {
    if (poly[i].x() < minX) minX = poly[i].x();
    if (poly[i].y() < minY) minY = poly[i].y();
    if (poly[i].x() > maxX) maxX = poly[i].x();
    if (poly[i].y() > maxY) maxY = poly[i].y();
    if (0) qDebug("  P %d: %d/%d", i, poly[i].x(), poly[i].y());
  }
  QPointArray a = poly.copy(),  b = poly.copy();
  if (minX == maxX) {
    a.translate(-2, 0);
    b.translate(2, 0);
  }
  else {
    a.translate(0, -2);
    b.translate(0, 2);
  }
  a.resize(2*len);
  for (i=0;i<len;i++)
    a[2 * len - 1 -i] = b[i];

  if (0) {
      qDebug(" Result:");
      for (i=0;i<2*len;i++)
      qDebug("  P %d: %d/%d", i, a[i].x(), a[i].y());
  }

  return a;

}

int GraphEdge::rtti()const
{
    return GRAPHTREE_LINE;
}

GraphEdgeArrow::GraphEdgeArrow(GraphEdge*_parent,QCanvas*c)
    : QCanvasPolygon(c),_edge(_parent)
{
}

void GraphEdgeArrow::drawShape(QPainter&p)
{
    QCanvasPolygon::drawShape(p);
}

int GraphEdgeArrow::rtti()const
{
    return GRAPHTREE_ARROW;
}

GraphEdge*GraphEdgeArrow::edge()
{
    return _edge;
}
