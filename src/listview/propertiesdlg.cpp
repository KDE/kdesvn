/****************************************************************************
** Form implementation generated from reading ui file 'propertiesdlg.ui'
**
** Created: Fr Jul 15 12:53:01 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "propertiesdlg.h"
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

/*
 *  Constructs a PropertiesDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PropertiesDlg::PropertiesDlg(const QString&which, svn::Client*aClient, const svn::Revision&aRev, QWidget* parent, const char* name, bool modal, WFlags fl )
    : KDialog( parent, name, modal, fl ),m_Item(which),m_changed(false),m_Client(aClient),m_Rev(aRev)
{
    if ( !name )
    setName( "PropertiesDlg" );
    PropertiesDlgLayout = new QVBoxLayout( this, 2, 2, "PropertiesDlgLayout");

    m_Headlabel = new QLabel( this, "m_Headlabel" );
    m_Headlabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    PropertiesDlgLayout->addWidget( m_Headlabel );

    midLayout = new QHBoxLayout( 0, 0, 2, "midLayout");

    m_PropertiesListview = new KListView( this, "m_PropertiesListview" );
    m_PropertiesListview->addColumn( i18n( "Properties" ) );
    m_PropertiesListview->addColumn( i18n( "Value" ) );
    m_PropertiesListview->setAllColumnsShowFocus( TRUE );
    m_PropertiesListview->setShowSortIndicator( TRUE );
    //m_PropertiesListview->setDefaultRenameAction( KListView::Accept );
    m_PropertiesListview->setItemsRenameable(true);
    m_PropertiesListview->setRenameable(0,true);
    m_PropertiesListview->setRenameable(1,true);
    m_PropertiesListview->setFullWidth( TRUE );
    midLayout->addWidget( m_PropertiesListview );

    m_rightLayout = new QVBoxLayout( 0, 0, 2, "m_rightLayout");

    m_AddButton = new KPushButton( this, "m_AddButton" );
    m_rightLayout->addWidget( m_AddButton );

    m_ModifyButton = new KPushButton( this, "m_ModifyButton" );
    m_rightLayout->addWidget( m_ModifyButton );

    m_DeleteButton = new KPushButton( this, "m_DeleteButton" );
    m_rightLayout->addWidget( m_DeleteButton );

    m_HelpButton = new KPushButton( this, "m_HelpButton" );
    m_rightLayout->addWidget( m_HelpButton );
    m_rightSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_rightLayout->addItem( m_rightSpacer );
    midLayout->addLayout( m_rightLayout );
    PropertiesDlgLayout->addLayout( midLayout );

    m_ButtonLayout = new QHBoxLayout( 0, 0, 2, "m_ButtonLayout");

    m_CloseButton = new KPushButton( this, "m_CloseButton" );
    m_ButtonLayout->addWidget( m_CloseButton );

    m_CancelButton = new KPushButton( this, "m_CancelButton" );
    m_ButtonLayout->addWidget( m_CancelButton );
    m_bottomSpacer = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    m_ButtonLayout->addItem( m_bottomSpacer );
    PropertiesDlgLayout->addLayout( m_ButtonLayout );

    languageChange();
    resize( QSize(489, 236).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_CloseButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_CancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_HelpButton, SIGNAL( clicked() ), this, SLOT( slotHelp() ) );

    connect(m_PropertiesListview,SIGNAL(itemRenamed(QListViewItem*,const QString&,int)),this,SLOT(slotItemRenamed(QListViewItem*,const QString&,int)));
//    connect(m_PropertiesListview,SIGNAL(selectionChanged(QListViewItem*)),this,SLOT(slotSelectionChanged(QListViewItem*)));
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
    setCaption( i18n( "Properties" ) );
    m_Headlabel->setText( i18n( "<font size=\"+1\">View and modify properties</font>" ) );
    m_PropertiesListview->header()->setLabel( 0, i18n( "Properties" ) );
    m_PropertiesListview->header()->setLabel( 1, i18n( "Value" ) );
    QToolTip::add( m_PropertiesListview, i18n( "List of properties set" ) );
    m_CloseButton->setText( i18n( "&Accept" ) );
    m_CancelButton->setText( i18n( "&Cancel" ) );
    m_HelpButton->setText( i18n( "&Help" ) );
    m_AddButton->setText(i18n("Add property"));
    m_ModifyButton->setText(i18n("Modify property"));
    m_DeleteButton->setText(i18n("Delete property"));
}

void PropertiesDlg::slotHelp()
{
    qWarning( "PropertiesDlg::slotHelp(): Not implemented yet" );
}

void PropertiesDlg::slotSelectionChanged(QListViewItem*)
{
    qWarning( "PropertiesDlg::slotSelectionChanged(QListViewItem*): Not implemented yet" );
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
    svn::Path what(m_Item.local8Bit());
    svn::PathPropertiesMapList propList;
    try {
        propList = m_Client->proplist(what,m_Rev);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    svn::PathPropertiesMapList::const_iterator lit;
    svn::PropertiesMap pmap;
    for (lit=propList.begin();lit!=propList.end();++lit) {
        pmap = lit->second;
        /* just want the first one */
        break;
    }
    svn::PropertiesMap::const_iterator pit;
    for (pit=pmap.begin();pit!=pmap.end();++pit) {
        PropertyListViewItem * ki = new PropertyListViewItem(m_PropertiesListview,
            QString::fromLocal8Bit(pit->first.c_str()),
            QString::fromLocal8Bit(pit->second.c_str()));
        ki->setMultiLinesEnabled(true);
    }
    initDone = true;
}

/*!
    \fn PropertiesDlg::exec()
 */
int PropertiesDlg::exec()
{
    initDone = false;
    initItem();
    if (!initDone) {
        return Rejected;
    }
    return KDialog::exec();
}


/*!
    \fn PropertiesDlg::slotSelectionExecuted(QListViewItem*)
 */
void PropertiesDlg::slotSelectionExecuted(QListViewItem*)
{
    qWarning( "PropertiesDlg::slotSelectionExecuted(QListViewItem*): Not implemented yet" );
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
