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
#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H

#include <qvariant.h>
#include <kdialogbase.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qstring.h>

#include "src/svnqt/revision.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class KListView;
class QListViewItem;
class KPushButton;
class FileListViewItem;
class SvnItem;

namespace svn {
    class Client;
}

class PropertiesDlg : public KDialogBase
{
    Q_OBJECT

public:
    typedef QMap<QString,QString> tPropEntries;

    PropertiesDlg(SvnItem*, svn::Client*,
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

    SvnItem *m_Item;
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
