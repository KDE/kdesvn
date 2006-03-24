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
#include "revgraphview.h"
#include "graphtreelabel.h"
#include <kapp.h>
#include <kdebug.h>

RevGraphView::RevGraphView(QWidget * parent, const char * name, WFlags f)
 : QCanvasView(parent,name,f)
{
    m_Canvas = 0L;
}

RevGraphView::~RevGraphView()
{
    setCanvas(0);
    delete m_Canvas;
}

void RevGraphView::addLabel(int row,const QString&path,const QString&action,const svn::LogEntry&e)
{
    if (!m_Canvas) return;
    /* dummy for test*/
    QRect r(10,100*row,360,100);
    GraphTreeLabel*t=new GraphTreeLabel(path,action,e,r,m_Canvas);
    t->show();
    kdDebug()<<"addLabel "<<path << endl;
    m_Canvas->update();
    viewport()->setUpdatesEnabled(true);
}

void RevGraphView::showText(const QString&s)
{
    clear();
    m_Canvas = new QCanvas(QApplication::desktop()->width(),
                        QApplication::desktop()->height());

    QCanvasText* t = new QCanvasText(s, m_Canvas);
    t->move(5, 5);
    t->show();
    center(0,0);
    setCanvas(m_Canvas);
    m_Canvas->update();
}

void RevGraphView::clear()
{
    if (!m_Canvas) return;
    delete m_Canvas;
    m_Canvas = 0;
    setCanvas(0);
}

#include "revgraphview.moc"
