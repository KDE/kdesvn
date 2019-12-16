/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
#ifndef DBOVERVIEW_H
#define DBOVERVIEW_H

#include "ksvndialog.h"
#include <svnqt/client.h>

namespace Ui
{
class DBOverView;
}
class QItemSelection;
class QStringListModel;

class DbOverview: public KSvnDialog
{
    Q_OBJECT
private:
    explicit DbOverview(const svn::ClientP &aClient, QWidget *parent = nullptr);
    ~DbOverview();

public:
    static void showDbOverview(const svn::ClientP &aClient, QWidget *parent = nullptr);

private Q_SLOTS:
    virtual void itemActivated(const QItemSelection &, const QItemSelection &);
    virtual void deleteCacheItems();
    virtual void deleteRepository();
    virtual void repositorySettings();

protected:
    QString selectedRepository()const;
    void enableButtons(bool);
    void genInfo(const QString &);

private:
    svn::ClientP m_clientP;
    QStringListModel *m_repo_model;
    Ui::DBOverView *m_ui;
};

#endif
