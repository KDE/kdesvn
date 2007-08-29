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

#if QT_VERSION < 0x040000
    #include <qstring.h>
    #include <qpair.h>
    #include <qvaluelist.h>
    #include <qmap.h>
#else
    #include <QtCore>
#endif

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

    typedef QLIST<AnnotateLine> AnnotatedFile;
    typedef QLIST<DirEntry> DirEntries;
    typedef QLIST<InfoEntry> InfoEntries;
    /// simple list of log entries
    typedef QLIST<LogEntry> LogEntries;
    /// shared_pointer for LogEntriesMap
    typedef SharedPointer<LogEntries> LogEntriesPtr;

    /// map of logentries - key is revision
    typedef QMap<long,LogEntry> LogEntriesMap;
    /// shared_pointer for LogEntriesMap
    typedef SharedPointer<LogEntriesMap> LogEntriesMapPtr;

    typedef QLIST<Status> StatusEntries;
    typedef QLIST<Revision> Revisions;

    /// map of property names to values
    typedef QMap<QString,QString> PropertiesMap;
    /// pair of path, PropertiesMap
    typedef QPair<QString, PropertiesMap> PathPropertiesMapEntry;
    /// vector of path, Properties pairs
    typedef QLIST<PathPropertiesMapEntry> PathPropertiesMapList;
    /// shared pointer for properties
    typedef SharedPointer<PathPropertiesMapList> PathPropertiesMapListPtr;
}

#endif
