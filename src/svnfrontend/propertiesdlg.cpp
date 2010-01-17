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
#include "src/svnfrontend/fronthelpers/propertyitem.h"
#include "src/svnfrontend/fronthelpers/propertylist.h"
#include "fronthelpers/createdlg.h"
#include "editproperty_impl.h"
#include "svnitem.h"
#include "src/svnqt/client.h"

#include <qvariant.h>
#include <qlabel.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>

/*
 *  Constructs a PropertiesDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PropertiesDlg::PropertiesDlg(SvnItem*which, svn::Client*aClient, const svn::Revision&aRev, QWidget* parent, const char* name, bool modal)
    : KDialog(parent),
      m_Item(which),m_changed(false),
      m_Client(aClient),m_Rev(aRev)
{
    setModal(modal);
    setButtons(KDialog::Ok|KDialog::Cancel);
    setCaption(i18n("Modify properties"));
    if ( !name ) {
        setObjectName( "PropertiesDlg" );
    } else {
        setObjectName(name);
    }
    QWidget * m = new QWidget(this);
    setMainWidget(m);
    PropertiesDlgLayout = new QHBoxLayout(m);

    m_PropertiesListview = new Propertylist(m, "m_PropertiesListview" );
    m_PropertiesListview->setAllColumnsShowFocus(true);
    m_PropertiesListview->setCommitchanges(false);
    PropertiesDlgLayout->addWidget( m_PropertiesListview);

    m_rightLayout = new QVBoxLayout();
    m_AddButton = new KPushButton(m);
    m_AddButton->setObjectName("m_AddButton");
    m_rightLayout->addWidget(m_AddButton);
    m_ModifyButton = new KPushButton(m);
    m_ModifyButton->setObjectName("m_ModifyButton" );
    m_rightLayout->addWidget(m_ModifyButton);
    m_DeleteButton = new KPushButton(m);
    m_DeleteButton->setObjectName("m_DeleteButton");
    m_rightLayout->addWidget( m_DeleteButton );
    m_rightSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_rightLayout->addItem(m_rightSpacer);
    PropertiesDlgLayout->addLayout(m_rightLayout);
    m_DeleteButton->setEnabled(false);
    m_ModifyButton->setEnabled(false);

    //PropertiesDlgLayout->addLayout(midLayout);
    languageChange();

    // signals and slots connections
    connect( m_AddButton, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect( m_ModifyButton, SIGNAL(clicked()), this, SLOT(slotModify()));
    connect( m_DeleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
    connect(this,SIGNAL(helpClicked()),SLOT(slotHelp()));
    connect(m_PropertiesListview,
        SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
        this,
        SLOT(slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    if (!m_Client) {
        m_PropertiesListview->setEnabled(false);
    }
}

bool PropertiesDlg::hasChanged()const
{
    return m_changed;
}

/*
 *  Destroys the object and frees any allocated resources
 */
PropertiesDlg::~PropertiesDlg()
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PropertiesDlg::languageChange()
{
    setCaption( i18n("View and modify properties") );
    m_PropertiesListview->setToolTip(i18n( "List of properties set"));
    m_AddButton->setText(i18n("Add property"));
    m_ModifyButton->setText(i18n("Modify property"));
    m_DeleteButton->setText(i18n("Delete property"));
}

void PropertiesDlg::slotHelp()
{
    qWarning( "PropertiesDlg::slotHelp(): Not implemented yet" );
}

void PropertiesDlg::slotCurrentItemChanged(QTreeWidgetItem*item,QTreeWidgetItem*)
{
    m_DeleteButton->setEnabled(item);
    m_ModifyButton->setEnabled(item);
    if (!item || item->type()!=PropertyListViewItem::_RTTI_) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (item);
    if (PropertyListViewItem::protected_Property(ki->currentName())) {
        m_DeleteButton->setEnabled(false);
        m_ModifyButton->setEnabled(false);
        return;
    }
    if (ki->deleted()) {
        m_DeleteButton->setText(i18n("Undelete property"));
    } else {
        m_DeleteButton->setText(i18n("Delete property"));
    }
}



