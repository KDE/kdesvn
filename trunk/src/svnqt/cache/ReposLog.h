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
#ifndef REPOS_LOG_H
#define REPOS_LOG_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/revision.h"

#include <QSqlDatabase>
#include <QString>
#include <QStringList>


namespace svn
{

class Client;

namespace cache
{

class SVNQT_EXPORT ReposLog
{
protected:
    svn::Client*m_Client;
    mutable QDataBase m_Database;
    QString m_ReposRoot;
    svn::Revision m_latestHead;
    //! internal insert.
    bool _insertLogEntry(const svn::LogEntry&);
    bool checkFill(svn::Revision&_start,svn::Revision&_end,bool checkHead);

public:
    explicit ReposLog(svn::Client*aClient,const QString&aRepository=QString());

    QString ReposRoot() const
    {
        return m_ReposRoot;
    }

    QDataBase Database() const
    {
        return m_Database;
    }
    //! search for latest head revision on network for assigned repository
    svn::Revision latestHeadRev();
    //! return lates revision in cache
    svn::Revision latestCachedRev();
    //! simple retrieves logentries
    /*!
     * This method acts on network, too for checking if there are new entries on server.
     *
     * @param target where to store the result
     * @param start revision to start for search
     * @param end revision to end for search
     * @param noNetwork if yes, no check on network for newer revisions will made
     * @return true if entries found and no error, if no entries found false
     * @exception svn::DatabaseException in case of errors
     */
    bool simpleLog(LogEntriesMap&target,const svn::Revision&start,const svn::Revision&end,bool noNetwork=false,const QStringList&exclude=QStringList());
    svn::Revision date2numberRev(const svn::Revision&,bool noNetwork=false);
    bool fillCache(const svn::Revision&end);
    bool insertLogEntry(const svn::LogEntry&);
    void cleanLogEntries();
    bool log(const svn::Path&,const svn::Revision&start, const svn::Revision&end,const svn::Revision&peg,svn::LogEntriesMap&target, bool strictNodeHistory,int limit);
    bool itemExists(const svn::Revision&,const svn::Path&);

    qlonglong count()const;
    qlonglong itemCount()const;
    qlonglong fileSize()const;

    bool isValid()const;
};

}
}

#endif
