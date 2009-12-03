/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "repoparameter.h"

#define SETIT(x,y) _data->x = y ; return *this;

namespace svn {

namespace repository {

    struct CreateRepoParameterData
    {
        CreateRepoParameterData()
            :path(),fstype(QString::fromLatin1("fsfs")),_bdbnosync(false),_bdbautologremove(true),
            _pre_1_4_compat(false),_pre_1_5_compat(false),_pre_1_6_compat(false)
        {}
        QString path;
        QString fstype;
        bool _bdbnosync;
        bool _bdbautologremove;
        bool _pre_1_4_compat;
        bool _pre_1_5_compat;
        bool _pre_1_6_compat;
    };

    CreateRepoParameter::CreateRepoParameter()
    {
        _data = new CreateRepoParameterData();
    }
    CreateRepoParameter::~CreateRepoParameter()
    {
    }
    const QString&CreateRepoParameter::path()const
    {
        return _data->path;
    }
    CreateRepoParameter&CreateRepoParameter::path(const QString&path)
    {
        SETIT(path,path);
    }
    const QString&CreateRepoParameter::fstype()const
    {
        return _data->fstype;
    }
    CreateRepoParameter&CreateRepoParameter::fstype(const QString&fstype)
    {
        SETIT(fstype,fstype);
    }
    bool CreateRepoParameter::bdbnosync()const
    {
        return _data->_bdbnosync;
    }
    CreateRepoParameter&CreateRepoParameter::bdbnosync(bool nosync)
    {
        SETIT(_bdbnosync,nosync)
    }
    bool CreateRepoParameter::bdbautologremove()const
    {
        return _data->_bdbautologremove;
    }
    CreateRepoParameter&CreateRepoParameter::bdbautologremove(bool autologremove)
    {
        SETIT(_bdbautologremove,autologremove)
    }
    bool CreateRepoParameter::pre14_compat()const
    {
        return _data->_pre_1_4_compat;
    }
    CreateRepoParameter&CreateRepoParameter::pre14_compat(bool value)
    {
        SETIT(_pre_1_4_compat,value);
    }
    bool CreateRepoParameter::pre15_compat()const
    {
        return _data->_pre_1_5_compat;
    }
    CreateRepoParameter&CreateRepoParameter::pre15_compat(bool value)
    {
        SETIT(_pre_1_5_compat,value);
    }
    bool CreateRepoParameter::pre16_compat()const
    {
        return _data->_pre_1_6_compat;
    }
    CreateRepoParameter&CreateRepoParameter::pre16_compat(bool value)
    {
        SETIT(_pre_1_6_compat,value);
    }

} // namespace repository
} // namespace svn
