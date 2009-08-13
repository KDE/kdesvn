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
#ifndef SVNREPOSITORYDATA_H
#define SVNREPOSITORYDATA_H

#include "svnqt/pool.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/apr.hpp"
#include "svnqt/svnqt_defines.hpp"

#include <qstring.h>

#include <svn_repos.h>
#include <svn_error.h>

namespace svn {

namespace repository {

class Repository;
class RepositoryListener;
class CreateRepoParameter;
/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class SVNQT_NOEXPORT RepositoryData{
    friend class Repository;

public:
    RepositoryData(RepositoryListener*);

    virtual ~RepositoryData();
    void Close();
    svn_error_t * Open(const QString&);
    svn_error_t * CreateOpen(const CreateRepoParameter&params);

    void reposFsWarning(const QString&msg);
    svn_error_t* dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas);
    svn_error_t* loaddump(const QString&dump,svn_repos_load_uuid uuida, const QString&parentFolder, bool usePre, bool usePost);
    static svn_error_t* hotcopy(const QString&src,const QString&dest,bool cleanlogs);

protected:
    Pool m_Pool;
    svn_repos_t*m_Repository;
    RepositoryListener*m_Listener;

private:
    static void warning_func(void *baton, svn_error_t *err);
    static svn_error_t*cancel_func(void*baton);
};

}

}

#endif
