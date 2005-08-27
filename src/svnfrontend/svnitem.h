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

#ifndef __SVN_ITEM_H
#define __SVN_ITEM_H

#include "helpers/smart_pointer.h"
#include "svncpp/status.hpp"
#include <qstring.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qptrlist.h>

class FileListViewItem;
class SvnItem_p;
class SvnActions;

class SvnItem
{
public:
    SvnItem();
    SvnItem(const svn::Status&);
    virtual ~SvnItem();

    virtual const QString&fullName()const;
    virtual const QString&shortName()const;
    virtual const QString&Url()const;
    virtual const QDateTime&fullDate()const;
    virtual bool isDir()const;
    virtual bool isVersioned()const;
    virtual bool isValid()const;
    virtual bool isRealVersioned()const;
    virtual bool isIgnored()const;
    virtual QString infoText()const;
    virtual QString cmtAuthor()const;
    virtual long int cmtRev()const;
    virtual bool isLocked()const;
    virtual QString lockOwner()const;
    virtual QString getParentDir()const=0;
    virtual void refreshStatus(bool childs=false,QPtrList<SvnItem> *exclude = 0,bool depsonly=false)=0;

    QPixmap getPixmap(int size,bool overlay=true);

    FileListViewItem*fItem(){return 0;}
    virtual void setStat(const svn::Status&);

protected:
    smart_pointer<SvnItem_p> p_Item;
    virtual SvnActions*getWrapper()const = 0;

};

typedef QPtrList<SvnItem> SvnItemList;
typedef QPtrListIterator<SvnItem> SvnItemListIterator;

#endif
