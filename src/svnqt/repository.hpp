/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#ifndef SVNREPOSITORY_H
#define SVNREPOSITORY_H

#include "exception.hpp"
#include "revision.hpp"

#include <qstring.h>

namespace svn {

class RepositoryData;
class RepositoryListener;

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class Repository{
public:
    Repository(svn::RepositoryListener*);

    virtual ~Repository();
    void Open(const QString&) throw (ClientException);
    void CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync = false,
        bool _bdbautologremove = true, bool nosvn1diff=false) throw (ClientException);
    void dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)throw (ClientException);
    void hotcopy(const QString&src,const QString&dest,bool cleanlogs)throw (ClientException);

private:
    RepositoryData*m_Data;
};

}

#endif
