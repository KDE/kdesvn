/*
    The Kdesvn project - subversion client for kde http://kdesvn.alwins-world.de/
    Copyright (C) 2010  Rajko Albrecht

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef COMMITBUGTRACKERVALUES_H
#define COMMITBUGTRACKERVALUES_H

#include "src/svnqt/shared_pointer.h"

class CommitBugtrackerValuesData;

//! Simple class holding the values for bugtracker integration into commit dialog
/*!
 * Document is taken from here: http://kdesvn.alwins-world.de/attachment/ticket/156/issuetrackers.txt
 * origin document from Tortoise SVN http://tortoisesvn.tigris.org/svn/tortoisesvn/trunk/doc/issuetrackers.txt (login as 'guest')
 */
class CommitBugtrackerValues
{
private:
    //! data holder
    svn::SharedPointer<CommitBugtrackerValuesData> _data;

public:
    //! constructor
    CommitBugtrackerValues();
    //! copy constructor
    CommitBugtrackerValues(const CommitBugtrackerValues&old);
    //! destructor
    ~CommitBugtrackerValues();

    //! bugtraq:warnifnoissue
    bool Warnifnoissue()const;
    //! bugtraq:warnifnoissue
    CommitBugtrackerValues& Warnifnoissue(bool);

    //! bugtraq:label
    const QString&Label()const;
    //! bugtraq:label
    CommitBugtrackerValues&Label(const QString&);

    //! bugtraq:message
    const QString&Message()const;
    //! bugtraq:message
    CommitBugtrackerValues&Message(const QString&);

    //! bugtraq:number
    bool Number()const;
    //! bugtraq:number
    CommitBugtrackerValues&Number(bool);

    //! bugtraq:append
    bool Append()const;
    //! bugtraq:append
    CommitBugtrackerValues&Append(bool);
};

#endif // COMMITBUGTRACKERVALUES_H
