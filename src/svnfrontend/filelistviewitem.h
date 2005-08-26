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
#ifndef FILELISTVIEWITEM_H
#define FILELISTVIEWITEM_H

#include "svnitem.h"
#include <klistview.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include "svncpp/status.hpp"

class QPainter;
class KFileItem;
class kdesvnfilelist;
class SvnActions;

/**
@author Rajko Albrecht
*/
class FileListViewItem : public KListViewItem,public SvnItem
{
public:
    FileListViewItem(kdesvnfilelist*,const svn::Status&);
    FileListViewItem(kdesvnfilelist*,FileListViewItem*,const svn::Status&);

    virtual ~FileListViewItem();
    virtual int compare( QListViewItem* i, int col, bool ascending ) const;

    virtual void updateStatus(const svn::Status&s);
    virtual void refreshStatus(bool childs=false,QPtrList<SvnItem> *exclude = 0,bool depsonly=false);

#if 0
    virtual void refreshMe();
#endif

    void removeChilds();
    bool isParent(QListViewItem*which);

    static const int COL_ICON,COL_NAME,COL_LAST_REV,COL_LAST_AUTHOR,COL_LAST_DATE,COL_STATUS/*,COL_CURRENT_REV*/,COL_IS_LOCKED;

    virtual QString getParentDir()const;
    FileListViewItem*fItem(){return this;}
    virtual void setStat(const svn::Status&);

protected:
    short int sortChar;
    kdesvnfilelist*m_Ksvnfilelist;

    virtual void update();

    void makePixmap();
    void init();
    virtual SvnActions*getWrapper();
    SvnActions*m_SvnWrapper;
};

typedef QPtrList<FileListViewItem> FileListViewItemList;
typedef QPtrListIterator<FileListViewItem> FileListViewItemListIterator;

#endif
