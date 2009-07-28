/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht (ral@alwins-world.de)
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

#ifndef _SVNCPP_CLIENT_H_
#define _SVNCPP_CLIENT_H_

// Ignore MSVC 6 compiler warning: debug symbol truncated
#if defined (_MSC_VER) && _MSC_VER <= 1200
#pragma warning (disable: 4786)
#endif

// Ignore MSVC 7, 2005 & 2008 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1550
#pragma warning (disable: 4290)
#endif

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/svnstream.hpp"

// qt
#include <qglobal.h>
#include <QtCore>

// svnqt
#include "svnqt/context.hpp"
#include "svnqt/exception.hpp"
#include "svnqt/path.hpp"
#include "svnqt/entry.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/log_entry.hpp"
#include "svnqt/info_entry.hpp"
#include "svnqt/annotate_line.hpp"
#include "svnqt/stringarray.hpp"
#include "svnqt/diffoptions.hpp"
#include "svnqt/conflictresult.hpp"

class QStringList;

namespace svn
{
  /** Subversion client API.
   *
   * Never use an object of this as global static! This will make problems with subversion
   * initialize.
   */
  class SVNQT_EXPORT Client
  {
  public:

    /**
     * Initializes the primary memory pool.
     */
    Client();

    virtual ~Client ();


    /**
     * @return returns the Client context
     */
    virtual const ContextP
    getContext () const = 0;

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    virtual void
    setContext (const ContextP&context) = 0;

    /**
     * get a real instance. Result must cleaned with delete.
     * \param context The context to use
     * \param subtype the wanted implementation - this moment only 0 allowed.
     * \return an instance of client or 0L if error.
     */
     static Client*getobject(ContextP context,int subtype=0);

    /**
     * Enumerates all files/dirs at a given path.
     *
     * Throws an exception if an error occurs
     *
     * @param params the parameter for this method
     * @return vector with Status entries.
     */
    virtual StatusEntries
    status (const StatusParameter&params) throw (ClientException) = 0;

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
    singleStatus (const Path& path,bool update=false,const Revision revision = svn::Revision::HEAD) throw (ClientException)=0;

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
             ) throw (ClientException) = 0;

    /**
     * relocate wc @a from to @a to
     * @exception ClientException
     */
    virtual void
    relocate (const Path & path, const QString &from_url,
              const QString &to_url, bool recurse) throw (ClientException)=0;

