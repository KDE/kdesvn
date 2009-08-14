/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H

#include <qvariant.h>
#include <kdialog.h>
#include <qmap.h>
#include <qstring.h>
#include <QStringList>

#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/revision.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class Propertylist;
class QTreeWidgetItem;
class KPushButton;
class FileListViewItem;
class SvnItem;

namespace svn {
    class Client;
}

class PropertiesDlg : public KDialog
{
    Q_OBJECT

public:
    PropertiesDlg(SvnItem*, svn::Client*,
        const svn::Revision&aRev=svn::Revision(svn_opt_revision_working),
        QWidget* parent = 0, const char* name = 0, bool modal = true);
    ~PropertiesDlg();

    bool hasChanged()const;
    void changedItems(svn::PropertiesMap&toSet,QStringList&toDelete);

protected:
    Propertylist* m_PropertiesListview;
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

protected slots:
    virtual void languageChange();

    virtual void slotHelp();
    virtual void slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    virtual void slotSelectionExecuted(QTreeWidgetItem*);
    virtual void slotAdd();
    virtual void slotDelete();
    virtual void slotModify();

protected:
    virtual void initItem();
    virtual bool event (QEvent * event);

public slots:
    int exec();

signals:
    void clientException(const QString&);
};

#endif // PROPERTIESDLG_H
