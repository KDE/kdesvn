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
 * \file client_parameter.h
 * \brief defining classes working as named parameters for different subversion methods
 *
 * Subversion has various functions which has growing / changing parameter lists from version to version.
 * since subversion 1.4 this changes are more and more unhandy for a c++ wrapper due every time changes to
 * virtual class. This special data containers may reduce changes of signatures to the client interface.
 */

#ifndef CLIENT_PARAMETER_H
#define CLIENT_PARAMETER_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/revision.h"
#include "svnqt/targets.h"
#include "svnqt/path.h"
#include "svnqt/shared_pointer.h"

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
        //! Targets for copy operation
        CopyParameter&srcPath(const Targets&_srcPath);
        //! Destination path for copy operation
        const Path&destination()const;
        //! Destination path for copy operation
        CopyParameter&destination(const Path&destination);

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

        //! set copy/move operation parameter makeParent
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
        DiffParameter& changeList(const svn::StringArray&changeList);
        DiffParameter& extra(const svn::StringArray&_extra);
        DiffParameter& ignoreAncestry(bool value);
        DiffParameter& ignoreContentType(bool value);
        DiffParameter& peg(const svn::Revision&_rev);
        DiffParameter& rev1(const svn::Revision&_rev);
        DiffParameter& rev2(const svn::Revision&_rev);
        DiffParameter& noDiffDeleted(bool value);
        
        //! use gits diff format
        /*!
         * \since subversion  1.7
         * \sa svn_client_diff_peg5,svn_client_diff5
         */
        DiffParameter& git_diff_format(bool value);
        //! use gits diff format
        /*!
         * \since subversion  1.7
         * \sa svn_client_diff_peg5,svn_client_diff5
         */
        bool git_diff_format()const;
        
        //! show copies as new add
        /*!
         * \since subversion  1.7
         * \sa svn_client_diff_peg5,svn_client_diff5
         */
        DiffParameter& copies_as_adds(bool value);
        //! show copies as new add
        /*!
         * \since subversion  1.7
         * \sa svn_client_diff_peg5,svn_client_diff5
         */
        bool copies_as_adds()const;

        
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

    struct LogParameterData;

    class SVNQT_EXPORT LogParameter
    {
    private:
        SharedPointer<LogParameterData> _data;

    public:
        LogParameter();
        ~LogParameter();

        //! items to get the logs for
        const Targets&targets()const;
        //! set items to get the logs for
        LogParameter&targets(const Targets&targets);
        //! range of revisions getting logs for
        /*!
         * when build against subversion prior 1.6 only the first pair is used!
         */
        const RevisionRanges&revisions()const;
        //! set range of revisions getting logs for
        LogParameter&revisions(const RevisionRanges&revisions);
        //! simple start-end range.
        /*!
         * in fact it is the first item in internal revision range. May used when only one pair is required.
         */
        const RevisionRange&revisionRange()const;
        //! set a simple start-end range
        /*!
         * this is useful if only one range is required. This will converted into internal ranges when set.
         */
        LogParameter&revisionRange(const Revision&start,const Revision&end);

        //! the peg revision to use
        const Revision&peg()const;
        //! set the peg revision to use
        LogParameter&peg(const Revision&peg);
        //! if not zero limit logs to this count
        int limit()const;
        LogParameter&limit(int limit);
        bool discoverChangedPathes()const;
        LogParameter&discoverChangedPathes(bool value);
        bool strictNodeHistory()const;
        LogParameter&strictNodeHistory(bool value);
        bool includeMergedRevisions()const;
        LogParameter&includeMergedRevisions(bool value);
        const StringArray& revisionProperties()const;
        LogParameter&revisionProperties(const StringArray&props);
        const StringArray& excludeList()const;
        LogParameter&excludeList(const StringArray&props);
    };

    struct PropertiesParameterData;

    class SVNQT_EXPORT PropertiesParameter
    {
    private:
        SharedPointer<PropertiesParameterData> _data;
    public:
        PropertiesParameter();
        ~PropertiesParameter();

        PropertiesParameter& propertyName(const QString&);
        const QString&propertyName()const;

        PropertiesParameter& propertyValue(const QString&);
        const QString&propertyValue()const;

        //! Old value to check against
        /*!
         * used for revpropset only
         */
        PropertiesParameter& propertyOriginalValue(const QString&);
        const QString&propertyOriginalValue()const;

        //! path or url
        PropertiesParameter& path(const Path&);
        const Path& path()const;

        //! set on revision
        /*! for revpropset it should be a valid Revision, for propset it should be INVALID
         * if url is a local path otherwise it must be a valid revision.
         */
        PropertiesParameter& revision(const Revision&);
        //! set/get on revision
        /*! for revpropset it should be a valid Revision, for propset it should be INVALID
         * if url is a local path otherwise it must be a valid revision.
         */
        const Revision& revision()const;

        //! allow newlines in author property
        /*!
         * used for revprop_set only
         */
        PropertiesParameter& force(bool);
        //! allow newlines in author property
        /*!
         * used for revprop_set only
         */
        bool force()const;

        //! set depth of operation
        /*!
         * used for local propset only
         */
        PropertiesParameter&depth(Depth depth);
        //! depth of operation
        /*!
         * used for local propset only
         */
        Depth depth()const;

        //! set skip check
        /*!
         * used for local propset only
         */
        PropertiesParameter&skipCheck(bool value);
        //! skip check
        /*!
         * used for local propset only
         */
        bool skipCheck()const;

        //! set filter list. if empty no filtering is done
        /*!
         * used for local propset only
         */
        PropertiesParameter&changeList(const StringArray&_list);
        //! filter list. if empty no filtering is done
        /*!
         * used for local propset only
         */
        const StringArray&changeList()const;

        PropertiesParameter&revisionProperties(const PropertiesMap&props);
        const PropertiesMap&revisionProperties()const;
    };

    struct MergeParameterData;

    /**
     * Wrapper for all mergeparameters.
     */
    class SVNQT_EXPORT MergeParameter
    {
    private:
        SharedPointer<MergeParameterData> _data;
    public:
        MergeParameter();
        ~MergeParameter();

        MergeParameter&path1(const Path&path);
        const Path&path1()const;
        MergeParameter&path2(const Path&path);
        const Path&path2()const;
        MergeParameter&localPath(const Path&path);
        const Path&localPath()const;

        /*!
         * used for Client::merge_peg only, when build against subversion prior 1.6 only the first pair is used!
         */
        MergeParameter&peg(const Revision&rev);
        /*!
         * used for Client::merge_peg only or reintegrate merge
         */
        const Revision&peg()const;
        /*!
         * used for Client::merge_peg only, when build against subversion prior 1.6 only the first pair is used!
         */
        MergeParameter&revisions(const RevisionRanges&revs);
        /*!
         * used for Client::merge_peg only, when build against subversion prior 1.6 only the first pair is used!
         */
        const RevisionRanges&revisions()const;
        //! simple start-end range.
        /*!
         * in fact it is the first item in internal revision range. May used when only one pair is required.
         * used for Client::merge, pair is [start,end], with subversion prior 1.6 for Client::merge_peg, too.
         */
        const RevisionRange&revisionRange()const;
        //! set a simple start-end range
        /*!
         * this is useful if only one range is required. This will converted into internal ranges when set.
         * used for Client::merge, pair is [rev1,rev2], with subversion prior 1.6 for Client::merge_peg, too.
         */
        MergeParameter&revisionRange(const Revision&start,const Revision&end);

        //! get start revision
        /*!
         * used for Client::merge. Revision1 is the first item in first pair of Revision ranges
         */
        const Revision&revision1()const;
        //! get end revision
        /*!
         * used for Client::merge. Revision2 is the second item in first pair of Revision ranges
         */
        const Revision&revision2()const;

        MergeParameter&force(bool how);
        bool force()const;
        MergeParameter&notice_ancestry(bool how);
        bool notice_ancestry()const;
        MergeParameter&dry_run(bool how);
        bool dry_run()const;
        MergeParameter&record_only(bool how);
        bool record_only()const;

        MergeParameter&depth(Depth depth);
        Depth depth()const;

        MergeParameter&merge_options(const StringArray&options);
        const StringArray&merge_options()const;

        /**
         * @param reintegrate must be true if this parameter are for a reintegrate merge.
         */
        MergeParameter&reintegrate(bool reintegrate);
        /**
         * Check wheter the parameters are for a reintegrate merge or not. If yes, than parameters are used as follows:
         *   - peg() - setup the required peg revision
         *   - path1() gives the source path
         *   - localPath() the local working copy to merge into
         *   - dry_run() run without real modifications
         *   - merge_options() all other svn options for merge
         *
         * All other parameters are ignored in that case.
         */
        bool reintegrate()const;
    };

    struct CheckoutParameterData;
    //! parameter for Checkout and Export
    class SVNQT_EXPORT CheckoutParameter
    {
    private:
        SharedPointer<CheckoutParameterData> _data;

    public:
        CheckoutParameter();
        ~CheckoutParameter();

        //!name of the module to checkout.
        CheckoutParameter&moduleName(const Path&path);
        //!name of the module to checkout.
        const Path&moduleName()const;
        //!destination directory for checkout.
        CheckoutParameter&destination(const Path&path);
        //!destination directory for checkout.
        const Path&destination()const;
        //!the revision number to checkout.
        /*! If the number is -1
         *  then it will checkout the latest revision.
         */
        CheckoutParameter&revision(const Revision&rev);
        //!the revision number to checkout.
        /*! If the number is -1
         *  then it will checkout the latest revision.
         */
        const Revision&revision()const;
        //! Revision to look up
        CheckoutParameter&peg(const Revision&rev);
        //! Revision to look up
        const Revision&peg()const;
        //! depth of operation
        /*!
         * \sa svn::Depth
         */
        CheckoutParameter&depth(Depth depth);
        //! depth of operation
        /*!
         * \sa svn::Depth
         */
        Depth depth()const;
        //!if true don't process externals definitions.
        CheckoutParameter&ignoreExternals(bool ignore);
        //!if true don't process externals definitions.
        bool ignoreExternals()const;
        //!if true overwrite existing not versioned items.
        CheckoutParameter&overWrite(bool overwrite);
        //!if true overwrite existing not versioned items.
        bool overWrite()const;

        //!Either "LF", "CR" or "CRLF" or QString().
        /*!
         * Used only from Client::doExport, QString() is default (will used as NULL for subversion)
         */
        CheckoutParameter&nativeEol(const QString&native);
        //!Either "LF", "CR" or "CRLF" or QString().
        /*!
         * Used only from Client::doExport, QString() is default (will used as NULL for subversion)
         */
        const QString&nativeEol()const;
    };
}


#endif
