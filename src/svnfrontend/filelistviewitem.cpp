/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "src/settings/kdesvnsettings.h"
#include "helpers/sub2qt.h"
#include "src/svnqt/status.hpp"
#include "src/svnqt/revision.hpp"
#include "src/svnqt/exception.hpp"
#include "src/svnqt/url.hpp"
#include "fronthelpers/widgetblockstack.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kconfig.h>

#include <qfileinfo.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>

const int FileListViewItem::COL_ICON = 0;
const int FileListViewItem::COL_NAME = 0;
const int FileListViewItem::COL_STATUS = 1;
const int FileListViewItem::COL_LAST_REV = 2;
const int FileListViewItem::COL_LAST_AUTHOR = 3;
const int FileListViewItem::COL_LAST_DATE = 4;
const int FileListViewItem::COL_IS_LOCKED = 5;

//const int FileListViewItem::COL_CURRENT_REV = 5;

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,const svn::StatusPtr&_stat)
 : K3ListViewItem(_parent),SvnItem(_stat),
 sortChar(0),
 m_Ksvnfilelist(_parent)
{
    m_SvnWrapper = _parent->m_SvnWrapper;
    init();
}

FileListViewItem::FileListViewItem(kdesvnfilelist*_parent,FileListViewItem*_parentItem,const svn::StatusPtr&_stat)
    : K3ListViewItem(_parentItem),SvnItem(_stat),
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
    setExpandable(isDir());
    if (shortName()[0]=='.') --sortChar;
    update();
}

void FileListViewItem::setOpen(bool o)
{
    if (o && childCount()==0) {
        {
            WidgetBlockStack a(m_Ksvnfilelist);
            m_Ksvnfilelist->slotItemRead(this);
        }
        m_Ksvnfilelist->setFocus();
    }
    K3ListViewItem::setOpen(o);
}

void FileListViewItem::setOpenNoBlock(bool o)
{
    if (o && childCount()==0) {
        {
            m_Ksvnfilelist->slotItemRead(this);
        }
    }
    KListViewItem::setOpen(o);
}

FileListViewItem::~FileListViewItem()
{
}

void FileListViewItem::setStat(const svn::StatusPtr&stat)
{
    SvnItem::setStat(stat);
    init();
}

void FileListViewItem::refreshStatus(bool childs,Q3PtrList<SvnItem>*exclude,bool depsonly)
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
    int size = Kdesvnsettings::listview_icon_size();
    bool overlay = Kdesvnsettings::display_overlays();
    QPixmap pm;
    if (m_Pixmap.isNull()) {
        pm = getPixmap(size,overlay);
    } else {
        pm = getPixmap(m_Pixmap,size,overlay);
    }
    setPixmap(COL_ICON,pm);
}

void FileListViewItem::setPreviewPix(const QPixmap& pixmap)
{
    if (pixmap.isNull()) return;
    m_Pixmap = pixmap;
    int size = Kdesvnsettings::listview_icon_size();
    bool overlay = Kdesvnsettings::display_overlays();
    QPixmap pm = getPixmap(pixmap,size,overlay);
    setPixmap(COL_ICON,pm);
}

bool FileListViewItem::isParent(Q3ListViewItem*which)
{
    if (!which) return false;
    Q3ListViewItem*item = this;
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
    setText(COL_IS_LOCKED,lockOwner());
}

int FileListViewItem::compare( Q3ListViewItem* item, int col, bool ascending ) const
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

    if (Kdesvnsettings::case_sensitive_sort()) {
        if (Kdesvnsettings::locale_is_casesensitive()) {
            return text(col).localeAwareCompare(k->text(col));
        }
        return text(col).compare(k->text(col));
    } else {
        return text(col).lower().localeAwareCompare(k->text(col).lower());
    }
}

void FileListViewItem::removeChilds()
{
    Q3ListViewItem*temp;
    while ((temp=firstChild())) {
        delete temp;
    }
}

void FileListViewItem::updateStatus(const svn::StatusPtr&s)
{
    setStat(s);
}

SvnItem* FileListViewItem::getParentItem()const
{
    return static_cast<FileListViewItem*>(parent());
}
/*!
    \fn FileListViewItem::getParentDir()const
 */
 QString FileListViewItem::getParentDir()const
{
    SvnItem*temp = getParentItem();
    if (!temp) return QString::null;
    return temp->fullName();
}

void FileListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
    bool colors = Kdesvnsettings::colored_state();
    if (!colors||m_bgColor==NONE) {
        K3ListViewItem::paintCell(p,cg,column,width,alignment);
        return;
    }
    QColorGroup _cg = cg;
    QColor _bgColor;
    switch(m_bgColor) {
        case UPDATES:
            _bgColor = Kdesvnsettings::color_need_update();
            break;
        case  LOCKED:
            _bgColor = Kdesvnsettings::color_locked_item();
            break;
        case  ADDED:
            _bgColor = Kdesvnsettings::color_item_added();
            break;
        case  DELETED:
            _bgColor = Kdesvnsettings::color_item_deleted();
            break;
        case  MODIFIED:
            _bgColor = Kdesvnsettings::color_changed_item();
            break;
        case MISSING:
            _bgColor = Kdesvnsettings::color_missed_item();
            break;
        case NOTVERSIONED:
            _bgColor = Kdesvnsettings::color_notversioned_item();
            break;
        case CONFLICT:
            _bgColor = Kdesvnsettings::color_conflicted_item();
            break;
        case NEEDLOCK:
            _bgColor = Kdesvnsettings::color_need_lock();
            break;
        default:
            K3ListViewItem::paintCell(p,cg,column,width,alignment);
            return;
            break;
    }
    const QPixmap *pm = listView()->viewport()->backgroundPixmap();
    if (pm && !pm->isNull()) {
        _cg.setBrush(QColorGroup::Base, QBrush(_bgColor, *pm));
        QPoint o = p->brushOrigin();
        p->setBrushOrigin( o.x()-listView()->contentsX(), o.y()-listView()->contentsY() );
    } else {
        if (listView()->viewport()->backgroundMode()==Qt::FixedColor) {
            _cg.setColor(QColorGroup::Background,_bgColor);
        } else {
            _cg.setColor(QColorGroup::Base,_bgColor);
        }
    }
    Q3ListViewItem::paintCell(p, _cg, column, width, alignment);
}

const svn::Revision&FileListViewItem::correctPeg()const
{
    return m_Ksvnfilelist->remoteRevision();
}

FileListViewItem*FileListViewItem::findChild(const QString&aName)
{
    FileListViewItem*_item = (FileListViewItem*)firstChild();
    while (_item) {
        if (_item->fullName()==aName) {
            return _item;
        }
        _item = (FileListViewItem*)_item->nextSibling();
    }
    return 0L;
}
