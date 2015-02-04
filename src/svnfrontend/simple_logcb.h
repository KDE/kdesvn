/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#ifndef SIMPLE_LOGCB_H
#define SIMPLE_LOGCB_H

namespace svn
{
class LogEntry;
class Revision;
}

class QString;

//! Helper for getting specific svn logentries
class SimpleLogCb
{
public:
    //! empty constructor
    SimpleLogCb() {}
    //! destructor
    virtual ~SimpleLogCb() {}
    //! retrieve a logentry
    /*!
     * @param logtarget target buffer where to store logentry
     * @param rev Revision to get logentry
     * @param what item to search for
     * @param peg Peg revision, may equal to  \a rev
     * @param root the root repository of item, if not given, it has to get detected in implementation from \a what and used.
     * @return true if logentry and root found, otherwise false.
     */
    virtual bool getSingleLog(svn::LogEntry &logtarget, const svn::Revision &rev, const QString &what, const svn::Revision &peg, QString &root) = 0;
};

#endif
