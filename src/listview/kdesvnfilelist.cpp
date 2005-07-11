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
#include <kdirlister.h>

kdesvnfilelist::kdesvnfilelist(QWidget *parent, const char *name)
 : KListView(parent, name),m_dirLister(new KDirLister(true))
{
    m_dirLister->setMainWindow(this);
    setMultiSelection(true);
    setSelectionModeExt( FileManager );
    setDragEnabled(true);
    setItemsMovable(false);
    addColumn("Name");
    m_dirLister->setNameFilter("*");
    m_dirLister->clearMimeFilter();

   connect( m_dirLister, SIGNAL(started( const KURL & )),
            this, SLOT(slotStarted()) );
   connect( m_dirLister, SIGNAL(completed()), this, SLOT(slotCompleted()) );
   connect( m_dirLister, SIGNAL(canceled()), this, SLOT(slotCanceled()) );
   connect( m_dirLister, SIGNAL(clear()), this, SLOT(slotClear()) );
   connect( m_dirLister, SIGNAL(newItems( const KFileItemList & ) ),
            this, SLOT(slotNewItems( const KFileItemList & )) );
   connect( m_dirLister, SIGNAL(deleteItem( KFileItem * )),
            this, SLOT(slotDeleteItem( KFileItem * )) );
   connect( m_dirLister, SIGNAL(refreshItems( const KFileItemList & )),
            this, SLOT( slotRefreshItems( const KFileItemList & )) );
   connect( m_dirLister, SIGNAL(redirection( const KURL & )),
            this, SLOT(slotRedirection( const KURL & )) );
#if 0
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
    qDebug("Open url "+url.prettyURL());
    m_dirLister->openURL(url);
    return true;
}

kdesvnfilelist::~kdesvnfilelist()
{
    delete m_dirLister;
}

void kdesvnfilelist::slotStarted()
{
}

void kdesvnfilelist::slotClear()
{
}

void kdesvnfilelist::slotNewItems( const KFileItemList & entries)
{
    qDebug("New items");
    for ( QPtrListIterator<KFileItem> kit ( entries ); kit.current(); ++kit ) {
        new FileListViewItem(this,kit);
    }
}

#include "kdesvnfilelist.moc"
