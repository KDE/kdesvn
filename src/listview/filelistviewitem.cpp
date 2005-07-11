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
#include "filelistviewitem.h"
#include "kdesvnfilelist.h"

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,KFileItem*_item)
 : KListViewItem(_parent),
 m_Item(_item),
 sortChar(0)
{
    update();
}


FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::update()
{
    setText(0,m_Item->name());
    sortChar = S_ISDIR( m_Item->mode() ) ? 1 : 3;
    if ( m_Item->text()[0] == '.' )
        --sortChar;
}

int FileListViewItem::compare( QListViewItem* item, int col, bool ascending ) const
{
    FileListViewItem* k = static_cast<FileListViewItem*>( item );
    if ( sortChar != k->sortChar ) {
        // Dirs are always first, even when sorting in descending order
        return !ascending ? k->sortChar - sortChar : sortChar - k->sortChar;
    }
    return text(col).localeAwareCompare(k->text(col));
}
