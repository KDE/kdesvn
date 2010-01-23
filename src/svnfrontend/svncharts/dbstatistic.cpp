/***************************************************************************
 *   Copyright (C) 2006-2010 by Rajko Albrecht                             *
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

#include "dbstatistic.h"
#include "src/svnqt/cache/LogCache.h"
#include "src/svnqt/cache/ReposConfig.h"

#include <kdebug.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#define getdb() QDataBase reposDB = svn::cache::LogCache::self()->reposDb(_reposName)

DbStatistic::DbStatistic(const QString&reposName)
    :_reposName(reposName)
{
}

DbStatistic::~DbStatistic()
{
}

bool DbStatistic::getUserCommits(Usermap&target)const
{
    getdb();
    if (!reposDB.isValid()) {
        return false;
    }
    QStringList _v = svn::cache::ReposConfig::self()->readEntry(_reposName,"exclude_log_users",QStringList());
    if (svn::cache::ReposConfig::self()->readEntry(_reposName,"filter_empty_author",false)) {
        _v.append(QString());
    }
    static QString s_aCount("select count(*),author from logentries %1 group by author");

    QString where;

    if (_v.count()>0) {
        where = QString("where author not in ('%1')").arg(_v.join("','"));
    } else {
        where = "where 1";
    }
    _v = svn::cache::ReposConfig::self()->readEntry(_reposName,"exclude_log_pattern",QStringList());
    for (int i = 0; i<_v.count();++i) {
        where.append(QString(" and message not like '%%1%'").arg(_v[i]));
    }

    QString r_aCount = s_aCount.arg(where);
    kDebug()<<"Query: "<<r_aCount<<endl;
    QSqlQuery acount(QString(),reposDB);
    acount.prepare(r_aCount);
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif

    if (!acount.exec()) {
        kDebug() << acount.lastError().text();
        //throw svn::cache::DatabaseException(QString("Could not retrieve authors count: ")+acount.lastError().text());
        return false;
    }
#ifdef DEBUG_TIMER
    kDebug()<<"Time for getting db entries: "<<_counttime.elapsed();
    _counttime.restart();
#endif
    while(acount.next()) {
        target[acount.value(1).toString()]=acount.value(0).toUInt();
    }
#ifdef DEBUG_TIMER
    kDebug()<<"Time for copy db entries: "<<_counttime.elapsed();
#endif
    return true;
}
