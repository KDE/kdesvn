/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * https://kde.org/applications/development/org.kde.kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
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

#ifndef SVNQT_CLIENT_IMPL_H
#define SVNQT_CLIENT_IMPL_H

#include <svnqt/client.h>
#include <svnqt/svnqt_defines.h>

class QStringList;

namespace svn
{
namespace stream
{
class SvnStream;
}

/**
 * Subversion client API.
 */
class SVNQT_NOEXPORT Client_impl: public Client
{
public:
    /**
     * Initializes the primary memory pool.
     */
    explicit Client_impl(const ContextP &context);

    virtual ~Client_impl();

    /**
     * @return returns the Client context
     */
    const ContextP getContext() const override;

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    void setContext(const ContextP &context) override;


    /**
     * Enumerates all files/dirs at a given path.
     *
     * Throws an exception if an error occurs
     *
     * @param params the parameter for this method
     * @return vector with Status entries.
     */
    StatusEntries status(const StatusParameter &params) override;

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
    StatusPtr singleStatus(const Path &path, bool update = false, const Revision &revision = svn::Revision::HEAD) override;

    /**
     * Executes a revision checkout.
     * @param params the parameters to use
     * @return revision number checked out
     * @exception ClientException
     */
    Revision checkout(const CheckoutParameter &params) override;

    /**
     * relocate wc @a from to @a to
     * @exception ClientException
     */
    void relocate(const Path &path, const Url &from_url,
                  const Url &to_url, bool recurse, bool ignore_externals) override;

    /**
     * Sets files for deletion.
     *
     * @param targets targets to delete
     * @param force force if files are locally modified
     * @exception ClientException
     */
    svn::Revision
    remove(const Targets &targets,
           bool force,
           bool keep_local = true,
           const PropertiesMap &revProps = PropertiesMap()) override;

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException
     */
    void
    revert(const Targets &targets,
           Depth depth,
           const StringArray &changelist = StringArray()
          ) override;


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
    void add(const Path &path, svn::Depth depth, bool force = false, bool no_ignore = false, bool add_parents = true) override;

    /**
     * Updates the file or directory.
     * @param params the parameter for subversion
     * @exception ClientException
     */
    Revisions update(const UpdateParameter &params) override;

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
    QByteArray
    cat(const Path &path,
        const Revision &revision,
        const Revision &peg_revision = Revision::UNDEFINED) override;
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
    void
    cat(svn::stream::SvnStream &buffer,
        const Path &path,
        const Revision &revision,
        const Revision &peg_revision) override;

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
    void get(const Path &path,
        const QString   &target,
        const Revision &revision,
        const Revision &peg_revision = Revision::UNDEFINED) override;

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
    void annotate(AnnotatedFile &target, const AnnotateParameter &params) override;

    /**
     * Commits changes to the repository. This usually requires
     * authentication, see Auth.
     * @param parameters CommitParameter to use
     * @return Returns revision transferred or svn::Revision::UNDEFINED if the revision number is invalid.
     * @exception ClientException
     */
    svn::Revision commit(const CommitParameter &parameters) override;

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException
     */
    svn::Revision
    copy(const Path &srcPath,
         const Revision &srcRevision,
         const Path &destPath) override;
    /**
     * Copies a versioned file with the history preserved.
     * @since subversion 1.5 api
     * @param parameter Class holding old required parameter
     * @see svn_client_copy4,svn_client_copy5
     * @exception ClientException
     */
    svn::Revision copy(const CopyParameter &parameter) override;

    /**
     * Moves or renames a file.
     * @param parameter Class holding old required parameter
     * @exception ClientException
     */
    svn::Revision move(const CopyParameter &parameter) override;

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
    svn::Revision
    mkdir(const Targets &targets,
          const QString &message,
          bool makeParent = true,
          const PropertiesMap &revProps = PropertiesMap()
         ) override;

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * @param path a local directory.
     * @exception ClientException
     */
    void cleanup(const Path &path) override;

    /**
     * Removes the 'conflicted' state on a file.
     * @exception ClientException
     */
    void resolve(const Path &path, Depth depth, const ConflictResult &resolution = ConflictResult()) override;

    /**
     * Exports the contents of either a subversion repository into a
     * 'clean' directory (meaning a directory with no administrative
     * directories).
     * @exception ClientException
     * @param params Parameter to use
     * @return revision exported
     */
    Revision doExport(const CheckoutParameter &params) override;

