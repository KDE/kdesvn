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

#include "src/svnfrontend/fronthelpers/propertyitem.h"
#include "src/svnfrontend/fronthelpers/propertylist.h"
#include "propertiesdlg.h"
#include "editproperty_impl.h"
#include "svnitem.h"
#include "src/svnqt/client.hpp"

#include <qvariant.h>
#include <qlabel.h>
#include <qheader.h>
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
    :
    KDialogBase(parent,name,modal,i18n("Modify properties"),Ok|Cancel/*|Help|User1|User2*/, Ok,
      true/*, KStandardGuiItem::add(),KStandardGuiItem::remove() */),
      m_Item(which),m_changed(false),
      m_Client(aClient),m_Rev(aRev)
{
    if ( !name )
    setName( "PropertiesDlg" );
    QWidget * m = makeMainWidget();
    PropertiesDlgLayout = new QHBoxLayout(m, marginHint(), spacingHint(), "PropertiesDlgLayout");

    m_PropertiesListview = new Propertylist(m, "m_PropertiesListview" );
    m_PropertiesListview->setAllColumnsShowFocus( TRUE );
    m_PropertiesListview->setShowSortIndicator( TRUE );
    m_PropertiesListview->setCommitchanges(false);
    m_PropertiesListview->setItemsRenameable(true);
    m_PropertiesListview->setRenameable(0,true);
    m_PropertiesListview->setRenameable(1,true);

    m_PropertiesListview->setFullWidth( TRUE );
    PropertiesDlgLayout->addWidget( m_PropertiesListview);

    m_rightLayout = new QVBoxLayout(0, marginHint(), spacingHint(), "m_rightLayout");
    m_AddButton = new KPushButton(m, "m_AddButton" );
    m_rightLayout->addWidget( m_AddButton );
    m_ModifyButton = new KPushButton(m, "m_ModifyButton" );
    m_rightLayout->addWidget( m_ModifyButton );
    m_DeleteButton = new KPushButton(m, "m_DeleteButton" );
    m_rightLayout->addWidget( m_DeleteButton );
    m_rightSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_rightLayout->addItem(m_rightSpacer);
    PropertiesDlgLayout->addLayout(m_rightLayout);
    m_DeleteButton->setEnabled(false);
    m_ModifyButton->setEnabled(false);

    //PropertiesDlgLayout->addLayout(midLayout);
    languageChange();
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_AddButton, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect( m_ModifyButton, SIGNAL(clicked()), this, SLOT(slotModify()));
    connect( m_DeleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
    connect(this,SIGNAL(helpClicked()),SLOT(slotHelp()));
    connect(m_PropertiesListview,SIGNAL(selectionChanged(QListViewItem*)),this,SLOT(slotSelectionChanged(QListViewItem*)));
//    connect(m_PropertiesListview,SIGNAL(executed(QListViewItem*)),this,SLOT(slotSelectionExecuted(QListViewItem*)));

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
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PropertiesDlg::languageChange()
{
    setCaption( i18n("View and modify properties") );
    QToolTip::add(m_PropertiesListview, i18n( "List of properties set" ));
    m_AddButton->setText(i18n("Add property"));
    m_ModifyButton->setText(i18n("Modify property"));
    m_DeleteButton->setText(i18n("Delete property"));
}

void PropertiesDlg::slotHelp()
{
    qWarning( "PropertiesDlg::slotHelp(): Not implemented yet" );
}

void PropertiesDlg::slotSelectionChanged(QListViewItem*item)
{
    m_DeleteButton->setEnabled(item);
    m_ModifyButton->setEnabled(item);
    if (!item || item->rtti()!=PropertyListViewItem::_RTTI_) return;
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
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    m_PropertiesListview->displayList(propList,true,m_Item->fullName());
    initDone = true;
}

/*!
    \fn PropertiesDlg::exec()
 */
int PropertiesDlg::exec()
{
    return KDialogBase::exec();
}

void PropertiesDlg::polish()
{
    KDialogBase::polish();
    initItem();
}

/*!
    \fn PropertiesDlg::slotSelectionExecuted(QListViewItem*)
 */
void PropertiesDlg::slotSelectionExecuted(QListViewItem*)
{
}

/*!
    \fn PropertiesDlg::slotAdd()
 */
void PropertiesDlg::slotAdd()
{
    /// @TODO Use a object variable to store a reference to dlg for further reuse
    EditProperty_impl dlg(this);
    dlg.setDir(m_Item->isDir());
    if (dlg.exec()==QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(dlg.propName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (m_PropertiesListview->checkExisting(dlg.propName())) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        PropertyListViewItem * ki = new PropertyListViewItem(m_PropertiesListview);
        ki->setMultiLinesEnabled(true);
        ki->setText(0,dlg.propName());
        ki->setText(1,dlg.propValue());
        ki->checkName();
        ki->checkValue();
    }
}

/*!
    \fn PropertiesDlg::slotDelete
 */
void PropertiesDlg::slotDelete()
{
    QListViewItem*qi = m_PropertiesListview->selectedItem();
    if (!qi) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) return;
    if (ki->deleted()) {
        ki->unDeleteIt();
    } else {
        ki->deleteIt();
    }
    slotSelectionChanged(qi);
}


/*!
    \fn PropertiesDlg::slotModify()
 */
void PropertiesDlg::slotModify()
{
    QListViewItem*qi = m_PropertiesListview->selectedItem();
    if (!qi) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (qi);
    if (PropertyListViewItem::protected_Property(ki->currentName())) return;
    /// @TODO Use a object variable to store a reference to dlg for further reuse
    EditProperty_impl dlg(this);
    dlg.setDir(m_Item->isDir());
    dlg.setPropName(ki->currentName());
    dlg.setPropValue(ki->currentValue());
    if (dlg.exec()==QDialog::Accepted) {
        if (PropertyListViewItem::protected_Property(dlg.propName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (m_PropertiesListview->checkExisting(dlg.propName(),qi)) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        ki->setText(0,dlg.propName());
        ki->setText(1,dlg.propValue());
        ki->checkName();
        ki->checkValue();
    }
}

void PropertiesDlg::changedItems(svn::PropertiesMap&toSet,QValueList<QString>&toDelete)
{
    toSet.clear();
    toDelete.clear();
    QListViewItemIterator iter( m_PropertiesListview );
    PropertyListViewItem*ki;
    while ( iter.current() ) {
        ki = static_cast<PropertyListViewItem*> (iter.current());
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
