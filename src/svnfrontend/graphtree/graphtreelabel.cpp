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

GraphTreeLabel::GraphTreeLabel(const QString&name,const QString&action,const svn::LogEntry&entry
    ,const QRect&r,QCanvas*c)
 : QCanvasRectangle(r,c),StoredDrawParams(), RevGraphItem(name,action,entry)
{
    setText(0,name);
    setPosition(0, DrawParams::BottomCenter);
}

GraphTreeLabel::~GraphTreeLabel()
{
}

int GraphTreeLabel::rtti()const
{
    return GRAPHTREE_LABEL;
}
void GraphTreeLabel::drawShape(QPainter& p)
{
  QRect r = rect();
  p.setPen(blue);
  p.drawRect(r);
  RectDrawing d(r);
  d.drawField(&p, 0, this);
  d.drawField(&p, 1, this);
}
