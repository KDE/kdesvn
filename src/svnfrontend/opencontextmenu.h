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
#ifndef OPENCONTEXTMENU_H
#define OPENCONTEXTMENU_H

#include <kservice.h>
#include <kaction.h>
#include <kurl.h>
#include <kmenu.h>

#include <QMap>

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class OpenContextmenu : public KMenu
{
Q_OBJECT
public:
    OpenContextmenu(const KUrl&,const KService::List&,QWidget* parent, const char* name);
    virtual ~OpenContextmenu();
protected:
    KUrl m_Path;
    KService::List m_List;
    QMap<int,KService::Ptr> m_mapPopup;

    void setup();

protected slots:
    virtual void slotOpenWith();
    virtual void slotRunService(QAction*);
};

#endif
