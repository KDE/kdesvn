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
#include "repository.hpp"
#include "repositorydata.hpp"

namespace svn {

namespace repository {

Repository::Repository(svn::repository::RepositoryListener*aListener)
{
    m_Data=new RepositoryData(aListener);
}


Repository::~Repository()
{
    delete m_Data;
}


/*!
    \fn svn::Repository::Open(const QString&)
 */
void Repository::Open(const QString&name) throw (ClientException)
{
    svn_error_t * error = m_Data->Open(name);
    if (error!=0) {
        throw ClientException (error);
    }
}

void Repository::CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync, bool _bdbautologremove, bool _nosvn1diff) throw (ClientException)
{
    svn_error_t * error = m_Data->CreateOpen(path,fstype,_bdbnosync,_bdbautologremove,_nosvn1diff);
    if (error!=0) {
        throw ClientException (error);
    }
}


/*!
    \fn svn::Repository::dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)throw (ClientException)
 */
void Repository::dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)throw (ClientException)
{
    svn_error_t * error = m_Data->dump(output,start,end,incremental,use_deltas);
    if (error!=0) {
        throw ClientException (error);
    }
}


/*!
    \fn svn::Repository::hotcopy(const QString&src,const QString&dest,bool cleanlogs)
 */
void Repository::hotcopy(const QString&src,const QString&dest,bool cleanlogs)throw (ClientException)
{
    svn_error_t * error = RepositoryData::hotcopy(src,dest,cleanlogs);
    if (error!=0) {
        throw ClientException (error);
    }
}

} // namespace repository

} // namespace svn
