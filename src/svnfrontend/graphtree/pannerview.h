/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef PANNERVIEW_H
#define PANNERVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>


class GraphPanMark;
/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class PannerView : public QGraphicsView
{
    Q_OBJECT
public:
    PannerView(QWidget* parent=0, const char* name=0);
    virtual ~PannerView();

    void setZoomRect(const QRectF& theValue);
    void updateCurrentRect();
    virtual void setScene(QGraphicsScene*sc);

signals:
    void zoomRectMoved(qreal dx, qreal dy);
    void zoomRectMoveFinished();

protected:
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

protected:
    QRectF m_ZoomRect;
    bool m_Moving;
    QPoint m_LastPos;

    GraphPanMark*m_Mark;
};

#endif
