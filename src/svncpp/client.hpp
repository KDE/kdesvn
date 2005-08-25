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


// qt
#include <qstring.h>
#include <qpair.h>
#include <qvaluelist.h>
#include <qmap.h>

// svncpp
#include "svncpp/context.hpp"
#include "svncpp/exception.hpp"
#include "svncpp/path.hpp"
#include "svncpp/entry.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/log_entry.hpp"
#include "svncpp/info_entry.hpp"
#include "svncpp/annotate_line.hpp"


namespace svn
{
  // forward declarations
  class Context;
  class Status;
  class Targets;
  class DirEntry;

  typedef QValueList<LogEntry> LogEntries;
  typedef QValueList<InfoEntry> InfoEntries;
  typedef QValueList<Status> StatusEntries;
  typedef QValueList<DirEntry> DirEntries;
  typedef QValueList<AnnotateLine> AnnotatedFile;

  // map of property names to values
  typedef QMap<QString,QString> PropertiesMap;
  // pair of path, PropertiesMap
  typedef QPair<QString, PropertiesMap> PathPropertiesMapEntry;
  // vector of path, Properties pairs
  typedef QValueList<PathPropertiesMapEntry> PathPropertiesMapList;

  /**
   * Subversion client API.
   */
  class Client
  {
  public:
    /**
     * Initializes the primary memory pool.
     */
    Client (Context * context = 0);

    virtual ~Client ();


    /**
     * @return returns the Client context
     */
    const Context *
    getContext () const;

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    void
    setContext (Context * context = NULL);

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
     * @return vector with Status entries.
     */
    StatusEntries
    status (const QString& path,
            const bool descend = false,
            const bool get_all = true,
            const bool update = false,
            const bool no_ignore = false) throw (ClientException);

    /**
     * Returns the status of a single file in the path.
     *
     * Throws an exception if an error occurs
     *
     * @param path File to gather status.
     * @param update if check against repository if new updates are there (for WC only)
     * @return a Status with Statis.isVersioned = FALSE
     */
    Status
    singleStatus (const QString& path,bool update=false) throw (ClientException);

  /**
     * Executes a revision checkout.
     * @param moduleName name of the module to checkout.
     * @param destPath destination directory for checkout.
     * @param revision the revision number to checkout. If the number is -1
     *                 then it will checkout the latest revision.
     * @param recurse whether you want it to checkout files recursively.
     * @exception ClientException
     */
    svn_revnum_t
    checkout (const QString& moduleName, const Path & destPath,
              const Revision & revision,
              bool recurse) throw (ClientException);

    /**
     * relocate wc @a from to @a to
     * @exception ClientException
     */
    void
    relocate (const Path & path, const QString &from_url,
              const QString &to_url, bool recurse) throw (ClientException);

    /**
     * Sets a single file for deletion.
     * @exception ClientException
     */
    void
    remove (const Path & path, bool force) throw (ClientException);

    /**
     * Sets files for deletion.
     *
     * @param targets targets to delete
     * @param force force if files are locally modified
     * @exception ClientException
     */
    void
    remove (const Targets & targets,
            bool force) throw (ClientException);

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException
     */
    void
    revert (const Targets & targets, bool recurse) throw (ClientException);


    /**
     * Adds a file to the repository.
     * @exception ClientException
     */
    void
    add (const Path & path, bool recurse) throw (ClientException);

    /**
     * Updates the file or directory.
     * @param path target file.
     * @param revision the revision number to checkout.
     *                 Revision::HEAD will checkout the
     *                 latest revision.
     * @param recurse recursively update.
     * @exception ClientException
     */
    svn_revnum_t
    update (const Path & path, const Revision & revision,
            bool recurse) throw (ClientException);

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path
     *
     * @param path path of file or directory
     * @param revision revision to retrieve
     * @return contents of the file
     */
    QByteArray
    cat (const Path & path,
         const Revision & revision) throw (ClientException);


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
    void
    get (Path & dstPath, const Path & path,
         const Revision & revision) throw (ClientException);


    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path
     *
     * @param path path of file or directory
     * @param revisionStart revision to retrieve
     * @param revisionEnd revision to retrieve
     * @return contents of the file
     */
    AnnotatedFile *
    annotate (const Path & path,
              const Revision & revisionStart,
              const Revision & revisionEnd) throw (ClientException);

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
    svn_revnum_t
    commit (const Targets & targets,
            const QString& message,
            bool recurse) throw (ClientException);

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException
     */
    void
    copy (const Path & srcPath,
          const Revision & srcRevision,
          const Path & destPath) throw (ClientException);

