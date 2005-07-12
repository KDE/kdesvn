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
#include "dirlistviewitem.h"
#include "kdesvndirlist.h"

DirListViewItem::DirListViewItem(KdeSvnDirList*_parent,const svn::Status&_status)
    : KListViewItem(_parent),m_DisplayName(""),m_SubName(""),m_Status(_status),m_Parent(_parent)
{
    init();
}

DirListViewItem::DirListViewItem(KdeSvnDirList*_parent,DirListViewItem*_parentItem,const svn::Status&_status)
    : KListViewItem(_parentItem),m_DisplayName(""),m_SubName(""),m_Status(_status),m_Parent(_parent)
{
    init();
}

DirListViewItem::~DirListViewItem()
{
}

QString DirListViewItem::cleanDirname(const QString&p)
{
    QString o = p;
    while (o.endsWith("/")) {
        o.truncate(o.length()-1);
    }
    int pos = o.find(":");
    if (pos!=-1) {
        o=o.right(o.length()-(++pos));
    }
    while (o.find("//")!=-1) {
        o.replace("//","/");
    }

    return o;
}

void DirListViewItem::init()
{
    m_fullName = cleanDirname(m_Status.path());
    if (m_fullName.length()==0) {
        m_fullName = cleanDirname(m_Status.entry().url());
    }
    int length = m_Parent->currentUrl().length();

    qDebug("URL: %s - Laenge url: %i - Laenge input: %i",m_Parent->currentUrl().ascii(),length,m_fullName.length());

    m_SubName = m_fullName.right(m_fullName.length()-length);
    if (m_SubName.isEmpty()) {
        m_DisplayName=m_fullName;
    } else {
        int p = m_SubName.findRev("/");
        if (p>-1) {
            ++p;
            m_DisplayName = m_SubName.right(m_SubName.length()-p);
        }
    }
    qDebug("FullName: %s Subname: %s",m_fullName.ascii(),m_SubName.ascii());
    setText(0,m_DisplayName);
}

/*!
    \fn DirListViewItem::displayName()const
 */
const QString& DirListViewItem::displayName()const
{
    return m_DisplayName;
}


/*!
    \fn DirListViewItem::subName()const
 */
const QString& DirListViewItem::subName()const
{
    return m_SubName;
}

bool DirListViewItem::matchName(const QString&path)
{
    if (path==m_fullName) return true;
    return false;
}

DirListViewItem* DirListViewItem::findSubItem(const QString&path,DirListViewItem*start)
{
    DirListViewItem *pitem,*sitem;
    if (!start)pitem = (DirListViewItem*)firstChild();
    else pitem=(DirListViewItem*)start->firstChild();
    while (pitem)
    {
        if (pitem->matchName(path))
        {
            break;
        }
        if (pitem->childCount()>0)
        {
            sitem = findSubItem(path,pitem);
            if (sitem)
            {
                pitem = sitem;
                break;
            }
        }
        pitem=(DirListViewItem*)pitem->nextSibling();
    }
    return pitem;
}