    /**
     * Sets a single file for deletion.
     * @exception ClientException
     */
    virtual svn::Revision
            remove (const Path & path, bool force,
                    bool keep_local = true,
                    const PropertiesMap&revProps = PropertiesMap()) throw (ClientException)=0;

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
            bool keep_local=true,
            const PropertiesMap&revProps=PropertiesMap()) throw (ClientException) = 0;

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException
     */
    virtual void
    revert (const Targets & targets,
            Depth depth,
            const StringArray&changelist=StringArray()
           ) throw (ClientException)=0;


    /**
     * Adds a file to the repository.
     * @param path the path to add
     * @param depth if @a path is a folder add items recursive depending on value if it.
     * @param force if true, do not error on already-versioned items.
     * @param no_ignore if false don't add files or directories that match ignore patterns. When build against svn 1.2 always false
     * @param add_parents if true, go up to the next versioned folder and add all between path and this folder.
     * @exception ClientException
     */
    virtual void
    add (const Path & path, svn::Depth depth,bool force=false, bool no_ignore=false, bool add_parents = true) throw (ClientException)=0;

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
            bool sticky_depth) throw (ClientException) = 0;

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path at @a peg_revision
     *
     * @param path path of file or directory
     * @param peg_revision revision to base the URL
     * @param revision revision to retrieve
     * @return contents of the file
     */
    virtual QByteArray
    cat (const Path & path,
          const Revision & revision,
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException)=0;
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
            const Revision & peg_revision) throw (ClientException)=0;
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
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException)=0;

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
             ) throw (ClientException)=0;

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
            const svn::StringArray&contents=svn::StringArray(),
            const PropertiesMap&revProps=PropertiesMap(),
            bool keep_changelist=false
           ) throw (ClientException)=0;

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException
     */
    virtual svn::Revision
    copy (const Path & srcPath,
          const Revision & srcRevision,
          const Path & destPath) throw (ClientException)=0;
    /**
     * Copies a versioned file with the history preserved.
     * @since subversion 1.5 api
     * @see svn_client_copy4
     * @exception ClientException
     */
    virtual svn::Revision
    copy (const CopyParameter&parameter) throw (ClientException)=0;

    /**
     * Moves or renames a file.
     * @exception ClientException
     */
    virtual svn::Revision
            move (const CopyParameter&parameter) throw (ClientException)=0;

    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth and
     * the callback asks for a logmessage. With subversion 1.4 the target
     * must not exist (\sa svn_client_move4)
     *
     * @param path
     * @param message log message. if it is QString::null asks when working on repository
     * @param makeParent create parent folders if not existant (only when build with svn 1.5 or above)
     * @exception ClientException
     */
    virtual svn::Revision
    mkdir (const Path & path,
           const QString& message,
           bool makeParent=true,
           const PropertiesMap&revProps=PropertiesMap()
          ) throw (ClientException)=0;
    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth and
     * the callback asks for a logmessage.
     *
     * @param targets encoded pathes to create
     * @param message log message. if it is QString::null asks when working on repository
     * @param makeParent create parent folders if not existant (only when build with svn 1.5 or above)
     * @exception ClientException
     */
    virtual svn::Revision
    mkdir (const Targets & targets,
           const QString& message,
           bool makeParent=true,
           const PropertiesMap&revProps=PropertiesMap()
          ) throw (ClientException)=0;

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * @param path a local directory.
     * @exception ClientException
     */
    virtual void
    cleanup (const Path & path) throw (ClientException)=0;

    /**
     * Removes the 'conflicted' state on a file.
     * @exception ClientException
     */
    virtual void resolve (const Path & path,Depth depth,const ConflictResult&resolution=ConflictResult()) throw (ClientException)=0;

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
     * @param recurse if true, export recursively.<br>
      Otherwise, export just the directory represented by from and its immediate non-directory children.
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
              ) throw (ClientException)=0;

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
             ) throw (ClientException)=0;

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param url
     * @param message log message.
     * @param depth kind of recurse operation
     * @param no_ignore if false, don't add items matching global ignore pattern (@since subversion 1.3)
     * @param no_unknown_nodetype if true ignore files type not known like pipes or device files (@since subversion 1.5)
     * @exception ClientException
     */
    virtual svn::Revision
    import (const Path & path, const QString& url,
            const QString& message,
            svn::Depth depth,
            bool no_ignore,bool no_unknown_nodetype,
            const PropertiesMap&revProps=PropertiesMap()) throw (ClientException)=0;

    /**
     * Merge changes from two paths into a new local path.
     * @exception ClientException
     */
    virtual void
    merge (const MergeParameter&parameters) throw (ClientException)=0;

    virtual void
    merge_peg(const MergeParameter&parameters) throw (ClientException)=0;

    /**
     * Retrieve information for the given path
     * remote or local.
     *
     * @param path path for info
     * @param rec recursive (if dir)
     * @param rev for which revision
     * @param peg_revision peg revision
     * @return InfoEntries
     */
    virtual InfoEntries
    info (const Path &path,
          Depth depth,
          const Revision & rev,
          const Revision & peg_revision=svn_opt_revision_unspecified,
          const StringArray&changelists=StringArray()
         ) throw (ClientException)=0;

    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. Result will stored
     * in a map where the key is the revision number
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param params Parameter to use for log
     * @param target where to store the resulting logs
     * @return true if success
     * @sa LogParameter
     */
    virtual bool
    log (const LogParameter&params,LogEntriesMap&target) throw (ClientException)=0;

    /**
     * Produce diff output which describes the delta between
     * @a path/@a revision1 and @a path/@a revision2. @a path
     * can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param options set of options required for diff
     * @return delta between the files
     * @exception ClientException
     */
    virtual QByteArray
            diff_peg(const DiffParameter&options) throw (ClientException)=0;

    /**
     * Produce diff output which describes the delta between
     * @a path1/@a revision1 and @a path2/@a revision2. @a path2
     * can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param options set of options required for diff
     * @return delta between the files
     * @exception ClientException
     */
    virtual QByteArray
            diff (const DiffParameter&options) throw (ClientException)=0;

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * If checking for locks is activated, it lists the locks inside repository, not locks inside
     * working copy!
     * @param pathOrUrl
     * @param revision
     * @param peg at wich revision path exists
     * @param depth @sa depth
     * @param retrieve_locks check for REPOSITORY locks while listing.
     * @return a vector of directory entries, each with
     *         a relative path (only filename). In subversion >= 1.4 an entry without a name is returned, too. This
     *         is the searched directory (done in subversion itself)
     */
    virtual DirEntries
    list (const Path& pathOrUrl,
          const Revision& revision,
          const Revision& peg,
          svn::Depth depth,bool retrieve_locks) throw (ClientException)=0;

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
             const StringArray&changelists=StringArray()
            )=0;

    /**
     * lists one property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param peg most case should set to @a revision
     * @param recurse
     * @return PathPropertiesMapList and revision where the properties are taken from (svn 1.5) or undefined revision (prior 1.5)
     */
    virtual QPair<QLONG,PathPropertiesMapList>
    propget(const QString& propName,
            const Path &path,
            const Revision &revision,
            const Revision &peg,
            Depth depth = svn::DepthEmpty,
            const StringArray&changelists=StringArray()) = 0;

    /**
     * set property in @a path no matter whether local or
     * repository
     *
     * @param params svn::PropertiesParamter holding required values.
     * Following is used:<br>
     * <ul>
     * <li> svn::PropertiesParameter::propertyName()
     * <li> svn::PropertiesParameter::propertyValue()
     * <li> svn::PropertiesParameter::depth()
     * <li> svn::PropertiesParameter::skipCheck()
     * <li> svn::PropertiesParameter::revision()
     * <li> svn::PropertiesParameter::changeList()
     * <li> svn::PropertiesParameter::revisionProperties()
     * </ul>
     */
    virtual void
    propset(const PropertiesParameter&params)=0;

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
                const Revision &revision)=0;

    /**
     * lists one revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @return PropertiesList
     */
    virtual QPair<QLONG,QString>
    revpropget(const QString& propName,
               const Path &path,
               const Revision &revision)=0;

    /**
     * set revision property in @a path no matter whether local or
     * repository
     *
     * @param params parameter to use
     * @return Revision
     * @sa PropertiesParameter
     */
    virtual QLONG
    revpropset(const PropertiesParameter&params)=0;

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
               const Revision &revision) = 0;

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
        bool steal_lock) throw (ClientException)=0;
    /**
     * unlock files in repository or working copy
     * @param targets items to unlock
     * @param break_lock ignore any errors
     */
    virtual void
    unlock (const Targets&targets,
            bool break_lock) throw (ClientException)=0;

    virtual void
    url2Revision(const QString&revstring,
        Revision&start,Revision&end)=0;
    virtual void
    url2Revision(const QString&revstring,
            Revision&start)=0;

    virtual bool RepoHasCapability(const Path&repository,Capability capability)=0;

  private:
    /**
     * disallow assignment operator
     */
    Client & operator= (const Client &);
    /**
     * disallow copy constructor
     */
    Client (const Client &);
  };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
