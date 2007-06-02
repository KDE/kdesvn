/* 
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
 * http://www.alwins-world.de/wiki/programs/kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
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
#include "log_entry.hpp"
#include "pool.hpp"

// subversion api
#include "svn_time.h"


namespace svn
{
  LogChangePathEntry::LogChangePathEntry (
    const char *path_,
    char action_,
    const char *copyFromPath_,
    const svn_revnum_t copyFromRevision_)
   : path(QString::FROMUTF8(path_)), action(action_),
     copyFromPath (QString::FROMUTF8(copyFromPath_)),
     copyFromRevision (copyFromRevision_)
  {
  }

  LogChangePathEntry::LogChangePathEntry (const QString &path_,
                      char action_,
                      const QString &copyFromPath_,
                      const svn_revnum_t copyFromRevision_)
    : path(path_),
        action(action_),
        copyFromPath(copyFromPath_),
        copyToPath(QString::null),
        copyFromRevision(copyFromRevision_),
        copyToRevision(-1)
  {
  }

  LogChangePathEntry::LogChangePathEntry()
    : path(QString::null),action(0),copyFromPath(QString::null),copyToPath(QString::null),
        copyFromRevision(-1),copyToRevision(-1)
  {
  }

  LogChangePathEntry::LogChangePathEntry (const QString &path_,
                        char action_,
                        const QString &copyFromPath_,
                        const svn_revnum_t copyFromRevision_,
                        const QString &copyToPath_,
                        const svn_revnum_t copyToRevision_)
    : path(path_),action(action_),copyFromPath(copyFromPath_),copyToPath(copyToPath_),
        copyFromRevision(copyFromRevision_),copyToRevision(copyToRevision_)
  {
  }

  LogEntry::LogEntry ()
    : revision(-1),date(0),author(""),message("")
  {
  }

  LogEntry::LogEntry (
    const svn_revnum_t revision_,
    const char * author_,
    const char * date_,
    const char * message_)
  {
    setDate(date_);

    revision = revision_;
    author = author_ == 0 ? "" : QString::FROMUTF8(author_);
    message = message_ == 0 ? "" : QString::FROMUTF8(message_);
  }

  void LogEntry::setDate(const char*date_)
  {
      apr_time_t date__ = 0;
      if (date_ != 0)
      {
          Pool pool;

          if (svn_time_from_cstring (&date__, date_, pool) != 0)
              date__ = 0;
      }
      date = date__;
  }
}


SVNQT_EXPORT QDataStream& operator<<(QDataStream&s,const svn::LogEntry&r)
{
    s << r.revision
            << r.author
            << r.message
            << r.changedPaths
            << r.date;
    return s;
}

SVNQT_EXPORT QDataStream& operator<<(QDataStream&s,const svn::LogChangePathEntry&r)
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

SVNQT_EXPORT QDataStream& operator>>(QDataStream&s,svn::LogEntry&r)
{
    s >> r.revision
            >> r.author
            >> r.message
            >> r.changedPaths
            >> r.date;
    return s;
}

SVNQT_EXPORT QDataStream& operator>>(QDataStream&s,svn::LogChangePathEntry&r)
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

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
