/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QPixmap>

GraphTreeLabel::GraphTreeLabel(const QString &text, const QString &_nodename, const QRectF &r, QGraphicsItem *p)
    : QGraphicsRectItem(r, p), StoredDrawParams(), m_Nodename(_nodename), m_SourceNode()
{
    m_Nodename = _nodename;
    setText(0, text);
    setPosition(0, DrawParams::TopCenter);
    drawFrame(true);
}

GraphTreeLabel::~GraphTreeLabel()
{
}

const QString &GraphTreeLabel::nodename()const
{
    return m_Nodename;
}

int GraphTreeLabel::type()const
{
    return GRAPHTREE_LABEL;
}

void GraphTreeLabel::setBgColor(const QColor &c)
{
    _backColor = c;
}

void GraphTreeLabel::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option);
    QRect r = rect().toRect();

    RectDrawing d(r);
    d.drawBack(p, this);
    d.drawField(p, 0, this);
    d.drawField(p, 1, this);
}

void GraphTreeLabel::setSelected(bool s)
{
    QGraphicsItem::setSelected(s);
    StoredDrawParams::setSelected(s);
    update();
}

const QString &GraphTreeLabel::source()const
{
    return m_SourceNode;
}

void GraphTreeLabel::setSource(const QString &_s)
{
    m_SourceNode = _s;
}

GraphEdge::GraphEdge(QGraphicsItem *c)
    : QGraphicsPathItem(c)
{
}

GraphEdge::~GraphEdge()
{
}

void GraphEdge::paint(QPainter *p, const QStyleOptionGraphicsItem *options, QWidget *)
{
    Q_UNUSED(options);
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    QPen pen = QPen(Qt::black);
    pen.setWidthF(1.0);
    p->setPen(pen);
    p->drawPath(path());
    p->restore();
}

const QPolygonF &GraphEdge::controlPoints()const
{
    return _points;
}

void GraphEdge::setControlPoints(const QPolygonF &pa)
{
    _points = pa;

    QPainterPath path;
    path.moveTo(pa[0]);
    for (int i = 1; i < pa.size(); i += 3) {
        path.cubicTo(pa[i], pa[(i + 1) % pa.size()], pa[(i + 2) % pa.size()]);
    }

    setPath(path);
}

int GraphEdge::type()const
{
    return GRAPHTREE_LINE;
}

GraphEdgeArrow::GraphEdgeArrow(GraphEdge *_parent, QGraphicsItem *p)
    : QGraphicsPolygonItem(p), _edge(_parent)
{
}

void GraphEdgeArrow::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(Qt::black);
    p->drawPolygon(polygon(), Qt::OddEvenFill);
    p->restore();
}

int GraphEdgeArrow::type()const
{
    return GRAPHTREE_ARROW;
}

GraphEdge *GraphEdgeArrow::edge()
{
    return _edge;
}

/* taken from KCacheGrind project */
QPixmap *GraphMark::_p = nullptr;

GraphMark::GraphMark(GraphTreeLabel *n, QGraphicsItem *p)
    : QGraphicsRectItem(p)
{
    if (!_p) {

        int d = 5;
        float v1 = 130.0f, v2 = 10.0f, v = v1, f = 1.03f;

        // calculate pix size
        QRect r(0, 0, 30, 30);
        while (v > v2) {
            r.setRect(r.x() - d, r.y() - d, r.width() + 2 * d, r.height() + 2 * d);
            v /= f;
        }

        _p = new QPixmap(r.size());
        _p->fill(Qt::white);
        QPainter p(_p);
        p.setPen(Qt::NoPen);

        r.translate(-r.x(), -r.y());

        while (v < v1) {
            v *= f;
            p.setBrush(QColor(265 - (int)v, 265 - (int)v, 265 - (int)v));

            p.drawRect(QRect(r.x(), r.y(), r.width(), d));
            p.drawRect(QRect(r.x(), r.bottom() - d, r.width(), d));
            p.drawRect(QRect(r.x(), r.y() + d, d, r.height() - 2 * d));
            p.drawRect(QRect(r.right() - d, r.y() + d, d, r.height() - 2 * d));

            r.setRect(r.x() + d, r.y() + d, r.width() - 2 * d, r.height() - 2 * d);
        }
    }

    setRect(QRectF(n->rect().center().x() - _p->width() / 2,
                   n->rect().center().y() - _p->height() / 2, _p->width(), _p->height()));
}

GraphMark::~ GraphMark()
{
}

bool GraphMark::hit(const QPoint &)const
{
    return false;
}

int GraphMark::type()const
{
    return GRAPHTREE_MARK;
}

void GraphMark::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (option->levelOfDetail < .5) {
        QRadialGradient g(rect().center(), rect().width() / 3);
        g.setColorAt(0.0, Qt::gray);
        g.setColorAt(1.0, Qt::white);

        p->setBrush(QBrush(g));
        p->setPen(Qt::NoPen);
        p->drawRect(rect());
        return;
    }

    p->drawPixmap(int(rect().x()), int(rect().y()), *_p);
}
