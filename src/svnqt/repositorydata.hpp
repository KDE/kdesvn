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
#ifndef SVNREPOSITORYDATA_H
#define SVNREPOSITORYDATA_H

#include "pool.hpp"
#include "apr.hpp"

#include <qstring.h>

#include <svn_repos.h>
#include <svn_error.h>

namespace svn {

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class RepositoryData{
public:
    RepositoryData();

    virtual ~RepositoryData();
    void Close();
    svn_error_t * Open(const QString&);
    svn_error_t * CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync = false,
        bool _bdbautologremove = true, bool nosvn1diff=false);

    void reposFsWarning(const QString&msg);

protected:
    Pool m_Pool;
    svn_repos_t*m_Repository;

private:
    static void warning_func(void *baton, svn_error_t *err);
};

}

#endif
