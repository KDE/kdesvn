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

kdesvnfilelist::kdesvnfilelist(QWidget *parent, const char *name)
 : KListView(parent, name),m_dirLister(new KDirLister(true)),m_Svnclient()
{
    m_dirLister->setMainWindow(this);
    setMultiSelection(true);
    setSelectionModeExt( FileManager );
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setDragEnabled(true);
    setItemsMovable(false);
    addColumn(" ");
    addColumn("Name");
    addColumn("Status");
    setSortColumn(1);

    m_dirLister->setNameFilter("*");
    m_dirLister->clearMimeFilter();
    m_dirLister->setShowingDotFiles(true);

   connect( m_dirLister, SIGNAL(started( const KURL & )),
            this, SLOT(slotStarted()) );
   //connect( m_dirLister, SIGNAL(completed()), this, SLOT(slotCompleted()) );
   //connect( m_dirLister, SIGNAL(canceled()), this, SLOT(slotCanceled()) );
   connect( m_dirLister, SIGNAL(clear()), this, SLOT(slotClear()) );
   connect( m_dirLister, SIGNAL(newItems( const KFileItemList & ) ),
            this, SLOT(slotNewItems( const KFileItemList & )) );
#if 0
   connect( m_dirLister, SIGNAL(deleteItem( KFileItem * )),
            this, SLOT(slotDeleteItem( KFileItem * )) );
   connect( m_dirLister, SIGNAL(refreshItems( const KFileItemList & )),
            this, SLOT( slotRefreshItems( const KFileItemList & )) );
   connect( m_dirLister, SIGNAL(redirection( const KURL & )),
            this, SLOT(slotRedirection( const KURL & )) );

   connect( m_dirLister, SIGNAL(itemsFilteredByMime( const KFileItemList & )),
            m_pBrowserView, SIGNAL(itemsFilteredByMime( const KFileItemList & )) );
   connect( m_dirLister, SIGNAL(infoMessage( const QString& )),
            m_pBrowserView->extension(), SIGNAL(infoMessage( const QString& )) );
   connect( m_dirLister, SIGNAL(percent( int )),
            m_pBrowserView->extension(), SIGNAL(loadingProgress( int )) );
   connect( m_dirLister, SIGNAL(speed( int )),
            m_pBrowserView->extension(), SIGNAL(speedProgress( int )) );
#endif

}

bool kdesvnfilelist::openURL( const KURL &url )
{
    clear();
    svn::Context*nc = new svn::Context();
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
    m_Svnclient.setContext(nc);
    svn::Revision rev(svn::Revision::HEAD);

    svn::StatusEntries dlist;
    try {
        dlist = m_Svnclient.status(url.path().ascii());
    } catch (svn::ClientException e) {
//        setText(2,e.message());
        return false;
    } catch (...) {
        return false;
    }
    KFileItemList fk;
    svn::StatusEntries::iterator it = dlist.begin();
    for (;it!=dlist.end();++it) {
        if (it->path()==url.path())continue;
        //KFileItem *i=new KFileItem(KFileItem::Unknown,KFileItem::Unknown,KURL(it->path()));
        //fk.append(i);
        new FileListViewItem(this,*it);
    }
    //slotNewItems(fk);
/*
    m_dirLister->openURL(url);
*/
    return true;
}

kdesvnfilelist::~kdesvnfilelist()
{
    delete m_dirLister;
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
}

void kdesvnfilelist::slotStarted()
{
}

void kdesvnfilelist::slotClear()
{
}

void kdesvnfilelist::slotNewItems( const KFileItemList & entries)
{
    for ( QPtrListIterator<KFileItem> kit ( entries ); kit.current(); ++kit ) {
        new FileListViewItem(this,kit);
    }
}

#include "kdesvnfilelist.moc"
