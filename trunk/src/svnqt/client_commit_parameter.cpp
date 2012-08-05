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
#include "client_commit_parameter.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/stringarray.h"
#include "svnqt/client_parameter_macros.h"

namespace svn
{
    struct CommitParameterData
    {
        CommitParameterData()
            :_targets(),_message(QString()),_depth(DepthInfinity),_changeList(StringArray()),
             _revProps(PropertiesMap()),_keepLocks(false),_keepChangeList(false)
        {}
        Targets _targets;
        QString _message;
        Depth _depth;
        StringArray _changeList;
        PropertiesMap _revProps;
        bool _keepLocks,_keepChangeList;
    };

    CommitParameter::CommitParameter()
    {
        _data = new CommitParameterData;
    }
    CommitParameter::~CommitParameter()
    {
        _data = 0;
    }
    GETSET(CommitParameter,Targets,_targets,targets)
    GETSET(CommitParameter,QString,_message,message)
    GETSET(CommitParameter,StringArray,_changeList,changeList)
    GETSET(CommitParameter,PropertiesMap,_revProps,revisionProperties)
    GETSETSI(CommitParameter,Depth,_depth,depth)
    GETSETSI(CommitParameter,bool,_keepLocks,keepLocks)
    GETSETSI(CommitParameter,bool,_keepChangeList,keepChangeList)
}
