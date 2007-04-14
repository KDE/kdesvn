/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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

#include <qcanvas.h>

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class PannerView : public QCanvasView
{
Q_OBJECT
public:
    PannerView(QWidget* parent=0, const char* name=0);
    virtual ~PannerView();

    void setZoomRect(const QRect& theValue);
    void updateCurrentRect();

signals:
  void zoomRectMoved(int dx, int dy);
  void zoomRectMoveFinished();

protected:
    virtual void drawContents(QPainter* p,  int clipx, int clipy, int clipw, int cliph);
    virtual void contentsMouseMoveEvent(QMouseEvent* e);
    virtual void contentsMousePressEvent(QMouseEvent* e);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);
protected:
    QRect m_ZoomRect;
    bool m_Moving;
    QPoint m_LastPos;
};

#endif
