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
#include "pannerview.h"
#include "graphtree_defines.h"

#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QStyleOptionGraphicsItem>

class GraphPanMark: public QGraphicsRectItem
{
public:
    GraphPanMark(QGraphicsItem *p = nullptr);
    ~GraphPanMark();
    int type()const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
};

GraphPanMark::GraphPanMark(QGraphicsItem *p)
    : QGraphicsRectItem(p)
{
    setZValue(1.9);
    setPen(QPen(Qt::red));
    QPen pe = pen();
    pe.setWidthF(0.0);
    pe.setStyle(Qt::DashDotLine);
    setPen(pe);
}

GraphPanMark::~GraphPanMark()
{
}

void GraphPanMark::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    if (option->levelOfDetail < .5) {
        QGraphicsRectItem::paint(p, option, w);
    }
}

int GraphPanMark::type()const
{
    return GRAPHTREE_PANMARK;
}

PannerView::PannerView(QWidget *parent)
    : QGraphicsView(parent)// KDE4 check , /*Qt::WNoAutoErase |*/ Qt::WA_StaticContents/*WStaticContents*/ )
{
    m_Mark = nullptr;
    m_Moving = false;
    viewport()->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::NoFocus);
}

PannerView::~PannerView()
{
    if (scene() && m_Mark) {
        scene()->removeItem(m_Mark);
        delete m_Mark;
    }
}

void PannerView::setScene(QGraphicsScene *sc)
{
    if (!sc) {
        if (scene()) {
            scene()->removeItem(m_Mark);
        }
    } else {
        if (!m_Mark) {
            m_Mark = new GraphPanMark;
        }
        sc->addItem(m_Mark);
    }
    QGraphicsView::setScene(sc);
}

void PannerView::setZoomRect(const QRectF &theValue)
{
    m_ZoomRect = theValue;
    if (m_Mark) {
        m_Mark->setRect(m_ZoomRect);
    }
}

/*!
    \fn PannerView::contentsMouseMoveEvent(QMouseEvent* e)
 */
void PannerView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_Moving) {
        QPointF sPos = mapToScene(e->pos());
        emit zoomRectMoved(sPos.x() - m_ZoomRect.center().x(),
                           sPos.y() - m_ZoomRect.center().y());

        m_LastPos = e->pos();
    }
}

/*!
    \fn PannerView::contentsMousePressEvent(QMouseEvent* e)
 */
void PannerView::mousePressEvent(QMouseEvent *e)
{
    if (m_ZoomRect.isValid()) {
        QPointF sPos = mapToScene(e->pos());
        if (!m_ZoomRect.contains(sPos)) {
            emit zoomRectMoved(sPos.x() - m_ZoomRect.center().x(),
                               sPos.y() - m_ZoomRect.center().y());
        }
        m_Moving = true;
        m_LastPos = e->pos();
    }
}

/*!
    \fn PannerView::contentsMouseReleaseEvent(QMouseEvent*)
 */
void PannerView::mouseReleaseEvent(QMouseEvent *)
{
    m_Moving = false;
    emit zoomRectMoveFinished();
}

/*!
    \fn PannerView::updateCurrentRect()
 */
void PannerView::updateCurrentRect()
{
    if (m_ZoomRect.isValid()) {
    }
}
