/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DIRLISTVIEWITEM_H
#define DIRLISTVIEWITEM_H

#include <klistview.h>
#include "svncpp/status.hpp"

class KdeSvnDirList;

/**
@author Rajko Albrecht
*/
class DirListViewItem : public KListViewItem
{
public:
    DirListViewItem(KdeSvnDirList*,const svn::Status&);
    DirListViewItem(KdeSvnDirList*,DirListViewItem*,const svn::Status&);
    virtual ~DirListViewItem();
    const QString& displayName()const;
    const QString& subName()const;

    DirListViewItem* findSubItem(const QString&,DirListViewItem*start);
    bool matchName(const QString&);

    static QString cleanDirname(const QString&);
protected:
    void init();

    QString m_DisplayName;
    QString m_SubName;
    QString m_fullName;
    svn::Status m_Status;
    KdeSvnDirList*m_Parent;
};

#endif
