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
        const Targets&srcPath()const;
        //! Destination path for copy operation
        const Path&destination()const;

        //! set copy operation parameter asChild to true
        CopyParameter&asChild(bool);
        //! return value for asChild
        bool asChild()const;

        //! copy should ignore externals
        /*!
         * \since subversion 1.6
         */
        CopyParameter&ignoreExternal(bool);
        //! return externals has to ignored
        /*!
         * \since subversion 1.6
         */
        bool ignoreExternal()const;

        //! set copy/move operation parameter makeParent to true
        CopyParameter&makeParent(bool);
        //! return value for asChild
        bool makeParent()const;

        //! set move operation parameter force to true
        /*! this is ignored for copy operation */
        CopyParameter&force(bool);
        //! return value for force
        /*! this is ignored for copy operation */
        bool force()const;

        //! set the source revision for the copy operation
        CopyParameter&srcRevision(const Revision&);
        //! get the source revision for the copy operation
        const Revision& srcRevision()const;
        //! set the peg revision for the copy operation
        CopyParameter&pegRevision(const Revision&);
        //! get the peg revision for the copy operation
        const Revision& pegRevision()const;

        //! set the properties map for the copy operation
        CopyParameter&properties(const PropertiesMap&);
        //! get the properties map for the copy operation
        const PropertiesMap& properties()const;

    };

    struct DiffParameterData;

    //! parameter for svn_diff and svn_diff_peg wrapper
    /*!
     * This class should never contains virtual methods. New Methods are always appended at end of class definition
     * \sa svn::Client::diff svn::Client::diff_peg
     */
    class SVNQT_EXPORT DiffParameter
    {
    private:
        SharedPointer<DiffParameterData> _data;
    public:
        DiffParameter();
        ~DiffParameter();

        //! Changelist filter
        /*!
         * if empty. no filtering is done
         * \since subversion 1.5
         * \sa svn_client_diff4
         */
        const svn::StringArray& changeList()const;
        //! type of recurse operation
        /*!
         * \sa svn::Depth
         */
        Depth depth()const;
        //! extra options for diff ("-b", "-w","--ignore-eol-style")
        const svn::StringArray& extra()const;
        //! whether the files will be checked for relatedness.
        bool ignoreAncestry()const;
        //! if true generate diff even the items are marked as binaries
        bool ignoreContentType()const;
        //! if true, no diff output will be generated on deleted files.
        bool noDiffDeleted()const;
        //! first file or folder to diff.
        const Path& path1()const;
        //! second file or folder to diff.
        /*!
         * this is ignored for diff_peg calls
         */
        const Path& path2()const;
        //! peg revision (only used for diff_peg)
        const svn::Revision& peg()const;
        //! if set, all pathes are related to this folder
        /*!
         * Must not be an url! May be empty
         */
        const Path&relativeTo()const;
        //! one of the revisions to check (path1).
        const svn::Revision& rev1()const;
        //! the other revision (path2 for non peg diff).
        const svn::Revision& rev2()const;
        //! prefix for a temporary directory needed by diff.
        /*!
         * Filenames will have ".tmp" and similar added to this prefix in
         * order to ensure uniqueness.
         */
        const Path& tmpPath()const;

        DiffParameter& path1(const Path&path);
        DiffParameter& path2(const Path&path);
        DiffParameter& tmpPath(const Path&path);
        DiffParameter& relativeTo(const Path&path);
        DiffParameter& depth(Depth _depth);
        svn::DiffParameter& changeList(const svn::StringArray&changeList);
        svn::DiffParameter& extra(const svn::StringArray&_extra);
        svn::DiffParameter& ignoreAncestry(bool value);
        svn::DiffParameter& ignoreContentType(bool value);
        svn::DiffParameter& peg(const svn::Revision&_rev);
        svn::DiffParameter& rev1(const svn::Revision&_rev);
        svn::DiffParameter& rev2(const svn::Revision&_rev);
        svn::DiffParameter& noDiffDeleted(bool value);
    };

    struct StatusParameterData;

    class SVNQT_EXPORT StatusParameter
    {
    private:
        SharedPointer<StatusParameterData> _data;
    public:
        StatusParameter(const Path&_path);
        ~StatusParameter();

        //! path to explore
        const Path &path()const;
        StatusParameter&path(const Path&_path);
        //! list specific revision when browsing remote, on working copies parameter will ignored
        const Revision& revision()const;
        StatusParameter&revision(const Revision&rev);
        //! recursion level
        Depth depth()const;
        StatusParameter&depth(Depth d);
        //! Return all entries, not just the interesting ones.
        bool all()const;
        StatusParameter&all(bool getall);
        //! Query the repository for updates.
        bool update()const;
        StatusParameter&update(bool updates);
        //! Disregard default and svn:ignore property ignores.
        bool noIgnore()const;
        StatusParameter&noIgnore(bool noignore);
        //! don't recurse into external definitions
        bool ignoreExternals()const;
        StatusParameter&ignoreExternals(bool noexternals);
        const StringArray& changeList()const;
        StatusParameter&changeList(const StringArray&list);
        //!if on remote listing detailed item info should get if possible
        /*! that may slow so should configureable in frontends!
         */
        bool detailedRemote()const;
        StatusParameter&detailedRemote(bool value);
    };
}


#endif
