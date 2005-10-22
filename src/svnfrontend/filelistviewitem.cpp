/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "filelistviewitem.h"
#include "kdesvnfilelist.h"
#include "fronthelpers/settings.h"
#include "helpers/sub2qt.h"
#include "svncpp/status.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/exception.hpp"
#include "svncpp/url.hpp"

#include <klocale.h>
#include <kiconloader.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kconfig.h>

#include <qfileinfo.h>

const int FileListViewItem::COL_ICON = 0;
const int FileListViewItem::COL_NAME = 0;
const int FileListViewItem::COL_STATUS = 1;
const int FileListViewItem::COL_LAST_REV = 2;
const int FileListViewItem::COL_LAST_AUTHOR = 3;
const int FileListViewItem::COL_LAST_DATE = 4;
const int FileListViewItem::COL_IS_LOCKED = 5;

//const int FileListViewItem::COL_CURRENT_REV = 5;

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,const svn::Status&_stat)
 : KListViewItem(_parent),SvnItem(_stat),
 sortChar(0),
 m_Ksvnfilelist(_parent)
{
    m_SvnWrapper = _parent->m_SvnWrapper;
    init();
}

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,FileListViewItem*_parentItem,const svn::Status&_stat)
    : KListViewItem(_parentItem),SvnItem(_stat),
    sortChar(0),
    m_Ksvnfilelist(_parent)
{
    m_SvnWrapper = _parent->m_SvnWrapper;
    init();
}

SvnActions*FileListViewItem::getWrapper()const
{
    return m_SvnWrapper;
}

void FileListViewItem::init()
{
    setText(COL_NAME,shortName());
    sortChar = isDir()?1:3;
    if (shortName()[0]=='.') --sortChar;
    update();
}

FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::setStat(const svn::Status&stat)
{
    SvnItem::setStat(stat);
    init();
}

void FileListViewItem::refreshStatus(bool childs,QPtrList<SvnItem>*exclude,bool depsonly)
{
    FileListViewItem*it;

    if (!depsonly) {
        if (!m_Ksvnfilelist->refreshItem(this)) {
            return;
        }
    }
    if (!isValid()) {
        return;
    }
    it = static_cast<FileListViewItem*>(parent());
    if (!childs) {
        if (it && (!exclude || exclude->find(it)==-1)) {
            it->refreshStatus(false,exclude);
        }
    } else if (firstChild()){
        it = static_cast<FileListViewItem*>(firstChild());
        while (it) {
            if (!exclude || exclude->find(it)==-1) {
                it->refreshStatus(true,exclude);
            }
            it = static_cast<FileListViewItem*>(it->nextSibling());
        }
    }
    repaint();
}

void FileListViewItem::makePixmap()
{
    int size = Settings::listview_icon_size();
    bool overlay = Settings::display_overlays();
    setPixmap(COL_ICON,getPixmap(size,overlay));
}

bool FileListViewItem::isParent(QListViewItem*which)
{
    if (!which) return false;
    QListViewItem*item = this;
    while ( (item=item->parent())) {
        if (item==which) {
            return true;
        }
    }
    return false;
}

void FileListViewItem::update()
{
    makePixmap();
    if (!isVersioned()) {
        setText(COL_STATUS,i18n("Not versioned"));
        return;
    }
    setText(COL_STATUS,infoText());
    setText(COL_LAST_AUTHOR,cmtAuthor());
    setText(COL_LAST_DATE,KGlobal::locale()->formatDateTime(fullDate()));
    setText(COL_LAST_REV,QString("%1").arg(cmtRev()));
    if (isLocked()) {
        setPixmap(COL_IS_LOCKED,KGlobal::iconLoader()->loadIcon("lock",KIcon::Desktop,16));
    } else {
        setPixmap(COL_IS_LOCKED,QPixmap());
    }
    setText(COL_IS_LOCKED,lockOwner());
}

int FileListViewItem::compare( QListViewItem* item, int col, bool ascending ) const
{
    FileListViewItem* k = static_cast<FileListViewItem*>( item );
    if ( sortChar != k->sortChar ) {
        // Dirs are always first, even when sorting in descending order
        return !ascending ? k->sortChar - sortChar : sortChar - k->sortChar;
    }
    if (col==COL_LAST_DATE) {
        return fullDate().secsTo(k->fullDate());
    }
    if (col==COL_LAST_REV) {
        return k->cmtRev()-cmtRev();
    }
    return text(col).localeAwareCompare(k->text(col));
}

void FileListViewItem::removeChilds()
{
    QListViewItem*temp;
    while ((temp=firstChild())) {
        delete temp;
    }
}

void FileListViewItem::updateStatus(const svn::Status&s)
{
    setStat(s);
}

/*!
    \fn FileListViewItem::getParentDir()const
 */
 QString FileListViewItem::getParentDir()const
{
    FileListViewItem*temp = static_cast<FileListViewItem*>(parent());
    if (!temp) return QString::null;
    return temp->fullName();
}
