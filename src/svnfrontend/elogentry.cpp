/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "elogentry.h"

eLogChangePathEntry::eLogChangePathEntry()
    : svn::LogChangePathEntry()
{
    copyToRevision = -1;
    copyToPath = "";
}

eLogChangePathEntry::eLogChangePathEntry(const svn::LogChangePathEntry&old)
    : svn::LogChangePathEntry()
{
    action = old.action;
    copyFromPath = old.copyFromPath;
    copyFromRevision = old.copyFromRevision;
    path = old.path;
    copyToRevision = -1;
    copyToPath = "";
}

eLog_Entry::eLog_Entry()
 : svn::LogEntry()
{
}

eLog_Entry::eLog_Entry(const svn::LogEntry&old)
    : svn::LogEntry()
{
    changedPaths=old.changedPaths;
    author = old.author;
    date = old.date;
    message = old.message;
    revision = old.revision;
    convertList(changedPaths);
}

eLog_Entry::~eLog_Entry()
{
}

void eLog_Entry::convertList(const QValueList<svn::LogChangePathEntry>&oldpathes)
{
    for (unsigned i = 0; i<oldpathes.count();++i) {
        echangedPaths.push_back(eLogChangePathEntry(oldpathes[i]));
    }
}

void eLog_Entry::addCopyTo(const QString&current,const QString&target,svn_revnum_t target_rev)
{
    eLogChangePathEntry _entry;
    _entry.copyToPath=target;
    _entry.path = current;
    _entry.copyToRevision = target_rev;
    _entry.action=0;
    echangedPaths.push_back(_entry);
}
