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
/*!
 * \file client_annotate_parameter.h
 * \brief defining classes working as named parameters for different subversion commit
 *
 * Subversion has various functions which has growing / changing parameter lists from version to version.
 * since subversion 1.4 this changes are more and more unhandy for a c++ wrapper due every time changes to
 * virtual class. This special data containers may reduce changes of signatures to the client interface.
 */

#ifndef CLIENT_ANNOTATE_PARAMETER_H
#define CLIENT_ANNOTATE_PARAMETER_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/revision.h"
#include "svnqt/path.h"
#include "svnqt/shared_pointer.h"

namespace svn
{
    class DiffOptions;
    struct AnnotateParameterData;
    class SVNQT_EXPORT AnnotateParameter
    {
    private:
        SharedPointer<AnnotateParameterData> _data;

    public:
        AnnotateParameter();
        ~AnnotateParameter();

        //!file to annotate.
        AnnotateParameter&path(const Path&path);
        //!file to annotate.
        const Path&path()const;

        AnnotateParameter&revisionRange(const RevisionRange&range);
        const RevisionRange&revisionRange()const;
        AnnotateParameter&pegRevision(const Revision&peg);
        const Revision&pegRevision()const;
        AnnotateParameter&diffOptions(const DiffOptions&options);
        const DiffOptions&diffOptions()const;
        AnnotateParameter&ignoreMimeTypes(bool ignore);
        bool ignoreMimeTypes()const;
        AnnotateParameter&includeMerged(bool inc);
        bool includeMerged()const;
    };
}


#endif
