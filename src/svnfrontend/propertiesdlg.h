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
#include <kdialogbase.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qstring.h>

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

class PropertiesDlg : public KDialogBase
{
    Q_OBJECT

public:
    typedef QMap<QString,QString> tPropEntries;

    PropertiesDlg(const QString&, svn::Client*,
        const svn::Revision&aRev=svn::Revision(svn_opt_revision_working),
        QWidget* parent = 0, const char* name = 0, bool modal = true);
    ~PropertiesDlg();

    bool hasChanged()const;
    void changedItems(tPropEntries&toSet,QValueList<QString>&toDelete);

protected:
    KListView* m_PropertiesListview;
    KPushButton* m_AddButton;
    KPushButton* m_DeleteButton;
    KPushButton* m_ModifyButton;

    QHBoxLayout* PropertiesDlgLayout;
    QVBoxLayout* m_rightLayout;
    QSpacerItem* m_rightSpacer;

    QString m_Item;
    bool m_changed;
    bool initDone;
    svn::Client*m_Client;
    svn::Revision m_Rev;

    bool checkExisting(const QString&aName,QListViewItem*it=0);

protected slots:
    virtual void languageChange();

    virtual void slotHelp();
    virtual void slotSelectionChanged(QListViewItem*);
    virtual void slotSelectionExecuted(QListViewItem*);
    virtual void slotItemRenamed(QListViewItem*item,const QString & str,int col );
    virtual void slotAdd();
    virtual void slotDelete();
    virtual void slotModify();

protected:
    virtual void initItem();

    //! Check if a specific property may just internale
    /*!
     * That means, a property of that may not edit,added or deleted.
     *
     * This moment it just checks for "svn:special"
     * \return true if protected property otherwise false
     */
    static bool protected_Property(const QString&);
public slots:
    int exec();
    virtual void polish();

signals:
    void clientException(const QString&);
};

#endif // PROPERTIESDLG_H
