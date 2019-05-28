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
#ifndef GRAPHTREELABEL_H
#define GRAPHTREELABEL_H

#include "graphtree/drawparams.h"

#include <QGraphicsRectItem>
#include <QPixmap>

/**
    @author Rajko Albrecht <ral@alwins-world.de>
*/
class GraphTreeLabel : public QGraphicsRectItem, StoredDrawParams
{
public:
    GraphTreeLabel(const QString &, const QString &, const QRectF &r, QGraphicsItem *p = nullptr);
    virtual ~GraphTreeLabel();

    int type()const override;
    //virtual void drawShape(QPainter& p);
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    void setBgColor(const QColor &);

    const QString &nodename()const;
    const QString &source()const;
    void setSource(const QString &);
    virtual void setSelected(bool);

protected:
    QString m_Nodename;
    QString m_SourceNode;
};

class GraphEdge;

class GraphEdgeArrow: public QGraphicsPolygonItem
{
public:
    explicit GraphEdgeArrow(GraphEdge *, QGraphicsItem *p = nullptr);
    GraphEdge *edge();
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    int type()const override;

private:
    GraphEdge *_edge;
};

/* line */
class GraphEdge: public QGraphicsPathItem
{
public:
    explicit GraphEdge(QGraphicsItem *p = nullptr);
    virtual ~GraphEdge();

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    const QPolygonF &controlPoints()const;
    void setControlPoints(const QPolygonF &a);
    int type()const override;

private:
    QPolygonF _points;
};

class GraphMark: public QGraphicsRectItem
{
public:
    explicit GraphMark(GraphTreeLabel *, QGraphicsItem *p = nullptr);
    virtual ~GraphMark();
    int type()const override;
    virtual bool hit(const QPoint &)const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    static QPixmap *_p;
};

#endif
