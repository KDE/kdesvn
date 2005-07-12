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
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "svncpp/revision.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"
#include <kdirlister.h>
#include <klocale.h>

kdesvnfilelist::kdesvnfilelist(QWidget *parent, const char *name)
 : KListView(parent, name),m_Svnclient()
{
    setMultiSelection(true);
    setSelectionMode(QListView::Multi);
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setDragEnabled(true);
    setItemsMovable(false);
    addColumn(i18n("Name"));
    addColumn(i18n("Status"));
    addColumn(i18n("Revision"));
    addColumn(i18n("Author"));
    addColumn(i18n("Date"));
    setSortColumn(FileListViewItem::COL_NAME);
}

bool kdesvnfilelist::openURL( const KURL &url )
{
    clear();
    m_LastException="";
    svn::Context*nc = new svn::Context();
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
    m_Svnclient.setContext(nc);
    svn::Revision rev(svn::Revision::HEAD);

    svn::StatusEntries dlist;
    m_directoryList.clear();

    QString what = url.url();
    if (url.isLocalFile()) {
        what = url.path();
    }
    try {
        /* settings about unknown and ignored files must be setable */
        dlist = m_Svnclient.status(what.ascii());//,false,true,true);
    } catch (svn::ClientException e) {
        //Message box!
        m_LastException = e.message();
        return false;
    } catch (...) {
        qDebug("Other exception");
        return false;
    }
    KFileItemList fk;
    svn::StatusEntries::iterator it = dlist.begin();

    for (;it!=dlist.end();++it) {
        if (it->path()==url.path()){
            m_mainEntry = *it;
            continue;
        }
        FileListViewItem * item = new FileListViewItem(this,*it);
        if (item->isDir()) {
            m_directoryList.push_back(*it);
        }
    }
    return true;
}

kdesvnfilelist::~kdesvnfilelist()
{
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
}

#include "kdesvnfilelist.moc"
