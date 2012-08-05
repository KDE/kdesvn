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
#ifndef ELOGENTRY_H
#define ELOGENTRY_H

#include <svnqt/log_entry.h>

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
struct eLog_Entry : public svn::LogEntry
{
    eLog_Entry();
    eLog_Entry(const svn::LogEntry&);
    ~eLog_Entry();

    void addCopyTo(const QString&,const QString&,svn_revnum_t,char _action,svn_revnum_t fromRev=-1);
    void addAction(const QString&,char _action);
};
#endif
