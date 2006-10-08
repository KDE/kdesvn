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

#ifndef _SVNCPP_LOG_ENTRY_H_
#define _SVNCPP_LOG_ENTRY_H_

#include "svnqt/svnqt_defines.hpp"

//Qt
#include <qglobal.h>

#if QT_VERSION < 0x040000

#include <qstring.h>
#include <qvaluelist.h>

#else

#include <QtCore>

#endif

// apr
#include "apr_time.h"

// subversion api
#include "svn_types.h"

namespace svn
{

  class SVNQT_EXPORT LogChangePathEntry
  {
  public:
    LogChangePathEntry (const char *path_,
                        char action_,
                        const char *copyFromPath_,
                        const svn_revnum_t copyFromRevision_);

    LogChangePathEntry (const QString &path_,
                        char action_,
                        const QString &copyFromPath_,
                        const svn_revnum_t copyFromRevision_);

    LogChangePathEntry (const QString &path_,
                        char action_,
                        const QString &copyFromPath_,
                        const svn_revnum_t copyFromRevision_,
                        const QString &copyToPath_,
                        const svn_revnum_t copyToRevision_);

    LogChangePathEntry();

    QString path;
    char action;
    QString copyFromPath;
    svn_revnum_t copyFromRevision;
    //! future use or useful in backends
    QString copyToPath;
    //! future use or useful in backends
    svn_revnum_t copyToRevision;
  };

#if QT_VERSION < 0x040000
  typedef QValueList<LogChangePathEntry> LogChangePathEntries;
#else
  typedef QList<LogChangePathEntry> LogChangePathEntries;
#endif

  class SVNQT_EXPORT LogEntry
  {
  public:
    LogEntry ();

    LogEntry (const svn_revnum_t revision,
              const char * author,
              const char * date,
              const char * message);

    svn_revnum_t revision;
    QString author;
    QString message;
    LogChangePathEntries changedPaths;
    apr_time_t date;
  };
}

SVNQT_EXPORT QDataStream& operator<<(QDataStream&s,const svn::LogEntry&r);
SVNQT_EXPORT QDataStream& operator<<(QDataStream&s,const svn::LogChangePathEntry&r);

SVNQT_EXPORT QDataStream& operator>>(QDataStream&s,svn::LogEntry&r);
SVNQT_EXPORT QDataStream& operator>>(QDataStream&s,svn::LogChangePathEntry&r);

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

