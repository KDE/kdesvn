/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
 * http://kdesvn.alwins-world.de
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not,
 * write to the Free Software Foundation, Inc., 51 Franklin St,
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

#ifndef _SVNCPP_CLIENT_IMPL_H_
#define _SVNCPP_CLIENT_IMPL_H_

#include "svnqt/client.hpp"
#include "svnqt/svnqt_defines.hpp"

// Ignore MSVC 6 compiler warning: debug symbol truncated
#if defined (_MSC_VER) && _MSC_VER <= 1200
#pragma warning (disable: 4786)
#endif

// Ignore MSVC 7, 2005 & 2008 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1550
#pragma warning (disable: 4290)
#endif

class QStringList;

namespace svn
{
  namespace stream {
    class SvnStream;
  }

  /**
   * Subversion client API.
   */
  class SVNQT_NOEXPORT Client_impl:public Client
  {
  public:
    /**
     * Initializes the primary memory pool.
     */
    Client_impl(ContextP context);

    virtual ~Client_impl();

    /**
     * @return returns the Client context
     */
    virtual const ContextP
    getContext () const;

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    virtual void
    setContext (ContextP context);


    /**
     * Enumerates all files/dirs at a given path.
     *
     * Throws an exception if an error occurs
     *
     * @param path Path to explore.
     * @param descend Recurse into subdirectories if existant.
     * @param get_all Return all entries, not just the interesting ones.
     * @param update Query the repository for updates.
     * @param no_ignore Disregard default and svn:ignore property ignores.
     * @param hide_externals don't recurse into external definitions
     * @param revision list specific revision when browsing remote, on working copies parameter will ignored
     * @param detailed_remote if on remote listing detailed item info should get if possible
     *                        that may slow so should configureable in frontends!
     * @return vector with Status entries.
     */
    virtual StatusEntries
    status (const Path& path,
            Depth depth=DepthEmpty,
            bool get_all = true,
            bool update = false,
            bool no_ignore = false,
            const Revision revision = svn::Revision::HEAD,
            bool detailed_remote = false,
            bool hide_externals = false,
            const StringArray & changelists=StringArray()) throw (ClientException);

    /**
     * Returns the status of a single file in the path.
     *
     * Throws an exception if an error occurs
     *
     * @param path File to gather status.
     * @param update if check against repository if new updates are there (for WC only)
     * @param revision list specific revision when browsing remote, on working copies parameter will ignored
     * @return a Status with Statis.isVersioned = FALSE
     */
    virtual StatusPtr
    singleStatus (const Path& path,bool update=false,const Revision revision = svn::Revision::HEAD) throw (ClientException);

  /**
     * Executes a revision checkout.
     * @param moduleName name of the module to checkout.
     * @param destPath destination directory for checkout.
     * @param revision the revision number to checkout. If the number is -1
     *                 then it will checkout the latest revision.
     * @param peg Revision to look up
     * @param recurse whether you want it to checkout files recursively.
     * @param ignore_externals if true don't process externals definitions.
     * @exception ClientException
     */
    virtual svn_revnum_t
    checkout (const Path& moduleName, const Path & destPath,
              const Revision & revision,
              const Revision & peg = Revision::UNDEFINED,
              svn::Depth depth=DepthInfinity,
              bool ignore_externals=false,
              bool overwrite=false
             ) throw (ClientException);

    /**
     * relocate wc @a from to @a to
     * @exception ClientException
     */
    virtual void
    relocate (const Path & path, const QString &from_url,
              const QString &to_url, bool recurse) throw (ClientException);

    /**
     * Sets a single file for deletion.
     * @exception ClientException
     */
    virtual svn::Revision
            remove (const Path & path, bool force,
                    bool keep_local = true,
                    const PropertiesMap&revProps = PropertiesMap()) throw (ClientException);

    /**
     * Sets files for deletion.
     *
     * @param targets targets to delete
     * @param force force if files are locally modified
     * @exception ClientException
     */
    virtual svn::Revision
    remove (const Targets & targets,
            bool force,
            bool keep_local = true,
            const PropertiesMap&revProps = PropertiesMap()) throw (ClientException);

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException
     */
    virtual void
            revert (const Targets & targets,
                    Depth depth,
                    const StringArray&changelist=StringArray()
                   ) throw (ClientException);


