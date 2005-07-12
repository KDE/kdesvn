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
#include "kdesvndirlist.h"
#include "dirlistviewitem.h"
#include <klocale.h>
#include <qfileinfo.h>

KdeSvnDirList::KdeSvnDirList(QWidget * parent, const char*name)
  : KListView(parent,name)
{
    addColumn(I18N_NOOP("Directory"));
    setRootIsDecorated(true);
    setMultiSelection(false);
}


KdeSvnDirList::~KdeSvnDirList()
{
}

void KdeSvnDirList::setCurrentUrl(const QString&_url)
{
    m_currentURL=DirListViewItem::cleanDirname(_url);
    clear();
    qDebug("Setting current url %s",m_currentURL.ascii());

}

const QString&KdeSvnDirList::currentUrl()const
{
    return m_currentURL;
}

void KdeSvnDirList::appendEntries(const svn::StatusEntries&_list)
{
    svn::StatusEntries::const_iterator it = _list.begin();

    DirListViewItem*pitem;

    for (;it!=_list.end();++it) {
        pitem = (DirListViewItem*)firstChild();
        if (!pitem) {
            new DirListViewItem(this,*it);
            continue;
        }
        QFileInfo f(DirListViewItem::cleanDirname(it->path()));
        QString p = f.dirPath();
        qDebug("Path = %s",p.ascii());
        /* we just have one toplevel element! (the URI) */
        if (pitem->matchName(p)) {
            new DirListViewItem(this,pitem,*it);
            pitem->setOpen(true);
            continue;
        }
        pitem = pitem->findSubItem(p,0);
        if (pitem) {
            new DirListViewItem(this,pitem,*it);

        } else {
            qDebug("Errrorrrrrr");
        }
    }
}

#include "kdesvndirlist.moc"
