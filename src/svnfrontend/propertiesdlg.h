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
#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H

#include <qvariant.h>
#include <kdialog.h>
#include <qmap.h>
#include <qstring.h>
#include <QStringList>

#include "src/svnqt/client.h"
#include "src/svnqt/svnqttypes.h"
#include "src/svnqt/revision.h"

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


class PropertiesDlg : public KDialog
{
    Q_OBJECT

public:
    PropertiesDlg(SvnItem *which, const svn::ClientP &aClient,
                  const svn::Revision &aRev, QWidget *parent = 0);
    ~PropertiesDlg();

    bool hasChanged()const;
    void changedItems(svn::PropertiesMap &toSet, QStringList &toDelete);

protected:
    Propertylist *m_PropertiesListview;
    KPushButton *m_AddButton;
    KPushButton *m_DeleteButton;
    KPushButton *m_ModifyButton;

    QHBoxLayout *PropertiesDlgLayout;
    QVBoxLayout *m_rightLayout;
    QSpacerItem *m_rightSpacer;

    SvnItem *m_Item;
    bool m_changed;
    bool initDone;
    svn::ClientP m_Client;
    svn::Revision m_Rev;

protected slots:
    virtual void languageChange();

    virtual void slotHelp();
    virtual void slotCurrentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    virtual void slotSelectionExecuted(QTreeWidgetItem *);
    virtual void slotAdd();
    virtual void slotDelete();
    virtual void slotModify();

protected:
    virtual void initItem();
    virtual bool event(QEvent *event);

public slots:
    int exec();

signals:
    void clientException(const QString &);
};

#endif // PROPERTIESDLG_H