    /**
     * Moves or renames a file.
     * @exception ClientException
     */
    void
    move (const Path & srcPath,
          const Revision & srcRevision,
          const Path & destPath,
          bool force) throw (ClientException);

    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth.
     *
     * @param path
     * @param message log message.
     * @exception ClientException
     */
    void
    mkdir (const Path & path,
           const QString& message) throw (ClientException);
    void
    mkdir (const Targets & targets,
           const QString& message) throw (ClientException);

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * @param path a local directory.
     * @exception ClientException
     */
    void
    cleanup (const Path & path) throw (ClientException);

    /**
     * Removes the 'conflicted' state on a file.
     * @exception ClientException
     */
    void
    resolved (const Path & path, bool recurse) throw (ClientException);

    /**
     * Exports the contents of either a subversion repository into a
     * 'clean' directory (meaning a directory with no administrative
     * directories).
     * @exception ClientException
     * @param srcPath source path
     * @param destPath a destination path that must not already exist.
     * @param revision revision to use for the export
     * @param force force export
     */
    svn_revnum_t
    doExport (const Path & srcPath,
              const Path & destPath,
              const Revision & revision,
              bool force=false) throw (ClientException);

    /**
     * Update local copy to mirror a new url. This excapsulates the
     * svn_client_switch() client method.
     * @exception ClientException
     */
    svn_revnum_t
    doSwitch (const Path & path, const QString& url,
              const Revision & revision,
              bool recurse) throw (ClientException);

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param url
     * @param message log message.
     * @param recurse
     * @exception ClientException
     */
    void
    import (const Path & path, const QString& url,
            const QString& message,
            bool recurse) throw (ClientException);

    /**
     * Merge changes from two paths into a new local path.
     * @exception ClientException
     */
    void
    merge (const Path & path1, const Revision & revision1,
           const Path & path2, const Revision & revision2,
           const Path & localPath, bool force,
           bool recurse,
           bool notice_ancestry=false,
           bool dry_run=false) throw (ClientException);

    /**
     * Retrieve information for the given path
     * from the wc
     *
     * @param path
     * @return Entry
     */
    Entry
    info (const QString& path );
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
    InfoEntries
    info2(const QString &path,
          bool rec,
          const Revision & rev,
          const Revision & peg_revision=svn_opt_revision_unspecified) throw (ClientException);
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
     * @return a vector with log entries
     */
    const LogEntries *
    log (const QString& path, const Revision & revisionStart,
         const Revision & revisionEnd,
         bool discoverChangedPaths=false,
         bool strictNodeHistory=true) throw (ClientException);

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
    QString
    diff (const Path & tmpPath, const Path & path,
          const Revision & revision1, const Revision & revision2,
          const bool recurse, const bool ignoreAncestry,
          const bool noDiffDeleted) throw (ClientException);

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * @param pathOrUrl
     * @param revision
     * @param recurse
     * @return a vector of directory entries, each with
     *         a relative path (only filename)
     */
    DirEntries
    list (const QString& pathOrUrl,
          svn_opt_revision_t * revision,
          bool recurse) throw (ClientException);

    /**
     * lists properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @param recurse
     * @return PropertiesList
     */
    PathPropertiesMapList
    proplist(const Path &path,
             const Revision &revision,
             bool recurse=false);

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
    PathPropertiesMapList
    propget(const QString& propName,
            const Path &path,
            const Revision &revision,
            bool recurse=false);

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
    void
    propset(const QString& propName,
            const QString& propValue,
            const Path &path,
            const Revision &revision,
            bool recurse=false);

    /**
     * delete property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param recurse
     */
    void
    propdel(const QString& propName,
            const Path &path,
            const Revision &revision,
            bool recurse=false);


    /**
     * lists revision properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @return PropertiesList
     */
    QPair<svn_revnum_t,PropertiesMap>
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
    QPair<svn_revnum_t,QString>
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
    svn_revnum_t
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
    svn_revnum_t
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
   void
   lock (const Targets & targets,
        const QString& message,
        bool steal_lock) throw (ClientException);
    /**
     * unlock files in repository or working copy
     * @param targets items to unlock
     * @param break_lock ignore any errors
     */
    void
    unlock (const Targets&targets,
            bool break_lock) throw (ClientException);

  private:
    Context * m_context;

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
