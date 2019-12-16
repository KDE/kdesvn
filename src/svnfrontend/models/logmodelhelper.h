/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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
#ifndef LOGMODELHELPER_H
#define LOGMODELHELPER_H

#include <QTreeWidgetItem>
#include <QString>

#include "svnqt/svnqttypes.h"
#include "svnqt/log_entry.h"

class LogChangePathItem: public QTreeWidgetItem
{
public:
    explicit LogChangePathItem(const svn::LogChangePathEntry &, QTreeWidget *view = nullptr);
    virtual ~LogChangePathItem() {}

    char action() const
    {
        return _action;
    }
    const QString &path() const
    {
        return _path;
    }
    const QString &source() const
    {
        return _source;
    }
    qlonglong revision() const
    {
        return _revision;
    }

protected:
    QString _path, _source;
    char _action;
    qlonglong _revision;

    void init(const svn::LogChangePathEntry &);
};

class SvnLogModelNode
{

public:
    explicit SvnLogModelNode(const svn::LogEntry &_entry);

    const svn::LogChangePathEntries &changedPaths()const;
    void setChangedPaths(const svn::LogEntry &);

    qlonglong revision()const
    {
        return _data.revision;
    }
    const QString &author()const
    {
        return _data.author;
    }
    const QString &message()const
    {
        return _data.message;
    }
    const QString &shortMessage()const
    {
        return _shortMessage;
    }
    const QDateTime &date()const
    {
        return _date;
    }
    const qlonglong &dateMSec() const
    {
        return _data.date;
    }
    void setRealName(const QString &_n)
    {
        _realName = _n;
    }
    const QString &realName()const
    {
        return _realName;
    }

    bool copiedFrom(QString &_n, qlonglong &_rev)const;
    static bool isParent(const QString &_par, const QString &tar);

protected:
    //we require the ownership!
    svn::LogEntry _data;
    QString _realName;
    QDateTime _date;
    QString _shortMessage;
};

#endif

