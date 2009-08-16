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
#include "kmultilinedelegate.h"
#include "src/svnfrontend/fronthelpers/propertyitem.h"

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <QKeyEvent>

Propertylist::Propertylist(QWidget *parent, const char *name)
    : QTreeWidget(parent),m_commitit(false)
{
    setObjectName(name);
    setItemDelegate(new KMultilineDelegate(this));
    m_Dir=false;

    headerItem()->setText(0,i18n("Property"));
    headerItem()->setText(1,i18n("Value"));
    //setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setRootIsDecorated(false);
    sortItems(0,Qt::AscendingOrder);
    setAcceptDrops(false);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

Propertylist::~Propertylist()
{
}

void Propertylist::displayList(const svn::PathPropertiesMapListPtr&propList,bool editable,bool isDir,const QString&aCur)
{
    disconnect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    viewport()->setUpdatesEnabled(false);
    clear();
    m_Dir=isDir;
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
                    pit.value());
            if (editable && !PropertyListViewItem::protected_Property(ki->currentName())) {
                ki->setFlags(ki->flags()|Qt::ItemIsEditable);
            }
        }
    }
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
    connect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void Propertylist::clear()
{
    QTreeWidget::clear();
}

/*!
    \fn PropertiesDlg::slotItemRenamed(QListViewItem*item,const QString & str,int col )
 */
void Propertylist::slotItemChanged(QTreeWidgetItem*_item,int col )
{
    if (!_item || _item->type()!=PropertyListViewItem::_RTTI_) return;
    PropertyListViewItem*item = static_cast<PropertyListViewItem*> (_item);
    QString text = item->text(col);

    if (text.isEmpty()&&col == 0) {
        // fresh added
        if (item->currentName().isEmpty()) {
            delete item;
        } else {
            item->setText(0,item->currentName());
        }
        return;
    }
    bool fail = false;
    disconnect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    if (PropertyListViewItem::protected_Property(item->text(0)) ||
        PropertyListViewItem::protected_Property(item->currentName())) {
        KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
        item->setText(0,item->currentName());
        item->setText(1,item->currentValue());
        fail = true;
    } else if (checkExisting(item->text(0),item)) {
        KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
        item->setText(0,item->currentName());
        item->setText(1,item->currentValue());
        fail = true;
    }
    connect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    if (fail) {
        return;
    }

    if (col==0) {
        item->checkName();
    } else {
        item->checkValue();
    }
    if (commitchanges() && item->different()) {
        svn::PropertiesMap pm;
        QStringList dels;
        pm[item->currentName()]=item->currentValue();
        if (item->currentName()!=item->startName()){
            dels.push_back(item->startName());
        }
        emit sigSetProperty(pm,dels,m_current);
    }
}

bool Propertylist::checkExisting(const QString&aName,QTreeWidgetItem*it)
{
    if (!it) {
        return findItems(aName,Qt::MatchExactly|Qt::MatchRecursive,0).size()!=0;
    }
    QTreeWidgetItemIterator iter(this);
    while (*iter) {
        if ((*iter)==it) {
            ++iter;
            continue;
        }
        if ( (*iter)->text(0)==aName) {
            return true;
        }
        ++iter;
    }
    return false;
}

void Propertylist::addCallback(QObject*ob)
{
    if (ob) {
        connect(this,SIGNAL(sigSetProperty(const svn::PropertiesMap&,const QStringList&,const QString&)),
                ob,SLOT(slotChangeProperties(const svn::PropertiesMap&,const QStringList&,const QString&)));
    }
}

void Propertylist::keyPressEvent(QKeyEvent*key)
{
    if (key->key()==Qt::Key_Return) {
        // kDebug(9510)<<"Return pressed"<<endl;
    }
    key->ignore();
}

#include "propertylist.moc"
