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

#ifndef __ITEMDISPLAY_H
#define __ITEMDISPLAY_H

#include <qptrlist.h>
#include <qstring.h>
#include <kurl.h>

class FileListViewItem;

class ItemDisplay
{
public:
    ItemDisplay(){}
    virtual ~ItemDisplay(){}

    virtual QWidget*realWidget() = 0;
    virtual FileListViewItem*singleSelected()=0;
    virtual bool isLocal()const = 0;
    virtual QPtrList<FileListViewItem> * allSelected()=0;
    virtual const QString&baseUri()const=0;
    virtual bool openURL( const KURL &url,bool noReinit=false )=0;
    virtual FileListViewItem*singleSelectedOrMain()=0;
};

#endif
