/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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

#include <kdebug.h>

eLog_Entry::eLog_Entry(const svn::LogEntry&old)
    : svn::LogEntry(old)
{
}

eLog_Entry::eLog_Entry()
    : svn::LogEntry()
{
}

eLog_Entry::~eLog_Entry()
{
}

void eLog_Entry::addCopyTo(const QString&current,const QString&target,
                            svn_revnum_t target_rev,char _action,svn_revnum_t from_rev)
{
    svn::LogChangePathEntry _entry;
    _entry.copyToPath=target;
    _entry.path = current;
    _entry.copyToRevision = target_rev;
    _entry.action=_action;
    _entry.copyFromRevision = from_rev;
    switch (_action) {
        case 'A':
            if (!target.isEmpty()) {
                _entry.action = 'H';
            }else{
            }
            break;
        case 'D':
            break;
        case 'R':
            break;
        case 'M':
            break;
        default:
            break;
    }
    /* make sure that ALL writing operations are BEFORE deletion of item,
     * otherwise search will fail */
    if (_action=='D') {
        changedPaths.push_back(_entry);
    } else {
        changedPaths.push_front(_entry);
    }
}