    /**
     * Adds a file to the repository.
     * @param path the path to add
     * @param depth if @a path is a folder add items recursive depending on value if it. Pre-subversion 1.5 DepthInfinity is mapped to recursive, all other to not-recursive.
     * @param force if true, do not error on already-versioned items.
     * @param no_ignore if false don't add files or directories that match ignore patterns.
     * @param add_parents if true, go up to the next versioned folder and add all between path and this folder. Used only with subversion 1.5 or newer
     * @exception ClientException
     * @sa svn::Depth
     */
    virtual void add (const Path & path, svn::Depth depth,bool force=false, bool no_ignore=false, bool add_parents = true) throw (ClientException);

    /**
     * Updates the file or directory.
     * @param path targets.
     * @param revision the revision number to checkout.
     *                 Revision::HEAD will checkout the
     *                 latest revision.
     * @param depth Depthness for operation
     * @param ignore_externals ignore externals
     * @param allow_unversioned will operation not fail if there are unversioned items in tree with same name.
     * @exception ClientException
     */
    virtual Revisions
    update (const Targets & path, const Revision & revision,
            Depth depth,bool ignore_externals,bool allow_unversioned,
            bool sticky_depth) throw (ClientException);

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path at @a peg_revision
     *
     * @param path path of file or directory
     * @param peg_revision revision to base the URL
     * @param revision revision to retrieve
     * @param peg_revision Revision to look at
     * @return contents of the file
     */
    virtual QByteArray
    cat (const Path & path,
          const Revision & revision,
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException);
    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path at @a peg_revision
     *
     * @param buffer Stream to store content direct
     * @param path path of file or directory
     * @param peg_revision revision to base the URL
     * @param revision revision to retrieve
     * @exception ClientException
     */
    virtual void
    cat(svn::stream::SvnStream&buffer,
            const Path & path,
            const Revision & revision,
            const Revision & peg_revision) throw (ClientException);

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path at @a peg_revision
     *
     * @param path path of file or directory
     * @param target new (local) name
     * @param peg_revision revision to base the URL
     * @param revision revision to retrieve
     * @param peg_revision Revision to look at
     */
    virtual void
    get (const Path & path,
          const QString  & target,
          const Revision & revision,
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException);

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path and stores the result in @a target
     *
     * @param target the container where to store the result
     * @param path path of file or directory
     * @param revisionStart revision to retrieve
     * @param revisionEnd revision to retrieve
     * @param peg indicates in which revision path is valid
     */
    virtual void
    annotate (AnnotatedFile&target,
              const Path & path,
              const Revision & revisionStart,
              const Revision & revisionEnd,
              const Revision & peg = Revision::UNDEFINED,
              const DiffOptions&diffoptions = DiffOptions(),
              bool ignore_mimetypes = false,
              bool include_merged_revisions = false
             ) throw (ClientException);

    /**
     * Commits changes to the repository. This usually requires
     * authentication, see Auth.
     * @return Returns revision transferred or svn::Revision::UNDEFINED if the revision number is invalid.
     * @param targets files to commit.
     * @param message log message.
     * @param depth whether the operation should be done recursively.
     * @param keep_locks if false unlock items in paths
     * @param changelist
     * @param keep_changelist
     * @exception ClientException
     */
    virtual svn::Revision
    commit (const Targets & targets,
            const QString& message,
            svn::Depth depth,bool keep_locks=true,
            const svn::StringArray&changelist=svn::StringArray(),
            const PropertiesMap&revProps=PropertiesMap(),
            bool keep_changelist=false
           ) throw (ClientException);

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException
     */
    virtual svn::Revision
    copy (const Path & srcPath,
          const Revision & srcRevision,
          const Path & destPath) throw (ClientException);
    /**
     * Copies a versioned file with the history preserved.
     * @since subversion 1.5 api
     * @see svn_client_copy4
     * @exception ClientException
     */
    virtual svn::Revision
    copy (const Targets & srcPath,
          const Revision & srcRevision,
          const Revision & pegRevision,
          const Path & destPath,
          bool asChild=false,bool makeParent=false,const PropertiesMap&revProps=PropertiesMap()) throw (ClientException);

    /**
     * Moves or renames a file.
     * @exception ClientException
     */
    virtual svn::Revision
    move (const Path & srcPath,
          const Path & destPath,
          bool force) throw (ClientException);

