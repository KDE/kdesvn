/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#include "propertylist.h"
#include "src/svnfrontend/fronthelpers/propertyitem.h"
#include <klocale.h>
#include <kdebug.h>


Propertylist::Propertylist(QWidget *parent, const char *name)
 : KListView(parent, name)
{
    addColumn(i18n("Property"));
    addColumn(i18n("Value"));
    setSelectionModeExt(FileManager);
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setRootIsDecorated(false);
    setSortColumn(0);
    setAcceptDrops(false);
    //setFullWidth( TRUE );
}


Propertylist::~Propertylist()
{
}

void Propertylist::displayList(const svn::PathPropertiesMapListPtr&propList)
{
    clear();
    if (propList) {
        svn::PathPropertiesMapList::const_iterator lit;
        svn::PropertiesMap pmap;
        for (lit=propList->begin();lit!=propList->end();++lit) {
            pmap = (*lit).second;
            /* just want the first one */
            break;
        }
        svn::PropertiesMap::const_iterator pit;
        for (pit=pmap.begin();pit!=pmap.end();++pit) {
            PropertyListViewItem * ki = new PropertyListViewItem(this,
                    pit.key(),
                    pit.data());
            ki->setMultiLinesEnabled(true);
        }
    }
}

void Propertylist::clear()
{
    KListView::clear();
}

#include "propertylist.moc"