    /**
     * Update local copy to mirror a new url. This excapsulates the
     * svn_client_switch() client method.
     * @exception ClientException
     */
    Revision
    doSwitch(const Path &path, const Url &url,
        const Revision &revision,
        Depth depth,
        const Revision &peg = Revision::UNDEFINED,
        bool sticky_depth = true,
        bool ignore_externals = false,
        bool allow_unversioned = false,
        bool ignore_ancestry = false
      ) override;

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param importRepository
     * @param message log message.
     * @param depth kind of recurse operation
     * @param no_ignore if false, don't add items matching global ignore pattern
     * @param no_unknown_nodetype if true ignore files type not known like pipes or device files
     * @exception ClientException
     */
    svn::Revision
    import(const Path &path, const Url &importRepository,
           const QString &message,
           svn::Depth depth,
           bool no_ignore, bool no_unknown_nodetype,
           const PropertiesMap &revProps = PropertiesMap()) override;

    /**
     * Merge changes from two paths into a new local path. For reintegrate merge see svn::MergeParameter!
     * @exception ClientException
     * @sa svn::MergeParameter
     */
    void merge(const MergeParameter &parameters) override;

    void merge_peg(const MergeParameter &parameters) override;

    /**
     * Retrieve information for the given path
     * remote or local. Only gives with subversion 1.2
     * useful results
     *
     * @param path path for info
     * @param rec recursive (if dir)
     * @param rev for which revision
     * @param peg_revision peg revision
     * @return InfoEntries
     * @since subversion 1.2
     */
    InfoEntries
    info(const Path &path,
         Depth depth,
         const Revision &rev,
         const Revision &peg_revision = Revision::UNDEFINED,
         const StringArray &changelists = StringArray()
        ) override;

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
    bool log(const LogParameter &params, LogEntriesMap &target) override; 
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
    QByteArray diff_peg(const DiffParameter &options) override;

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
    QByteArray diff(const DiffParameter &options) override;

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * @param pathOrUrl
     * @param revision
     * @param peg at which revision path exists
     * @param depth @sa svn::Depth
     * @param retrieve_locks check for REPOSITORY locks while listing
     * @return a vector of directory entries, each with
     *         a relative path (only filename)
     */
    virtual DirEntries
    list(const Path &pathOrUrl,
         const Revision &revision,
         const Revision &peg,
         svn::Depth depth, bool retrieve_locks) override;

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
             Depth depth = DepthEmpty,
             const StringArray &changelists = StringArray()) override;

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
    virtual QPair<qlonglong, PathPropertiesMapList>
    propget(const QString &propName,
            const Path &path,
            const Revision &revision,
            const Revision &peg,
            Depth depth = svn::DepthEmpty,
            const StringArray &changelists = StringArray()) override;

    /**
     * set property in @a path no matter whether local or
     * repository
     *
     * @param params svn::PropertiesParameter holding required values.
     * Following is used:<br/>
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
    propset(const PropertiesParameter &params) override;

    /**
     * lists revision properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @return PropertiesList
     */
    virtual QPair<qlonglong, PropertiesMap>
    revproplist(const Path &path,
                const Revision &revision) override;

    /**
     * lists one revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @return PropertiesList
     */
    QPair<qlonglong, QString>
    revpropget(const QString &propName,
               const Path &path,
               const Revision &revision) override;

    /**
     * set revision property in @a path no matter whether local or
     * repository
     *
     * @param params parameter to use
     * @return Revision
     * @sa PropertiesParameter
     */
    virtual qlonglong
    revpropset(const PropertiesParameter &params) override;

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
    virtual qlonglong
    revpropdel(const QString &propName,
               const Path &path,
               const Revision &revision) override;

    /**
     * lock files in repository or working copy
     * @param targets items to be locked
     * @param message if non null stored with each lock in repository
     * @param steal_lock if true locks in wc will stolen.
     * @since subversion 1.2
     */
    void lock(const Targets &targets, const QString &message,
              bool steal_lock) override;
    /**
     * unlock files in repository or working copy
     * @param targets items to unlock
     * @param break_lock ignore any errors
     */
    void unlock(const Targets &targets,
                bool break_lock) override;

    void url2Revision(const QString &revstring,
                      Revision &start, Revision &end) override;
    void url2Revision(const QString &revstring,
                      Revision &start) override;

    bool RepoHasCapability(const Path &repository, Capability capability) override;

    static void checkErrorThrow(svn_error_t *error)
    {
        if (!error || error->apr_err == APR_SUCCESS) {
            return;
        }
        throw ClientException(error);
    }

private:
    ContextP m_context;

    /**
     * disallow assignment operator
     */
    Client_impl &operator= (const Client &);

    /**
     * disallow copy constructor
     */
    Client_impl(const Client_impl &);

    svn_error_t *internal_cat(const Path &path,
                              const Revision &revision,
                              const Revision &peg_revision,
                              svn::stream::SvnStream &);

    static apr_hash_t *map2hash(const PropertiesMap &, const Pool &);

    /** helper method
     * @sa svn_client_merge_reintegrate
     */
    virtual void merge_reintegrate(const MergeParameter &parameters);
};


}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
