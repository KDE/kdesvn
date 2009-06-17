/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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

#ifndef _SVNQT_TYPES_HPP
#define _SVNQT_TYPES_HPP

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/shared_pointer.hpp"

// qt
#include <qglobal.h>
#include <QtCore>

namespace svn
{
    // forward declarations
    class AnnotateLine;
    class Context;
    class DirEntry;
    class InfoEntry;
    class LogEntry;
    class Revision;
    class Status;
    class Targets;
    class Path;
    class StringArray;
    class CommitItem;
    class CopyParameter;
    class DiffParameter;
    class StatusParameter;

    typedef QLIST<AnnotateLine> AnnotatedFile;

    typedef SharedPointer<DirEntry> DirEntryPtr;
    typedef QLIST<DirEntryPtr> DirEntries;
    typedef QLIST<InfoEntry> InfoEntries;
    /// simple list of log entries
    typedef QLIST<LogEntry> LogEntries;
    /// shared_pointer for LogEntriesMap
    typedef SharedPointer<LogEntries> LogEntriesPtr;

    /// map of logentries - key is revision
    typedef QMap<long,LogEntry> LogEntriesMap;
    /// shared_pointer for LogEntriesMap
    typedef SharedPointer<LogEntriesMap> LogEntriesMapPtr;

    typedef SharedPointer<Status> StatusPtr;
    typedef QLIST<StatusPtr> StatusEntries;
    typedef QLIST<Revision> Revisions;

    /** Range of Revision */
    typedef QPair<Revision,Revision> RevisionRange;
    /** list of revision ranges */
    typedef QLIST<RevisionRange> RevisionRanges;

    /// map of property names to values
    typedef QMap<QString,QString> PropertiesMap;
    /// pair of path, PropertiesMap
    typedef QPair<QString, PropertiesMap> PathPropertiesMapEntry;
    /// vector of path, Properties pairs
    typedef QLIST<PathPropertiesMapEntry> PathPropertiesMapList;
    /// shared pointer for properties
    typedef SharedPointer<PathPropertiesMapList> PathPropertiesMapListPtr;

    typedef QLIST<Path> Pathes;

    typedef QLIST<CommitItem> CommitItemList;

    //! Mapper enum for svn_depth_t
    /*!
     * Until subversion prior 1.5 is supported by this lib we must hide the svn_depth_t enum from interface.
     * \since subversion 1.5 / svnqt 1.0
     * \sa svn_depth_t
     */
    enum Depth {
        DepthUnknown,
        DepthExclude,
        DepthEmpty,
        DepthFiles,
        DepthImmediates,
        DepthInfinity
    };

    //! For search specific server capabilities
    /*!
     * \since subversion 1.5
     * when build with subversion earlier 1.5 this will not used.
     * \sa svn_repos_has_capability
     */
    enum Capability {
        CapabilityMergeinfo=0
    };
}

#endif