/*!
    \fn PropertiesDlg::initItem
 */
void PropertiesDlg::initItem()
{
    QString ex;
    if (!m_Client) {
        ex = i18n("Missing SVN link");
        emit clientException(ex);
        return;
    }
    svn::Path what(m_Item->fullName());
    svn::PathPropertiesMapListPtr propList;
    try {
        propList = m_Client->proplist(what,m_Rev,m_Rev);
    } catch (const svn::ClientException&e) {
        emit clientException(e.msg());
        return;
    }
    m_PropertiesListview->displayList(propList,true,m_Item->isDir(),m_Item->fullName());
    initDone = true;
}

/*!
    \fn PropertiesDlg::exec()
 */
int PropertiesDlg::exec()
{
    return KDialog::exec();
}

bool PropertiesDlg::event (QEvent * event)
{
    bool res = KDialog::event(event);
    if (event->type()==QEvent::Polish) {
        initItem();
    }
    return res;
}

/*!
    \fn PropertiesDlg::slotSelectionExecuted(QTreeWidgetItem*)
 */
void PropertiesDlg::slotSelectionExecuted(QTreeWidgetItem*)
{
}

void PropertiesDlg::slotAdd()
{
    EditProperty_impl*ptr = 0L;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&ptr,QString(i18n("Modify property")),true,"modify_properties");
    if (!dlg) {
        return;
    }
    ptr->setDir(m_Item->isDir());

    if (dlg->exec()==QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(ptr->propName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (m_PropertiesListview->checkExisting(ptr->propName())) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        PropertyListViewItem * ki = new PropertyListViewItem(m_PropertiesListview);
        ki->setText(0,ptr->propName());
        ki->setText(1,ptr->propValue());
        ki->checkName();
        ki->checkValue();
    }
}

/*!
    \fn PropertiesDlg::slotDelete
 */
void PropertiesDlg::slotDelete()
{
    QTreeWidgetItem*qi = m_PropertiesListview->currentItem();
    if (!qi) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) return;
    if (ki->deleted()) {
        ki->unDeleteIt();
    } else {
        ki->deleteIt();
    }
    m_PropertiesListview->setCurrentItem(qi);
}


/*!
    \fn PropertiesDlg::slotModify()
 */
void PropertiesDlg::slotModify()
{
    QTreeWidgetItem*qi = m_PropertiesListview->currentItem();
    if (!qi) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) return;
    EditProperty_impl*ptr = 0L;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&ptr,QString(i18n("Modify property")),true,"modify_properties");
    if (!dlg) {
        return;
    }
    ptr->setDir(m_Item->isDir());
    ptr->setPropName(ki->currentName());
    ptr->setPropValue(ki->currentValue());

    if (dlg->exec()==QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(ptr->propName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (m_PropertiesListview->checkExisting(ptr->propName(),qi)) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        ki->setText(0,ptr->propName());
        ki->setText(1,ptr->propValue());
        ki->checkName();
        ki->checkValue();
    }
}

void PropertiesDlg::changedItems(svn::PropertiesMap&toSet,QStringList&toDelete)
{
    toSet.clear();
    toDelete.clear();
    QTreeWidgetItemIterator iter( m_PropertiesListview );
    PropertyListViewItem*ki;
    while ( *iter ) {
        ki = static_cast<PropertyListViewItem*> ( (*iter));
        ++iter;
        if (PropertyListViewItem::protected_Property(ki->currentName())||
            PropertyListViewItem::protected_Property(ki->startName())) {
            continue;
        }
        if (ki->deleted()) {
            toDelete.push_back(ki->currentName());
        } else if (ki->currentName()!=ki->startName()){
            toDelete.push_back(ki->startName());
            toSet[ki->currentName()]=ki->currentValue();
        } else if (ki->currentValue()!=ki->startValue()) {
            toSet[ki->currentName()]=ki->currentValue();
        }
    }
}

#include "propertiesdlg.moc"
