/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#include <kdebug.h>

OpenContextmenu::OpenContextmenu(const KUrl&aPath,const KService::List&aList,QWidget* parent, const char* name)
    : KMenu(parent),m_Path(aPath),m_List(aList)
{
    setObjectName(name);
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
    QAction*act;
    QStringList _found;
    for( ; it != m_List.end(); ++it ) {
        if (_found.indexOf((*it)->name())!=-1) {
            continue;
        }
        _found.append((*it)->name());
        QString actionName( (*it)->name().replace('&', "&&") );
        act = addAction(SmallIcon((*it)->icon()),actionName);
        QVariant _data=id;
        act->setData(_data);

        //post increment!!!!!
        m_mapPopup[ id++ ] = *it;
    }
    connect(this,SIGNAL(triggered(QAction*)),this,SLOT(slotRunService(QAction*)));
    if (m_List.count()>0) {
        addSeparator( );
    }
    act = new KAction(i18n("Other..."),this);
    QVariant _data=int(0);
    act->setData(_data);
    addAction(act);
}

void OpenContextmenu::slotRunService(QAction*act)
{
    QMap<int,KService::Ptr>::Iterator it = m_mapPopup.find(act->data().toInt());
    if ( it != m_mapPopup.end() )
    {
        KRun::run(**it,m_Path,KApplication::activeWindow());
    } else {
        slotOpenWith();
    }

}

void OpenContextmenu::slotOpenWith()
{
    KUrl::List lst;
    lst.append(m_Path);
    KRun::displayOpenWithDialog(lst,KApplication::activeWindow());
}

#include "opencontextmenu.moc"
