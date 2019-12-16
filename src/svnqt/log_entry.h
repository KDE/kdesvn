/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * https://kde.org/applications/development/org.kde.kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not,
 * write to the Free Software Foundation, Inc., 51 Franklin St,
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef SVNQT_LOG_ENTRY_H
#define SVNQT_LOG_ENTRY_H

#include <svnqt/svnqt_defines.h>
#include <svnqt/datetime.h>

#include <QList>
#include <QString>
#include <QVector>

// apr
#include <apr_time.h>

// subversion api
#include <svn_types.h>
#include <svn_version.h>

namespace svn
{

class StringArray;
struct LogChangePathEntry
{
    LogChangePathEntry() = default;
    LogChangePathEntry(const QString &path_,
                       char action_,
                       const QString &copyFromPath_,
                       const svn_revnum_t copyFromRevision_)
        : path(path_)
        , copyFromPath(copyFromPath_)
        , copyFromRevision(copyFromRevision_)
        , action(action_)
    {}

    QString path;
    QString copyFromPath;
    //! future use or useful in backends
    QString copyToPath;

    qlonglong copyFromRevision;
    //! future use or useful in backends
    qlonglong copyToRevision;
    char action = '\0';
};

typedef QVector<LogChangePathEntry> LogChangePathEntries;

class SVNQT_EXPORT LogEntry
{
public:
    LogEntry();
    LogEntry(svn_log_entry_t *log_entry, const StringArray &excludeList);

    //! if -1 the entry is a fake entry and not real usable!
    qlonglong revision;
    qlonglong date; // apr_time
    QString author;
    QString message;
    LogChangePathEntries changedPaths;
    QList<qlonglong> m_MergedInRevisions;
};
}

SVNQT_EXPORT QDataStream &operator<<(QDataStream &s, const svn::LogEntry &r);
SVNQT_EXPORT QDataStream &operator<<(QDataStream &s, const svn::LogChangePathEntry &r);

SVNQT_EXPORT QDataStream &operator>>(QDataStream &s, svn::LogEntry &r);
SVNQT_EXPORT QDataStream &operator>>(QDataStream &s, svn::LogChangePathEntry &r);

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
