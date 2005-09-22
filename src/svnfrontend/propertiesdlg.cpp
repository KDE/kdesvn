/****************************************************************************
** Form implementation generated from reading ui file 'propertiesdlg.ui'
**
** Created: Fr Jul 15 12:53:01 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "propertiesdlg.h"
#include "editproperty_impl.h"
#include "svncpp/client.hpp"

#include <qvariant.h>
#include <qlabel.h>
#include <qheader.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>

class PropertyListViewItem:public KListViewItem
{
    friend class PropertiesDlg;
public:
    PropertyListViewItem(KListView *parent,const QString&,const QString&);
    PropertyListViewItem(KListView *parent);
    virtual ~PropertyListViewItem();

    const QString&startName()const{return m_startName;}
    const QString&startValue()const{return m_startValue;}
    const QString&currentName()const{return m_currentName;}
    const QString&currentValue()const{return m_currentValue;}

    void checkValue();
    void checkName();
    void deleteIt();
    void unDeleteIt();
    bool deleted()const{return m_deleted;}

    bool different()const;

protected:
    QString m_currentName,m_startName,m_currentValue,m_startValue;
    bool m_deleted;
};

PropertyListViewItem::PropertyListViewItem(KListView *parent,const QString&aName,const QString&aValue)
    : KListViewItem(parent),m_currentName(aName),m_startName(aName),m_currentValue(aValue),m_startValue(aValue),m_deleted(false)
{
    setText(0,startName());
    setText(1,startValue());
}

PropertyListViewItem::PropertyListViewItem(KListView *parent)
    : KListViewItem(parent),m_currentName(""),m_startName(""),m_currentValue(""),m_startValue(""),m_deleted(false)
{
    setText(0,startName());
    setText(1,startValue());
}

PropertyListViewItem::~PropertyListViewItem()
{
}

void PropertyListViewItem::checkValue()
{
    m_currentValue=text(1);
}

void PropertyListViewItem::checkName()
{
    m_currentName=text(0);
}

bool PropertyListViewItem::different()const
{
    return m_currentName!=m_startName || m_currentValue!=m_startValue || deleted();
}

void PropertyListViewItem::deleteIt()
{
    m_deleted = true;
    setPixmap(0,KGlobal::iconLoader()->loadIcon("cancel",KIcon::Desktop,16));
}

void PropertyListViewItem::unDeleteIt()
{
    m_deleted = false;
    setPixmap(0,QPixmap());
}

/*
 *  Constructs a PropertiesDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PropertiesDlg::PropertiesDlg(const QString&which, svn::Client*aClient, const svn::Revision&aRev, QWidget* parent, const char* name, bool modal)
    :
    KDialogBase(parent,name,modal,i18n("Modify properties"),Ok|Cancel/*|Help|User1|User2*/, Ok,
      true/*, KStdGuiItem::add(),KStdGuiItem::remove() */),
      m_Item(which),m_changed(false),
      m_Client(aClient),m_Rev(aRev)
{
    if ( !name )
    setName( "PropertiesDlg" );

    QWidget * m = makeMainWidget();
    PropertiesDlgLayout = new QHBoxLayout(m, marginHint(), spacingHint(), "PropertiesDlgLayout");

    m_PropertiesListview = new KListView(m, "m_PropertiesListview" );
    m_PropertiesListview->addColumn( i18n( "Properties" ) );
    m_PropertiesListview->addColumn( i18n( "Value" ) );
    m_PropertiesListview->setAllColumnsShowFocus( TRUE );
    m_PropertiesListview->setShowSortIndicator( TRUE );
    //m_PropertiesListview->setDefaultRenameAction( KListView::Accept );
#if 0
    m_PropertiesListview->setItemsRenameable(true);
    m_PropertiesListview->setRenameable(0,true);
    m_PropertiesListview->setRenameable(1,true);
#endif
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
    connect(m_PropertiesListview,SIGNAL(itemRenamed(QListViewItem*,const QString&,int)),this,SLOT(slotItemRenamed(QListViewItem*,const QString&,int)));
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
    m_PropertiesListview->header()->setLabel( 0, i18n( "Properties" ) );
    m_PropertiesListview->header()->setLabel( 1, i18n( "Value" ) );
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
    if (!item) return;
    PropertyListViewItem*ki = static_cast<PropertyListViewItem*> (item);
    if (protected_Property(ki->currentName())) {
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
    svn::Path what(m_Item);
    svn::PathPropertiesMapList propList;
    try {
        propList = m_Client->proplist(what,m_Rev);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    svn::PathPropertiesMapList::const_iterator lit;
    svn::PropertiesMap pmap;
    for (lit=propList.begin();lit!=propList.end();++lit) {
        pmap = (*lit).second;
        /* just want the first one */
        break;
    }
    svn::PropertiesMap::const_iterator pit;
    for (pit=pmap.begin();pit!=pmap.end();++pit) {
        PropertyListViewItem * ki = new PropertyListViewItem(m_PropertiesListview,
            pit.key(),
            pit.data());
        ki->setMultiLinesEnabled(true);
    }
    initDone = true;
}

/*!
    \fn PropertiesDlg::exec()
 */
int PropertiesDlg::exec()
{
    kdDebug()<<"Exec"<<endl;
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
    \fn PropertiesDlg::slotItemRenamed(QListViewItem*item,const QString & str,int col )
 */
void PropertiesDlg::slotItemRenamed(QListViewItem*_item,const QString &,int col )
{
    if (!_item) return;
    PropertyListViewItem*item = static_cast<PropertyListViewItem*> (_item);
    if (col==0) {
        item->checkName();
    } else {
        item->checkValue();
    }
}


/*!
    \fn PropertiesDlg::slotAdd()
 */
void PropertiesDlg::slotAdd()
{
    EditProperty_impl dlg(this);
    // TODO decide if selected item is a directory or a file
    dlg.setDir(true);
    if (dlg.exec()==QDialog::Accepted) {
        if (protected_Property(dlg.PropName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (checkExisting(dlg.PropName())) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        PropertyListViewItem * ki = new PropertyListViewItem(m_PropertiesListview);
        ki->setMultiLinesEnabled(true);
        ki->setText(0,dlg.PropName());
        ki->setText(1,dlg.PropValue());
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
    if (protected_Property(ki->currentName())) return;
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
    if (protected_Property(ki->currentName())) return;
    EditProperty_impl dlg(this);
    dlg.setPropName(ki->currentName());
    dlg.setPropValue(ki->currentValue());
    // TODO decide if selected item is a directory or a file
    dlg.setDir(false);
    if (dlg.exec()==QDialog::Accepted) {
        if (protected_Property(dlg.PropName())) {
            KMessageBox::error(this,i18n("This property may not set by users.\nRejecting it."),i18n("Protected property"));
            return;
        }
        if (checkExisting(dlg.PropName(),qi)) {
            KMessageBox::error(this,i18n("A property with that name exists.\nRejecting it."),i18n("Double property"));
            return;
        }
        ki->setText(0,dlg.PropName());
        ki->setText(1,dlg.PropValue());
        ki->checkName();
        ki->checkValue();
    }
}

bool PropertiesDlg::checkExisting(const QString&aName,QListViewItem*it)
{
    if (!it) {
        return m_PropertiesListview->findItem(aName,0)!=0;
    }
    QListViewItemIterator iter( m_PropertiesListview );
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

void PropertiesDlg::changedItems(tPropEntries&toSet,QValueList<QString>&toDelete)
{
    toSet.clear();
    toDelete.clear();
    QListViewItemIterator iter( m_PropertiesListview );
    PropertyListViewItem*ki;
    while ( iter.current() ) {
        ki = static_cast<PropertyListViewItem*> (iter.current());
        ++iter;
        if (protected_Property(ki->currentName())||protected_Property(ki->startName())) {
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

bool PropertiesDlg::protected_Property(const QString&what)
{
    if (what.compare("svn:special")!=0) return false;
    return true;
}

#include "propertiesdlg.moc"

