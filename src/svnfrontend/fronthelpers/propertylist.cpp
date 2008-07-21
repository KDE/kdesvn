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
#include <kmessagebox.h>


Propertylist::Propertylist(QWidget *parent, const char *name)
    : K3ListView(parent, name),m_commitit(false)
{
    addColumn(i18n("Property"));
    addColumn(i18n("Value"));
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setRootIsDecorated(false);
    setSortColumn(0);
    setAcceptDrops(false);
    connect(this,SIGNAL(itemRenamed(QListViewItem*,const QString&,int)),this,SLOT(slotItemRenamed(QListViewItem*,const QString&,int)));

    connect(this,SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)),this,
            SLOT(slotContextMenuRequested(QListViewItem *, const QPoint &, int)));
    //setFullWidth( TRUE );
}


Propertylist::~Propertylist()
{
}

void Propertylist::displayList(const svn::PathPropertiesMapListPtr&propList,bool editable,const QString&aCur)
{
    viewport()->setUpdatesEnabled(false);
    clear();
    setItemsRenameable(editable);
    setRenameable(0,editable);
    setRenameable(1,editable);
    if (propList) {
        m_current = aCur;
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
        }
    }
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
}

void Propertylist::clear()
{
    K3ListView::clear();
}

/*!
    \fn PropertiesDlg::slotItemRenamed(QListViewItem*item,const QString & str,int col )
 */
void Propertylist::slotItemRenamed(QListViewItem*_item,const QString & text,int col )
{
    if (!_item || _item->rtti()!=PropertyListViewItem::_RTTI_) return;
    PropertyListViewItem*item = static_cast<PropertyListViewItem*> (_item);

    kDebug()<<"Text: "<< text << " in col "<<col << endl;

    if (text.isEmpty()&&col == 0) {
        // fresh added
        if (item->currentName().isEmpty()) {
            delete item;
        } else {
            item->setText(0,item->currentName());
        }
        return;
    }
    if (PropertyListViewItem::protected_Property(item->text(0)) ||
        PropertyListViewItem::protected_Property(item->currentName())) {
        KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
        item->setText(0,item->currentName());
        item->setText(1,item->currentValue());
        return;
    }
    if (checkExisting(item->text(0),item)) {
        KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
        item->setText(0,item->currentName());
        item->setText(1,item->currentValue());
        return;
    }

    if (col==0) {
        item->checkName();
    } else {
        item->checkValue();
    }
    if (commitchanges() && item->different()) {
        svn::PropertiesMap pm;
        QValueList<QString> dels;
        pm[item->currentName()]=item->currentValue();
        if (item->currentName()!=item->startName()){
            dels.push_back(item->startName());
        }
        emit sigSetProperty(pm,dels,m_current);
    }
}

bool Propertylist::checkExisting(const QString&aName,QListViewItem*it)
{
    if (!it) {
        return findItem(aName,0)!=0;
    }
    QListViewItemIterator iter(this);
    while ( iter.current() ) {
        if ( iter.current()==it) {
            ++iter;
            continue;
        }
        if (iter.current()->text(0)==aName) {
            return true;
        }
        ++iter;
    }
    return false;
}

void Propertylist::addCallback(QObject*ob)
{
    if (ob) {
        connect(this,SIGNAL(sigSetProperty(const svn::PropertiesMap&,const QValueList<QString>&,const QString&)),
                ob,SLOT(slotChangeProperties(const svn::PropertiesMap&,const QValueList<QString>&,const QString&)));
    }
}

/*!
    \fn Propertylist::slotContextMenuRequested(QListViewItem *, const QPoint &, int)
 */
void Propertylist::slotContextMenuRequested(QListViewItem *, const QPoint &, int)
{
    /// @todo implement me
}

#include "propertylist.moc"
