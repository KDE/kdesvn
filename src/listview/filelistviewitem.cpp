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
#include "svncpp/status.hpp"
#include "svncpp/revision.hpp"

#include <qfileinfo.h>
#include <klocale.h>

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,KFileItem*_item)
 : KListViewItem(_parent),
 sortChar(0),
 m_Ksvnfilelist(_parent),
 stat()
{
    m_shortName = QString(_item->name());
    update(_item);
}

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,const svn::Status&_stat)
 : KListViewItem(_parent),
 sortChar(0),
 m_Ksvnfilelist(_parent),
 stat(_stat)
{
    QFileInfo f(stat.path());
    m_shortName = f.fileName ();
    setText(1,m_shortName);
    sortChar = f.isDir()?1:3;
    if (m_shortName[0]=='.') --sortChar;
    update();
}

FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::update(KFileItem*_item)
{
    setText(1,_item->name());
    setPixmap(0,_item->pixmap(16,0));
    sortChar = S_ISDIR( _item->mode() ) ? 1 : 3;
    if ( _item->name()[0] == '.' )
        --sortChar;
    try {
      stat = m_Ksvnfilelist->svnclient()->singleStatus(_item->url().path());
    } catch (svn::ClientException e) {
        setText(2,e.message());
        return;
    } catch (...) {
        qDebug("Execption catched");
        setText(2,"?");
        return;
    }
    update();
}

void FileListViewItem::update()
{

    if (!stat.isVersioned()) {
        setText(2,"Not versioned");
        return;
    }
    QString info_text = "";
    switch(stat.textStatus ()) {
    case svn_wc_status_modified:
        info_text = i18n("Localy modified");
        break;
    case svn_wc_status_added:
        info_text = i18n("Localy added");
        break;
    case svn_wc_status_missing:
        info_text = i18n("Missing");
        break;
    case svn_wc_status_deleted:
        info_text = i18n("Deleted");
        break;
    case svn_wc_status_replaced:
        info_text = i18n("Replaced");
        break;
    case svn_wc_status_ignored:
        info_text = i18n("Ignored");
        break;
    case svn_wc_status_external:
        info_text=i18n("External");
        break;
    default:
        break;
    }
    if (info_text.isEmpty()) {
        switch (stat.propStatus ()) {
        case svn_wc_status_modified:
            info_text = i18n("Property modified");
            break;
        default:
            break;
        }
    }
    setText(2,info_text);
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
