/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#include "opencontextmenu.h"

#include <krun.h>
#include <klocale.h>

OpenContextmenu::OpenContextmenu(const KURL&aPath,const KTrader::OfferList&aList,QWidget* parent, const char* name)
    : QPopupMenu(parent, name),m_Path(aPath),m_List(aList)
{
    setup();
}

OpenContextmenu::~OpenContextmenu()
{
}

void OpenContextmenu::setup()
{
    m_mapPopup.clear();
    KTrader::OfferList::ConstIterator it = m_List.begin();
    int id = 1;
    KAction*act;
    for( ; it != m_List.end(); ++it ) {
        if ((*it)->noDisplay())
            continue;

        QCString nam;
        nam.setNum( id );

        QString actionName( (*it)->name().replace("&", "&&") );
        act = new KAction( actionName, (*it)->pixmap( KIcon::Small ), 0,
                                    this, SLOT( slotRunService() ), this, nam.prepend( "appservice_" ) );
        act->plug(this);
        m_mapPopup[ id++ ] = *it;
    }
    if (m_List.count()>0) {
        insertSeparator( );
    }
    act = new KAction(i18n("Other..."),0, 0,
        this, SLOT( slotOpenWith() ),this,"openwith");
    act->plug(this);
}

void OpenContextmenu::slotRunService()
{
  QCString senderName = sender()->name();
  int id = senderName.mid( senderName.find( '_' ) + 1 ).toInt();

  QMap<int,KService::Ptr>::Iterator it = m_mapPopup.find( id );
  if ( it != m_mapPopup.end() )
  {
    KRun::run( **it, m_Path );
    return;
  }

}

void OpenContextmenu::slotOpenWith()
{
    KURL::List lst;
    lst.append(m_Path);
    KRun::displayOpenWithDialog(lst);
}

#include "opencontextmenu.moc"
