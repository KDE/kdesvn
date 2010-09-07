/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2010  Rajko Albrecht
    ral@alwins-world.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "client_update_parameter.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/targets.h"
#include "svnqt/path.h"

#include "svnqt/client_parameter_macros.h"

namespace svn {
    struct  SVNQT_NOEXPORT UpdateParameterData
    {
        UpdateParameterData()
        :_destPaths(),_srcRevision(),_depth(DepthInfinity),_ignore_externals(false),_allow_unversioned(false),_sticky_depth(true)
        {}
        Targets _destPaths;
        Revision _srcRevision;
        Depth _depth;
        bool _ignore_externals;
        bool _allow_unversioned;
        bool _sticky_depth;
    };

    UpdateParameter::UpdateParameter()
    {
        _data = new UpdateParameterData();
    }
    UpdateParameter::~UpdateParameter()
    {
        _data = 0;
    }

    GETSET(UpdateParameter,Targets,_destPaths,targets);
    GETSET(UpdateParameter,Revision,_srcRevision,revision);

    GETSETSI(UpdateParameter,Depth,_depth,depth);
    GETSETSI(UpdateParameter,bool,_ignore_externals,ignore_externals);
    GETSETSI(UpdateParameter,bool,_allow_unversioned,allow_unversioned);
    GETSETSI(UpdateParameter,bool,_sticky_depth,sticky_depth);

}