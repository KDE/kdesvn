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
#include "client_annotate_parameter.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/stringarray.hpp"
#include "svnqt/client_parameter_macros.hpp"
#include "svnqt/diffoptions.hpp"

namespace svn
{
    struct AnnotateParameterData
    {
        AnnotateParameterData()
            :_path(),_revisions(Revision::UNDEFINED,Revision::UNDEFINED),_peg(Revision::UNDEFINED),_opts(),
            _ignoreMimeTypes(false),_includeMerged(true)
        {}
        Path _path;
        RevisionRange _revisions;
        Revision _peg;
        DiffOptions _opts;
        bool _ignoreMimeTypes,_includeMerged;
    };

    AnnotateParameter::AnnotateParameter()
    {
        _data = new AnnotateParameterData;
    }
    AnnotateParameter::~AnnotateParameter()
    {
        _data = 0;
    }
    GETSET(AnnotateParameter,Path,_path,path)
    GETSET(AnnotateParameter,RevisionRange,_revisions,revisionRange)
    GETSET(AnnotateParameter,Revision,_peg,pegRevision)
    GETSET(AnnotateParameter,DiffOptions,_opts,diffOptions)

    GETSETSI(AnnotateParameter,bool,_ignoreMimeTypes,ignoreMimeTypes)
    GETSETSI(AnnotateParameter,bool,_includeMerged,includeMerged)
}
