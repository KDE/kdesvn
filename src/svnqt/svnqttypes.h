/***************************************************************************
 *   Copyright (C) 2007-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/

#ifndef SVNQT_TYPES_H
#define SVNQT_TYPES_H

#include <QMap>
#include <QPair>
#include <QSharedPointer>
#include <QVector>

namespace svn
{
// forward declarations
class AnnotateLine;
class Context;
class DirEntry;
class Entry;
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
class LogParameter;
class PropertiesParameter;
class MergeParameter;
class CheckoutParameter;
class CommitParameter;
class AnnotateParameter;
class UpdateParameter;

typedef QVector<AnnotateLine> AnnotatedFile;
typedef QSharedPointer<svn::Context> ContextP;
typedef QWeakPointer<svn::Context> ContextWP;

typedef QVector<DirEntry> DirEntries;
typedef QVector<InfoEntry> InfoEntries;
/// simple list of log entries
typedef QVector<LogEntry> LogEntries;

/// map of logentries - key is revision
typedef QMap<long, LogEntry> LogEntriesMap;
typedef QSharedPointer<LogEntriesMap> LogEntriesMapPtr;

typedef QSharedPointer<Status> StatusPtr;
typedef QVector<StatusPtr> StatusEntries;
typedef QVector<Revision> Revisions;

/** Range of Revision */
typedef QPair<Revision, Revision> RevisionRange;
/** list of revision ranges */
typedef QVector<RevisionRange> RevisionRanges;

/// map of property names to values
typedef QMap<QString, QString> PropertiesMap;
/// pair of path, PropertiesMap
typedef QPair<QString, PropertiesMap> PathPropertiesMapEntry;
/// vector of path, Properties pairs
typedef QVector<PathPropertiesMapEntry> PathPropertiesMapList;
/// shared pointer for properties
typedef QSharedPointer<PathPropertiesMapList> PathPropertiesMapListPtr;

typedef QVector<Path> Paths;

typedef QVector<CommitItem> CommitItemList;

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
    CapabilityMergeinfo = 0,
    CapabilityDepth,
    CapabilityCommitRevProps,
    CapabilityLogRevProps
};

namespace repository
{
class CreateRepoParameter;
}
}

#endif