    /**
     * Moves or renames a file.
     * @exception ClientException
     */
    virtual svn::Revision
            move (const Targets & srcPath,
                  const Path & destPath,
                  bool force,bool asChild,bool makeParent,const PropertiesMap&revProps=PropertiesMap()) throw (ClientException);

    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth and
     * the callback asks for a logmessage.
     *
     * @param path
     * @param message log message. if it is QString::null asks when working on repository
     * @exception ClientException
     */
    virtual svn::Revision
    mkdir (const Path & path,
           const QString& message,
           bool makeParent=true,
           const PropertiesMap&revProps=PropertiesMap()
          ) throw (ClientException);
    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth and
     * the callback asks for a logmessage.
     *
     * @param targets encoded pathes to create
     * @param message log message. if it is QString::null asks when working on repository
     * @exception ClientException
     */
    virtual svn::Revision
    mkdir (const Targets & targets,
           const QString& message,
           bool makeParent=true,
           const PropertiesMap&revProps=PropertiesMap()
          ) throw (ClientException);

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * @param path a local directory.
     * @exception ClientException
     */
    virtual void
    cleanup (const Path & path) throw (ClientException);

    /**
     * Removes the 'conflicted' state on a file.
     * @exception ClientException
     */
    virtual void resolve (const Path & path,Depth depth,const ConflictResult&resolution=ConflictResult()) throw (ClientException);

    /**
     * Exports the contents of either a subversion repository into a
     * 'clean' directory (meaning a directory with no administrative
     * directories).
     * @exception ClientException
     * @param srcPath source path
     * @param destPath a destination path that must not already exist.
     * @param revision revision to use for the export
     * @param peg the revision where the path is first looked up when exporting from a repository.
     * @param overwrite overwrite existing files
     * @param native_eol Either "LF", "CR" or "CRLF" or NULL.
     * @param ignore_externals don't process externals definitions as part of this operation.
     * @param recurse if true, export recursively. Otherwise, export just the directory represented by from and its immediate non-directory children.
     */
    virtual svn_revnum_t
    doExport (const Path & srcPath,
              const Path & destPath,
              const Revision & revision,
              const Revision & peg = Revision::UNDEFINED,
              bool overwrite=false,
              const QString&native_eol=QString::null,
              bool ignore_externals = false,
              svn::Depth depth=svn::DepthInfinity
             ) throw (ClientException);

    /**
     * Update local copy to mirror a new url. This excapsulates the
     * svn_client_switch() client method.
     * @exception ClientException
     */
    virtual svn_revnum_t
    doSwitch (
              const Path & path, const QString& url,
              const Revision & revision,
              Depth depth,
              const Revision & peg=Revision::UNDEFINED,
              bool sticky_depth = true,
              bool ignore_externals=false,
              bool allow_unversioned=false
             ) throw (ClientException);

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param url
     * @param message log message.
     * @param depth kind of recurse operation
     * @param no_ignore if false, don't add items matching global ignore pattern
     * @param no_unknown_nodetype if true ignore files type not known like pipes or device files
     * @exception ClientException
     */
    virtual svn::Revision
            import (const Path & path, const QString& url,
                    const QString& message,
                    svn::Depth depth,
                    bool no_ignore,bool no_unknown_nodetype,
                    const PropertiesMap&revProps=PropertiesMap()) throw (ClientException);

    /**
     * Merge changes from two paths into a new local path.
     * @exception ClientException
     */
    virtual void
    merge (const Path & path1, const Revision & revision1,
           const Path & path2, const Revision & revision2,
           const Path & localPath, bool force,
           Depth depth,
           bool notice_ancestry=false,
           bool dry_run=false,
           bool record_only=false,
           const StringArray&merge_options=StringArray()
          ) throw (ClientException);

    virtual void
    merge_peg(const Path&src,
              const RevisionRanges&ranges,
                      const Revision&peg,
                      const Path&targetWc,
                      Depth depth,
                      bool notice_ancestry=false,
                      bool dry_run=false,
                      bool force=false,
                      bool record_only=false,
                      const StringArray&merge_options=StringArray()
                     ) throw (ClientException);

    virtual void
    merge_peg(const Path&src,
              const RevisionRange&range,
              const Revision&peg,
              const Path&targetWc,
              Depth depth,
              bool notice_ancestry,
              bool dry_run,
              bool force,
              const StringArray&merge_options
            ) throw (ClientException);

