/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef SVNTREEVIEW_H
#define SVNTREEVIEW_H

#include <KUrl>
#include <QTreeView>

class SvnTreeView: public QTreeView
{
    Q_OBJECT
private:
    static bool _isDrag;
public:
    explicit SvnTreeView(QWidget *parent = 0);
    virtual ~SvnTreeView();

protected:
    virtual void startDrag(Qt::DropActions supportedActions);
    virtual void dropEvent(QDropEvent *event);

protected Q_SLOTS:
    virtual void doDrop(const KUrl::List &, const QModelIndex &, bool, Qt::DropAction, Qt::KeyboardModifiers);
};

#endif
