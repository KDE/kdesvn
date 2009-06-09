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
/*!
 * \file client_parameter.hpp
 * \brief defining classes working as named parameters for different subversion methods
 *
 * Subversion has various functions which has growing / changing parameter lists from version to version.
 * since subversion 1.4 this changes are more and more unhandy for a c++ wrapper due every time changes to
 * virtual class. This special data containers may reduce changes of signatures to the client interface.
 */

#ifndef CLIENT_PARAMETER_H
#define CLIENT_PARAMETER_H

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/path.hpp"
#include "svnqt/shared_pointer.hpp"

namespace svn
{

    struct CopyParameterData;

    //! parameter for svn_copy wrapper
    /*!
     * This class should never contains virtual methods. New Methods are always appended at end of class definition
     * \sa svn::Client::copy
     */
    class SVNQT_EXPORT CopyParameter
    {
    private:
        SharedPointer<CopyParameterData> _data;
    public:
        CopyParameter(const Targets&_srcPath,const Path&_destPath);
        ~CopyParameter();

        //! Targets for copy operation
        const Targets&targets()const;
        //! Destination path for copy operation
        const Path&destination()const;

        //! set copy operation parameter asChild to true
        CopyParameter&asChild();
        //! set copy operation parameter asChild to false
        CopyParameter&notAsChild();
        //! return value for asChild
        bool getAsChild()const;

        //! copy should ignore externals
        /*!
         * \since subversion 1.6
         */
        CopyParameter&ignoreExternal();
        //! copy should not ignore externals
        /*!
         * \since subversion 1.6
         */
        CopyParameter&notIgnoreExternal();
        //! return externals has to ignored
        /*!
         * \since subversion 1.6
         */
        bool getIgnoreExternal()const;

        //! set copy operation parameter makeParent to true
        CopyParameter&makeParent();
        //! set copy operation parameter makeParent to false
        CopyParameter&notMakeParent();
        //! return value for asChild
        bool getMakeParent()const;

        //! set the source revision for the copy operation
        CopyParameter&srcRevision(const Revision&);
        //! get the source revision for the copy operation
        const Revision& getSrcRevision()const;
        //! set the peg revision for the copy operation
        CopyParameter&pegRevision(const Revision&);
        //! get the peg revision for the copy operation
        const Revision& getPegRevision()const;

        //! set the properties map for the copy operation
        CopyParameter&properties(const PropertiesMap&);
        //! get the properties map for the copy operation
        const PropertiesMap& getProperties()const;

    };
}


#endif