    /**
     * Retrieve information for the given path
     * remote or local. Only gives with subversion 1.2
     * usefull results
     *
     * @param path path for info
     * @param rec recursive (if dir)
     * @param rev for which revision
     * @param peg_revision peg revision
     * @return InfoEntries
     * @since subversion 1.2
     */
    virtual InfoEntries
    info(const Path &path,
         Depth depth,
         const Revision & rev,
         const Revision & peg_revision=svn_opt_revision_unspecified,
         const StringArray&changelists=StringArray()
        ) throw (ClientException);
    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. The first
     * entry  is the youngest revision.
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param path
     * @param revisionStart Start revision.
     * @param revisionEnd End revision
     * @param revisionPeg Revision where path is valid.
     * @param discoverChangedPaths Should changed pathes transferred
     * @param strictNodeHistory
     * @param limit the maximum log entries count.
     * @param include_merged_revisions log information for revisions which have been merged to targets will also be returned. (subversion 1.5)
     * @return a vector with log entries
     */
    virtual LogEntriesPtr
    log (const Path& path, const Revision & revisionStart,
         const Revision & revisionEnd,
         const Revision & revisionPeg,
         bool discoverChangedPaths=false,
         bool strictNodeHistory=true,int limit=0,
         bool include_merged_revisions = false,
         const StringArray&revprops=StringArray()
        ) throw (ClientException);
    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. Result will stored
     * in a map where the key is the revision number
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param path
     * @param revisionStart
     * @param revisionEnd
     * @param revisionPeg Revision where path is valid.
     * @param target the logmap where to store the entries
     * @param discoverChangedPaths
     * @param strictNodeHistory
     * @param limit (ignored when subversion 1.1 API)
     * @return true if success
     */
    virtual bool
    log (const Path& path, const Revision & revisionStart,
         const Revision & revisionEnd,
         LogEntriesMap&target,
         const Revision & revisionPeg,
         bool discoverChangedPaths,
         bool strictNodeHistory,int limit,
         bool include_merged_revisions = false,
         const StringArray&revprops=StringArray()
        ) throw (ClientException);

   /**
     * Produce diff output which describes the delta between
     * @a path/@a revision1 and @a path/@a revision2. @a path
     * can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param tmpPath prefix for a temporary directory needed by diff.
     * Filenames will have ".tmp" and similar added to this prefix in
     * order to ensure uniqueness.
     * @param path path of the file.
     * @param revision1 one of the revisions to check.
     * @param revision2 the other revision.
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated on deleted files.
     * @param ignore_contenttype if true generate diff even the items are marked as binaries
     * @param extra extra options for diff ("-b", "-w","--ignore-eol-style")
     * @return delta between the files
     * @exception ClientException
     */
    virtual QByteArray
            diff_peg (const Path & tmpPath, const Path & path,const Path&relativeTo,
                const Revision & revision1, const Revision & revision2, const Revision& peg_revision,
                Depth depth, bool ignoreAncestry,
                bool noDiffDeleted,bool ignore_contenttype,
                const StringArray&extra,
                const StringArray&changelists
                     )
            throw (ClientException);

    /**
     * Same as other diff but extra options and changelists always set to empty list.
     */
    virtual QByteArray
            diff_peg (const Path & tmpPath, const Path & path,const Path&relativeTo,
                const Revision & revision1, const Revision & revision2, const Revision& peg_revision,
                Depth depth, bool ignoreAncestry,
                bool noDiffDeleted,bool ignore_contenttype)
            throw (ClientException);

    /**
     * Produce diff output which describes the delta between
     * @a path1/@a revision1 and @a path2/@a revision2. @a path2
     * can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param tmpPath prefix for a temporary directory needed by diff.
     * Filenames will have ".tmp" and similar added to this prefix in
     * order to ensure uniqueness.
     * @param path1 first file or folder to diff.
     * @param path2 second file or folder to diff.
     * @param revision1 one of the revisions to check (path1).
     * @param revision2 the other revision (path2).
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated on deleted files.
     * @param ignore_contenttype if true generate diff even the items are marked as binaries
     * @param extra extra options for diff ("-b", "-w","--ignore-eol-style")
     * @return delta between the files
     * @exception ClientException
     */
    virtual QByteArray
            diff (const Path & tmpPath, const Path & path1,const Path & path2,const Path&relativeTo,
                const Revision & revision1, const Revision & revision2,
                Depth depth, bool ignoreAncestry,
                bool noDiffDeleted,bool ignore_contenttype,
                const StringArray&extra,
                const StringArray&changelists
                 )
            throw (ClientException);

