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

#include "propertiesdlg.h"
#include "ui_propertiesdlg.h"

#include "svnfrontend/fronthelpers/propertyitem.h"
#include "svnfrontend/fronthelpers/propertylist.h"
#include "editpropsdlg.h"
#include "svnitem.h"
#include "svnqt/client.h"

#include <KLocalizedString>
#include <KMessageBox>

PropertiesDlg::PropertiesDlg(SvnItem *which, const svn::ClientP &aClient, const svn::Revision &aRev, QWidget *parent)
    : KSvnDialog(QLatin1String("properties_dlg"), parent)
    , m_Item(which)
    , m_Client(aClient)
    , m_Rev(aRev)
    , m_ui(new Ui::PropertiesDlg)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Ok));
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_ui->buttonBox, SIGNAL(helpRequested()), this, SLOT(slotHelp()));

    m_ui->tvPropertyList->setAllColumnsShowFocus(true);
    m_ui->tvPropertyList->setCommitchanges(false);

    // signals and slots connections
    connect(m_ui->pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(m_ui->pbModify, SIGNAL(clicked()), this, SLOT(slotModify()));
    connect(m_ui->pbDelete, SIGNAL(clicked()), this, SLOT(slotDelete()));

    connect(m_ui->tvPropertyList,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this,
            SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));
    if (!m_Client) {
        m_ui->tvPropertyList->setEnabled(false);
    }
    slotCurrentItemChanged(nullptr);
    initItem();
}

PropertiesDlg::~PropertiesDlg()
{
    delete m_ui;
}

void PropertiesDlg::slotHelp()
{
    qWarning("PropertiesDlg::slotHelp(): Not implemented yet");
}

void PropertiesDlg::slotCurrentItemChanged(QTreeWidgetItem *item)
{
    m_ui->pbDelete->setEnabled(item != nullptr);
    m_ui->pbModify->setEnabled(item != nullptr);
    if (!item || item->type() != PropertyListViewItem::_RTTI_) {
        return;
    }
    PropertyListViewItem *ki = static_cast<PropertyListViewItem *>(item);
    if (PropertyListViewItem::protected_Property(ki->currentName())) {
        m_ui->pbDelete->setEnabled(false);
        m_ui->pbModify->setEnabled(false);
        return;
    }
    if (ki->deleted()) {
        m_ui->pbDelete->setText(i18n("Undelete property"));
    } else {
        m_ui->pbDelete->setText(i18n("Delete property"));
    }
}

void PropertiesDlg::initItem()
{
    if (!m_Client) {
        QString ex = i18n("Missing SVN link");
        emit clientException(ex);
        return;
    }
    svn::Path what(m_Item->fullName());
    svn::PathPropertiesMapListPtr propList;
    try {
        propList = m_Client->proplist(what, m_Rev, m_Rev);
    } catch (const svn::ClientException &e) {
        emit clientException(e.msg());
        return;
    }
    m_ui->tvPropertyList->displayList(propList, true, m_Item->isDir(), m_Item->fullName());
}

void PropertiesDlg::slotAdd()
{
    QPointer<EditPropsDlg> dlg(new EditPropsDlg(true, this));
    dlg->setDir(m_Item->isDir());

    if (dlg->exec() == QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(dlg->propName())) {
            KMessageBox::error(this, i18n("This property may not set by users.\nRejecting it."), i18n("Protected property"));
            return;
        }
        if (m_ui->tvPropertyList->checkExisting(dlg->propName())) {
            KMessageBox::error(this, i18n("A property with that name exists.\nRejecting it."), i18n("Double property"));
            return;
        }
        if (!dlg->propName().isEmpty()) {
            PropertyListViewItem *item = new PropertyListViewItem(m_ui->tvPropertyList);
            item->setName(dlg->propName());
            item->setValue(dlg->propValue());
        }
    }
    delete dlg;
}

void PropertiesDlg::slotDelete()
{
    QTreeWidgetItem *qi = m_ui->tvPropertyList->currentItem();
    if (!qi) {
        return;
    }
    PropertyListViewItem *ki = static_cast<PropertyListViewItem *>(qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) {
        return;
    }
    if (ki->deleted()) {
        ki->unDeleteIt();
    } else {
        ki->deleteIt();
    }
    slotCurrentItemChanged(qi);
}

void PropertiesDlg::slotModify()
{
    QTreeWidgetItem *qi = m_ui->tvPropertyList->currentItem();
    if (!qi) {
        return;
    }
    PropertyListViewItem *ki = static_cast<PropertyListViewItem *>(qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) {
        return;
    }
    QPointer<EditPropsDlg> dlg(new EditPropsDlg(false, this));
    dlg->setDir(m_Item->isDir());
    dlg->setPropName(ki->currentName());
    dlg->setPropValue(ki->currentValue());

    if (dlg->exec() == QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(dlg->propName())) {
            KMessageBox::error(this, i18n("This property may not set by users.\nRejecting it."), i18n("Protected property"));
            return;
        }
        if (m_ui->tvPropertyList->checkExisting(dlg->propName(), qi)) {
            KMessageBox::error(this, i18n("A property with that name exists.\nRejecting it."), i18n("Double property"));
            return;
        }
        ki->setName(dlg->propName());
        ki->setValue(dlg->propValue());
    }
    delete dlg;
}

void PropertiesDlg::changedItems(svn::PropertiesMap &toSet, QStringList &toDelete)
{
    toSet.clear();
    toDelete.clear();
    QTreeWidgetItemIterator iter(m_ui->tvPropertyList);
    while (*iter) {
        PropertyListViewItem *ki = static_cast<PropertyListViewItem *>((*iter));
        ++iter;
        if (PropertyListViewItem::protected_Property(ki->currentName()) ||
                PropertyListViewItem::protected_Property(ki->startName())) {
            continue;
        }
        if (ki->deleted()) {
            toDelete.push_back(ki->currentName());
        } else if (ki->currentName() != ki->startName()) {
            toDelete.push_back(ki->startName());
            toSet[ki->currentName()] = ki->currentValue();
        } else if (ki->currentValue() != ki->startValue()) {
            toSet[ki->currentName()] = ki->currentValue();
        }
    }
}
