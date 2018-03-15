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
    explicit PannerView(QWidget *parent = nullptr);
    ~PannerView();

    void setZoomRect(const QRectF &theValue);
    void updateCurrentRect();
    virtual void setScene(QGraphicsScene *sc);

signals:
    void zoomRectMoved(qreal dx, qreal dy);
    void zoomRectMoveFinished();

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

protected:
    QRectF m_ZoomRect;
    bool m_Moving;
    QPoint m_LastPos;

    GraphPanMark *m_Mark;
};

#endif