    /**
     * Same as other diff but extra options always set to empty list.
     */
    virtual QByteArray
            diff (const Path & tmpPath, const Path & path1,const Path & path2,const Path&relativeTo,
                const Revision & revision1, const Revision & revision2,
                Depth depth, bool ignoreAncestry,
                bool noDiffDeleted,bool ignore_contenttype)
            throw (ClientException);

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * @param pathOrUrl
     * @param revision
     * @param peg at wich revision path exists
     * @param depth @sa svn::Depth
     * @param retrieve_locks check for REPOSITORY locks while listing
     * @return a vector of directory entries, each with
     *         a relative path (only filename)
     */
    virtual DirEntries
    list (const Path& pathOrUrl,
          const Revision& revision,
          const Revision& peg,
          svn::Depth depth,bool retrieve_locks) throw (ClientException);

    /**
     * lists properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @param peg most case should set to @a revision
     * @param recurse
     * @return PropertiesList
     */
    virtual PathPropertiesMapListPtr
    proplist(const Path &path,
             const Revision &revision,
             const Revision &peg,
             Depth depth=DepthEmpty,
             const StringArray&changelists=StringArray());

    /**
     * lists one property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param peg most case should set to @a revision
     * @param recurse
     * @return PathPropertiesMapList
     */
    virtual QPair<QLONG,PathPropertiesMapList>
    propget(const QString& propName,
            const Path &path,
            const Revision &revision,
            const Revision &peg,
            Depth depth = svn::DepthEmpty,
            const StringArray&changelists=StringArray());

    /**
     * set property in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param propName
     * @param propValue
     * @param recurse
     * @param skip_check if true skip validity checks
     * @return PropertiesList
     */
    virtual void
    propset(const QString& propName,
            const QString& propValue,
            const Path &path,
            Depth depth=DepthEmpty,
            bool skip_check=false,
            const Revision&base_revision=Revision::UNDEFINED,
            const StringArray&changelists=StringArray(),
            const PropertiesMap&revProps=PropertiesMap()
           );

    /**
     * delete property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param recurse
     */
    virtual void
    propdel(const QString& propName,
            const Path &path,
            Depth depth=DepthEmpty,
            bool skip_check=false,
            const Revision&base_revision=Revision::UNDEFINED,
            const StringArray&changelists=StringArray());


    /**
     * lists revision properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @return PropertiesList
     */
    virtual QPair<QLONG,PropertiesMap>
    revproplist(const Path &path,
                const Revision &revision);

    /**
     * lists one revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @return PropertiesList
     */
    QPair<QLONG,QString>
    revpropget(const QString& propName,
               const Path &path,
               const Revision &revision);

    /**
     * set revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param propValue
     * @param path
     * @param revision
     * @param force
     * @return Revision
     */
    virtual QLONG
    revpropset(const QString& propName,
               const QString& propValue,
               const Path &path,
               const Revision &revision,
               bool force=false);

    /**
     * delete revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param force
     * @return Revision
     */
    virtual QLONG
    revpropdel(const QString& propName,
               const Path &path,
               const Revision &revision,
               bool force=false);

  /**
   * lock files in repository or working copy
   * @param targets items to be locked
   * @param message if non null stored with each lock in repository
   * @param steal_lock if true locks in wc will stolen.
   * @since subversion 1.2
   */
   virtual void
   lock (const Targets & targets,
        const QString& message,
        bool steal_lock) throw (ClientException);
    /**
     * unlock files in repository or working copy
     * @param targets items to unlock
     * @param break_lock ignore any errors
     */
    virtual void
    unlock (const Targets&targets,
            bool break_lock) throw (ClientException);

    virtual void
    url2Revision(const QString&revstring,
        Revision&start,Revision&end);
    virtual void
    url2Revision(const QString&revstring,
            Revision&start);

    struct sBaton {
        Context*m_context;
        void*m_data;
        void*m_revstack;
    };

    struct propBaton {
        Context*m_context;
        PathPropertiesMapList*resultlist;
    };

  private:
    ContextP m_context;

    /**
     * disallow assignment operator
     */
    Client_impl & operator= (const Client &);

    /**
     * disallow copy constructor
     */
    Client_impl (const Client &);

    DirEntries
    list_simple(const Path& pathOrUrl,
          const Revision& revision,
          const Revision& peg,
          bool recurse) throw (ClientException);
    DirEntries
    list_locks(const Path& pathOrUrl,
          const Revision& revision,
          const Revision& peg,
          bool recurse) throw (ClientException);

    svn_error_t * internal_cat(const Path & path,
                const Revision & revision,
                const Revision & peg_revision,
                svn::stream::SvnStream&);

    apr_hash_t * map2hash(const PropertiesMap&,const Pool&);
  };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
