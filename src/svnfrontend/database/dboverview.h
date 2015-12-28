/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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

#include "ui_dboverview.h"
#include <svnqt/client.h>

class DbOverViewData;

class DbOverview: public QWidget, public Ui::DBOverView
{
    Q_OBJECT
public:
    explicit DbOverview(QWidget *parent = 0);
    virtual ~DbOverview();

    static void showDbOverview(const svn::ClientP &aClient);

protected Q_SLOTS:
    virtual void itemActivated(const QItemSelection &, const QItemSelection &);
    virtual void setClient(const svn::ClientP &aClient);
    virtual void deleteCacheItems();
    virtual void deleteRepository();
    virtual void repositorySettings();

protected:
    QString selectedRepository()const;
    void enableButtons(bool);
    void genInfo(const QString &);

private:
    DbOverViewData *_data;
};

#endif
