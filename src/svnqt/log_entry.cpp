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
// svncpp
#include "log_entry.h"
#include "pool.h"
#include "stringarray.h"
#include "helper.h"

// subversion api
#include "svn_time.h"
#include "svn_compat.h"

#include <QDataStream>

namespace svn
{

LogEntry::LogEntry()
    : revision(-1), date(0)
{
}

LogEntry::LogEntry(svn_log_entry_t *log_entry, const StringArray &excludeList)
    : revision(-1), date(0)
{
    Pool pool;
    const char *author_ = nullptr;
    const char *date_ = nullptr;
    const char *message_ = nullptr;
    svn_compat_log_revprops_out(&author_, &date_, &message_, log_entry->revprops);

    author = author_ == nullptr ? QString() : QString::fromUtf8(author_);
    message = message_ == nullptr ? QString() : QString::fromUtf8(message_);
    apr_time_t apr_time = 0;
    if (date_) {
        svn_error_t *err = svn_time_from_cstring(&apr_time, date_, pool);
        svn_error_clear(err); // clear possible error
    }
    date = apr_time;
    revision = log_entry->revision;
    if (log_entry->changed_paths2) {
        bool blocked = false;
        for (apr_hash_index_t *hi = apr_hash_first(pool, log_entry->changed_paths2);
                hi != nullptr;
                hi = apr_hash_next(hi)) {
            const void *pv;
            void *val;
            blocked = false;
            apr_hash_this(hi, &pv, nullptr, &val);
            svn_log_changed_path2_t *log_item = reinterpret_cast<svn_log_changed_path2_t *>(val);
            const char *path = reinterpret_cast<const char *>(pv);
            QString _p(QString::fromUtf8(path));
            for (const QString &exclude : excludeList.data()) {
                if (_p.startsWith(exclude)) {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) {
                changedPaths.push_back(LogChangePathEntry(_p,
                                                          log_item->action,
                                                          QString::fromUtf8(log_item->copyfrom_path),
                                                          log_item->copyfrom_rev));
            }
        }
    }
}
}


SVNQT_EXPORT QDataStream &operator<<(QDataStream &s, const svn::LogEntry &r)
{
    s << r.revision
      << r.author
      << r.message
      << r.changedPaths
      << r.date;
    return s;
}

SVNQT_EXPORT QDataStream &operator<<(QDataStream &s, const svn::LogChangePathEntry &r)
{
    short ac = r.action;
    s << r.path
      << ac
      << r.copyFromPath
      << r.copyFromRevision
      << r.copyToPath
      << r.copyToRevision;
    return s;
}

SVNQT_EXPORT QDataStream &operator>>(QDataStream &s, svn::LogEntry &r)
{
    s >> r.revision
      >> r.author
      >> r.message
      >> r.changedPaths
      >> r.date;
    return s;
}

SVNQT_EXPORT QDataStream &operator>>(QDataStream &s, svn::LogChangePathEntry &r)
{
    short ac;
    s >> r.path
      >> ac
      >> r.copyFromPath
      >> r.copyFromRevision
      >> r.copyToPath
      >> r.copyToRevision;
    r.action = ac;
    return s;
}
