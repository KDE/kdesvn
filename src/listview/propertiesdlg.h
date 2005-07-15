/****************************************************************************
** Form interface generated from reading ui file 'propertiesdlg.ui'
**
** Created: Fr Jul 15 12:53:05 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H

#include <qvariant.h>
#include <kdialog.h>

#include "svncpp/revision.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class KListView;
class QListViewItem;
class KPushButton;
class FileListViewItem;
namespace svn {
    class Client;
}

class PropertiesDlg : public KDialog
{
    Q_OBJECT

public:
    PropertiesDlg(const QString&, svn::Client*,const svn::Revision&aRev=svn::Revision::HEAD,QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PropertiesDlg();

    bool hasChanged()const;

protected:
    QLabel* m_Headlabel;
    KListView* m_PropertiesListview;
    KPushButton* m_CloseButton;
    KPushButton* m_HelpButton;
    KPushButton* m_AddButton;
    KPushButton* m_DeleteButton;
    KPushButton* m_ModifyButton;
    KPushButton* m_CancelButton;
    QVBoxLayout* PropertiesDlgLayout;
    QHBoxLayout* midLayout;
    QHBoxLayout* m_ButtonLayout;
    QVBoxLayout* m_rightLayout;
    QSpacerItem* m_rightSpacer;
    QSpacerItem* m_bottomSpacer;

    QString m_Item;
    bool m_changed;
    bool initDone;
    svn::Client*m_Client;
    svn::Revision m_Rev;

protected slots:
    virtual void languageChange();

    virtual void slotHelp();
    virtual void slotSelectionChanged(QListViewItem*);
    virtual void slotSelectionExecuted(QListViewItem*);
    virtual void slotItemRenamed(QListViewItem*item,const QString & str,int col );

protected:
    virtual void initItem();
public slots:
    int exec();
signals:
    void clientException(const QString&);
};

#endif // PROPERTIESDLG_H
