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
#pragma once

#include <KService>

#include <QMenu>
#include <QUrl>
#include <QVector>

/**
    @author Rajko Albrecht <ral@alwins-world.de>
*/
class OpenContextmenu : public QMenu
{
    Q_OBJECT
public:
    OpenContextmenu(const QUrl &aPath, const KService::List &aList, QWidget *parent);
    ~OpenContextmenu();
protected:
    QUrl m_Path;
    KService::List m_List;
    QVector<KService::Ptr> m_mapPopup;

    void setup();

protected Q_SLOTS:
    void slotOpenWith();
    void slotRunService(QAction *);
};
