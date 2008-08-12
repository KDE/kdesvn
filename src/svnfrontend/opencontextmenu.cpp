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
#include <kapplication.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include <Q3CString>

OpenContextmenu::OpenContextmenu(const KUrl&aPath,const KService::List&aList,QWidget* parent, const char* name)
    : Q3PopupMenu(parent, name),m_Path(aPath),m_List(aList)
{
    setup();
}

OpenContextmenu::~OpenContextmenu()
{
}

void OpenContextmenu::setup()
{
    m_mapPopup.clear();
    KService::List::ConstIterator it = m_List.begin();
    int id = 1;
    KAction*act;
    for( ; it != m_List.end(); ++it ) {
        if ((*it)->noDisplay())
            continue;

        QString actionName( (*it)->name().replace("&", "&&") );
        act = new KAction(actionName,this);
        QVariant _data=id;
        act->setData(_data);
        addAction(act);
        //post increment!!!!!
        m_mapPopup[ id++ ] = *it;
    }
    connect(this,SIGNAL(triggered(QAction*)),this,SLOT(slotRunService(QAction*)));
    if (m_List.count()>0) {
        insertSeparator( );
    }
    act = new KAction(i18n("Other..."),this);
    QVariant _data=int(0);

    //connect(act,SIGNAL(triggered()),this,SLOT(slotOpenWith()));
    addAction(act);
}

void OpenContextmenu::slotRunService(QAction*act)
{
    QVariant _data = act->data();
    int id = _data.toInt();

    QMap<int,KService::Ptr>::Iterator it = m_mapPopup.find( id );
    if ( it != m_mapPopup.end() )
    {
        KRun::run(**it,m_Path,KApplication::activeWindow());
        return;
    }

}

void OpenContextmenu::slotOpenWith()
{
    KUrl::List lst;
    lst.append(m_Path);
    KRun::displayOpenWithDialog(lst,KApplication::activeWindow());
}

#include "opencontextmenu.moc"
