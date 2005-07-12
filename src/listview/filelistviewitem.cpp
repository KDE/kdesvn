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

#include <svn_time.h>

const int FileListViewItem::COL_ICON = 0;
const int FileListViewItem::COL_NAME = 0;
const int FileListViewItem::COL_STATUS = 1;
const int FileListViewItem::COL_LAST_REV = 2;
const int FileListViewItem::COL_LAST_AUTHOR = 3;
const int FileListViewItem::COL_LAST_DATE = 4;

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
    m_fullName = stat.path();
    while (m_fullName.endsWith("/")) {
        /* dir name possible */
        m_fullName.truncate(m_fullName.length()-1);
    }
    int p = m_fullName.findRev("/");
    if (p>-1) {
        ++p;
        m_shortName = m_fullName.right(m_fullName.length()-p);
    } else {
        m_shortName = m_fullName;
    }

    setText(COL_NAME,m_shortName);

    sortChar = isDir()?1:3;
    if (m_shortName[0]=='.') --sortChar;
    update();
}

bool FileListViewItem::isDir()const
{
    if (stat.isVersioned()) {
        return stat.entry().kind()==svn_node_dir;
    }
    /* must be a local file */
    QFileInfo f(stat.path());
    return f.isDir();
}

FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::update(KFileItem*_item)
{
    setText(COL_NAME,_item->name());
    setPixmap(COL_ICON,_item->pixmap(16,0));
    sortChar = S_ISDIR( _item->mode() ) ? 1 : 3;
    if ( _item->name()[0] == '.' )
        --sortChar;
    try {
      stat = m_Ksvnfilelist->svnclient()->singleStatus(_item->url().path());
    } catch (svn::ClientException e) {
        setText(COL_STATUS,e.message());
        return;
    } catch (...) {
        qDebug("Execption catched");
        setText(COL_STATUS,"?");
        return;
    }
    update();
}

void FileListViewItem::makePixmap()
{
    QPixmap p;
    if (QString::compare(stat.entry().url(),stat.path())==0) {
        /* remote access */
        p = KMimeType::pixmapForURL(KURL(stat.entry().url()),0,KIcon::Desktop,16);
    } else {
        p = KMimeType::pixmapForURL(stat.path(),0,KIcon::Desktop,16);
    }
    setPixmap(COL_ICON,p);
}

void FileListViewItem::update()
{
    makePixmap();
    if (!stat.isVersioned()) {
        setText(COL_STATUS,"Not versioned");
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
    setText(COL_STATUS,info_text);
    setText(COL_LAST_AUTHOR,stat.entry().cmtAuthor());
    apr_time_t d = stat.entry().cmtDate();
    svn::Pool pool;
    QDateTime t;
    t.setTime_t(d/(1000*1000),Qt::UTC);
    setText(COL_LAST_DATE,t.toString(Qt::LocalDate));
    setText(COL_LAST_REV,QString("%1").arg(stat.entry().cmtRev()));
}

int FileListViewItem::compare( QListViewItem* item, int col, bool ascending ) const
{
    FileListViewItem* k = static_cast<FileListViewItem*>( item );
    if (col==COL_LAST_DATE) {
        int j = k->stat.entry().cmtDate()-stat.entry().cmtDate();
        return j;
    }
    if ( sortChar != k->sortChar ) {
        // Dirs are always first, even when sorting in descending order
        return !ascending ? k->sortChar - sortChar : sortChar - k->sortChar;
    }
    return text(col).localeAwareCompare(k->text(col));
}
