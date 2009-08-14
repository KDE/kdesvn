/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef DBOVERVIEW_H
#define DBOVERVIEW_H

#include "ui_dboverview.h"

class DbOverViewData;

namespace svn
{
    class Client;
}

class DbOverview: public QWidget,public Ui::DBOverView
{
    Q_OBJECT
public:
    DbOverview(QWidget *parent = 0, const char *name = 0);
    virtual ~DbOverview();

    static void showDbOverview(svn::Client*aClient = 0);

protected Q_SLOTS:
    virtual void itemActivated(const QItemSelection&,const QItemSelection&);
    virtual void setClient(svn::Client*aClient);
    virtual void deleteCacheItems();
    virtual void deleteRepository();
    virtual void repositorySettings();

protected:
    QString selectedRepository()const;
    void enableButtons(bool);
    void genInfo(const QString&);

private:
    DbOverViewData * _data;
};

#endif
