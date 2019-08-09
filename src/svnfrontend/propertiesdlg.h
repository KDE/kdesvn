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
#pragma once

#include <QStringList>

#include "svnqt/client.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/revision.h"
#include "ksvnwidgets/ksvndialog.h"

class SvnItem;
class QTreeWidgetItem;

namespace Ui
{
class PropertiesDlg;
}
class PropertiesDlg : public KSvnDialog
{
    Q_OBJECT
public:
    PropertiesDlg(SvnItem *which, const svn::ClientP &aClient,
                  const svn::Revision &aRev, QWidget *parent = nullptr);
    ~PropertiesDlg();

    void changedItems(svn::PropertiesMap &toSet, QStringList &toDelete);

protected:
    SvnItem *m_Item;
    svn::ClientP m_Client;
    svn::Revision m_Rev;
    Ui::PropertiesDlg *m_ui;

protected Q_SLOTS:
    void slotCurrentItemChanged(QTreeWidgetItem *);
    void slotAdd();
    void slotDelete();
    void slotModify();

protected:
    void initItem();

Q_SIGNALS:
    void clientException(const QString &);
};
