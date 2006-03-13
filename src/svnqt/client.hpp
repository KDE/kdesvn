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

// Ignore MSVC 7 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1310
#pragma warning (disable: 4290)
#endif

#include "svncpp_defines.hpp"

// qt
#include <qglobal.h>

#if QT_VERSION < 0x040000
    #include <qstring.h>
    #include <qpair.h>
    #include <qvaluelist.h>
    #include <qmap.h>
#else
    #include <QtCore>
#endif

// svncpp
#include "context.hpp"
#include "exception.hpp"
#include "path.hpp"
#include "entry.hpp"
#include "revision.hpp"
#include "log_entry.hpp"
#include "info_entry.hpp"
#include "annotate_line.hpp"


namespace svn
{
  // forward declarations
  class Context;
  class Status;
  class Targets;
  class DirEntry;

#if QT_VERSION < 0x040000
  typedef QValueList<LogEntry> LogEntries;
  typedef QValueList<InfoEntry> InfoEntries;
  typedef QValueList<Status> StatusEntries;
  typedef QValueList<DirEntry> DirEntries;
  typedef QValueList<AnnotateLine> AnnotatedFile;
  typedef QValueList<Revision> Revisions;
#else
  typedef QList<LogEntry> LogEntries;
  typedef QList<InfoEntry> InfoEntries;
  typedef QList<Status> StatusEntries;
  typedef QList<DirEntry> DirEntries;
  typedef QList<AnnotateLine> AnnotatedFile;
  typedef QList<Revision> Revisions;
#endif

  // map of logentries - key is revision
  typedef QMap<long,LogEntry> LogEntriesMap;
  // map of property names to values
  typedef QMap<QString,QString> PropertiesMap;
  // pair of path, PropertiesMap
  typedef QPair<QString, PropertiesMap> PathPropertiesMapEntry;
  // vector of path, Properties pairs
#if QT_VERSION < 0x040000
  typedef QValueList<PathPropertiesMapEntry> PathPropertiesMapList;
#else
  typedef QList<PathPropertiesMapEntry> PathPropertiesMapList;
#endif

  /**
   * Subversion client API.
   */
  SVNQT_EXPORT class Client
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
    virtual const Context *
    getContext () const = 0;

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    virtual void
    setContext (Context * context = NULL) = 0;

    /**
     * get a real instance. Result must cleaned with delete.
     * \param context The context to use
     * \param subtype the wanted implementation - this moment only 0 allowed.
     * \return an instance of client or 0L if error.
     */
     static Client*getobject(Context * context = 0,int subtype=0);

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
    status (const QString& path,
            const bool descend = false,
            const bool get_all = true,
            const bool update = false,
            const bool no_ignore = false,
            Revision revision = svn::Revision::HEAD,
            bool detailed_remote = false,
            const bool hide_externals = false) throw (ClientException) = 0;

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
    virtual Status
    singleStatus (const QString& path,bool update=false,Revision revision = svn::Revision::HEAD) throw (ClientException)=0;

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
    checkout (const QString& moduleName, const Path & destPath,
              const Revision & revision,
              const Revision & peg = Revision::UNDEFINED,
              bool recurse = true,
              bool ignore_externals=false) throw (ClientException) = 0;

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
    virtual void
    remove (const Path & path, bool force) throw (ClientException)=0;

    /**
     * Sets files for deletion.
     *
     * @param targets targets to delete
     * @param force force if files are locally modified
     * @exception ClientException
     */
    virtual void
    remove (const Targets & targets,
            bool force) throw (ClientException) = 0;

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException
     */
    virtual void
    revert (const Targets & targets, bool recurse) throw (ClientException)=0;


    /**
     * Adds a file to the repository.
     * @exception ClientException
     */
    virtual void
    add (const Path & path, bool recurse) throw (ClientException)=0;

    /**
     * Updates the file or directory.
     * @param path targets.
     * @param revision the revision number to checkout.
     *                 Revision::HEAD will checkout the
     *                 latest revision.
     * @param recurse recursively update.
     * @param ignore_external ignore externals (only when Subversion 1.2 or above)
     * @exception ClientException
     */
    virtual Revisions
    update (const Targets & path, const Revision & revision,
            bool recurse,bool ignore_externals) throw (ClientException) = 0;

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
     * a @a path and saves it to the destination file @a dstPath.
     *
     * If @a dstPath is empty (""), then this path will be
     * constructed from the temporary directory on this system
     * and the filename in @a path. @a dstPath will still have
     * the file extension from @a path and uniqueness of the
     * temporary filename will be ensured.
     *
     * @param dstPath Filename in which the contents
     *                of the file file will be safed.
     * @param path path or url
     * @param revision
     */
    virtual void
    get (Path & dstPath, const Path & path,
         const Revision & revision) throw (ClientException) = 0;


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
              const Revision & peg = Revision::UNDEFINED) throw (ClientException)=0;

    /**
     * Commits changes to the repository. This usually requires
     * authentication, see Auth.
     * @return Returns a long representing the revision. It returns a
     *         -1 if the revision number is invalid.
     * @param targets files to commit.
     * @param message log message.
     * @param recurse whether the operation should be done recursively.
     * @exception ClientException
     */
    virtual svn_revnum_t
    commit (const Targets & targets,
            const QString& message,
            bool recurse) throw (ClientException)=0;

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException
     */
    virtual void
    copy (const Path & srcPath,
          const Revision & srcRevision,
          const Path & destPath) throw (ClientException)=0;

    /**
     * Moves or renames a file.
     * @exception ClientException
     */
    virtual void
    move (const Path & srcPath,
          const Path & destPath,
          bool force) throw (ClientException)=0;

    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth.
     *
     * @param path
     * @param message log message.
     * @exception ClientException
     */
    virtual void
    mkdir (const Path & path,
           const QString& message) throw (ClientException)=0;
    virtual void
    mkdir (const Targets & targets,
           const QString& message) throw (ClientException)=0;

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
    virtual void
    resolved (const Path & path, bool recurse) throw (ClientException)=0;

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
              bool recurse = true
              ) throw (ClientException)=0;

    /**
     * Update local copy to mirror a new url. This excapsulates the
     * svn_client_switch() client method.
     * @exception ClientException
     */
    virtual svn_revnum_t
    doSwitch (const Path & path, const QString& url,
              const Revision & revision,
              bool recurse) throw (ClientException)=0;

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param url
     * @param message log message.
     * @param recurse
     * @exception ClientException
     */
    virtual void
    import (const Path & path, const QString& url,
            const QString& message,
            bool recurse) throw (ClientException)=0;

    /**
     * Merge changes from two paths into a new local path.
     * @exception ClientException
     */
    virtual void
    merge (const Path & path1, const Revision & revision1,
           const Path & path2, const Revision & revision2,
           const Path & localPath, bool force,
           bool recurse,
           bool notice_ancestry=false,
           bool dry_run=false) throw (ClientException)=0;

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
    info(const QString &path,
          bool rec,
          const Revision & rev,
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException)=0;
    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. The first
     * entry  is the youngest revision.
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param path
     * @param revisionStart
     * @param revisionEnd
     * @param discoverChangedPaths
     * @param strictNodeHistory
     * @param limit (ignored when subversion 1.1 API)
     * @return a vector with log entries
     */
    virtual const LogEntries *
    log (const QString& path, const Revision & revisionStart,
         const Revision & revisionEnd,
         bool discoverChangedPaths=false,
         bool strictNodeHistory=true,int limit = 0) throw (ClientException)=0;

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
     * @param target the logmap where to store the entries
     * @param discoverChangedPaths
     * @param strictNodeHistory
     * @param limit (ignored when subversion 1.1 API)
     * @return true if success
     */
    virtual bool
    log (const QString& path, const Revision & revisionStart,
         const Revision & revisionEnd,
         LogEntriesMap&target,
         bool discoverChangedPaths=false,
         bool strictNodeHistory=true,int limit = 0) throw (ClientException)=0;

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
     * @param noDiffDeleted if true, no diff output will be generated
     * on deleted files.
     * @return delta between the files
     * @exception ClientException
     */
    virtual QString
    diff (const Path & tmpPath, const Path & path,
          const Revision & revision1, const Revision & revision2,
          const bool recurse, const bool ignoreAncestry,
          const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)=0;
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
     * @param path path of the file.
     * @param revision1 one of the revisions to check.
     * @param revision2 the other revision.
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated
     * on deleted files.
     * @return delta between the files
     * @exception ClientException
     */
    virtual QString
    diff (const Path & tmpPath, const Path & path1,const Path & path2,
          const Revision & revision1, const Revision & revision2,
          const bool recurse, const bool ignoreAncestry,
          const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)=0;

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * If checking for locks is activated, it lists the locks inside repository, not locks inside
     * working copy!
     * @param pathOrUrl
     * @param revision
     * @param recurse
     * @param retrieve_locks check for REPOSITORY locks while listing.
     * @return a vector of directory entries, each with
     *         a relative path (only filename)
     */
    virtual DirEntries
    list (const QString& pathOrUrl,
          Revision& revision,
          bool recurse,bool retrieve_locks) throw (ClientException)=0;

    /**
     * lists properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @param recurse
     * @return PropertiesList
     */
    virtual PathPropertiesMapList
    proplist(const Path &path,
             const Revision &revision,
             bool recurse=false)=0;

    /**
     * lists one property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param recurse
     * @return PathPropertiesMapList
     */
    virtual PathPropertiesMapList
    propget(const QString& propName,
            const Path &path,
            const Revision &revision,
            bool recurse=false) = 0;

    /**
     * set property in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @param propName
     * @param propValue
     * @param recurse
     * @return PropertiesList
     */
    virtual void
    propset(const QString& propName,
            const QString& propValue,
            const Path &path,
            const Revision &revision,
            bool recurse=false) = 0;

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
            const Revision &revision,
            bool recurse=false)=0;


    /**
     * lists revision properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @return PropertiesList
     */
    virtual QPair<svn_revnum_t,PropertiesMap>
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
    virtual QPair<svn_revnum_t,QString>
    revpropget(const QString& propName,
               const Path &path,
               const Revision &revision)=0;

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
    virtual svn_revnum_t
    revpropset(const QString& propName,
               const QString& propValue,
               const Path &path,
               const Revision &revision,
               bool force=false)=0;

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
    virtual svn_revnum_t
    revpropdel(const QString& propName,
               const Path &path,
               const Revision &revision,
               bool force=false) = 0;

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
