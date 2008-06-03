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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif
// svncpp
#include "svnqt/client_impl.hpp"

// subversion api
#include "svn_client.h"

#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "svnqt/stringarray.hpp"

#include "svnqt/helper.hpp"

namespace svn
{
  svn_revnum_t
  Client_impl::checkout (const Path& url, const Path & destPath,
              const Revision & revision,
              const Revision & peg,
              svn::Depth depth,
              bool ignore_externals,
              bool overwrite
                        ) throw (ClientException)
  {
    Pool subPool;
    svn_revnum_t revnum = 0;
    Path up(url);
    svn_error_t * error = 0;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_checkout3(&revnum,
                up.cstr(),
                destPath.cstr(),
                peg.revision(),
                revision.revision (),
                internal::DepthToSvn(depth),
                ignore_externals,
                overwrite,
                *m_context,
                subPool);
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(overwrite);
    error = svn_client_checkout2(&revnum,
                           up.cstr(),
                           destPath.cstr(),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           ignore_externals,
                           *m_context,
                           subPool);
#endif
    if(error != NULL)
      throw ClientException (error);
    return revnum;
  }

  void Client_impl::remove (const Path & path,bool force,
                            bool keep_local,
                            const PropertiesMap&revProps) throw (ClientException)
  {
      Targets targets (path.path());
      remove(targets,force,keep_local,revProps);
  }

  void
  Client_impl::remove (const Targets & targets,
                  bool force,
                  bool keep_local,
                  const PropertiesMap&revProps
                      ) throw (ClientException)
  {
    Pool pool;

    svn_commit_info_t *commit_info = 0;

    svn_error_t * error =
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
            svn_client_delete3(
                               &commit_info,
                               targets.array(pool),
                               force,
                               keep_local,
                               map2hash(revProps,pool),
                               *m_context,
                               pool
                              );
#else
      svn_client_delete2
                    (&commit_info,
                         const_cast<apr_array_header_t*> (targets.array (pool)),
                         force,
                         *m_context,
                         pool);
#endif
    if(error != 0) {
        throw ClientException (error);
    }
  }

  void
  Client_impl::revert (const Targets & targets,
                  Depth depth,
                  const StringArray&changelist
                      ) throw (ClientException)
  {
    Pool pool;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error =
            svn_client_revert2 ((targets.array (pool)),
                                internal::DepthToSvn(depth),
                                changelist.array(pool),
                                *m_context,
                                pool);

#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(changelist);
    svn_error_t * error =
      svn_client_revert ((targets.array (pool)),
                         recurse,
                         *m_context,
                         pool);
#endif
    if(error != NULL) {
        throw ClientException (error);
    }
  }

  void
  Client_impl::add (const Path & path,
                    svn::Depth depth,bool force, bool no_ignore, bool add_parents) throw (ClientException)
  {
    Pool pool;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error =
            svn_client_add4(path.cstr (),
                             internal::DepthToSvn(depth),
                             force,
                             no_ignore,
                             add_parents,
                             *m_context,
                             pool);
#else
    Q_UNUSED(add_parents);
    svn_error_t * error =
      svn_client_add3 (path.cstr (),
                      depth==DepthInfinity,
                      force,
                      no_ignore,
                      *m_context,
                      pool);
#endif
    if(error != NULL)
      throw ClientException (error);
  }

  Revisions
  Client_impl::update (const Targets & path,
                  const Revision & revision,
                  Depth depth,
                  bool ignore_externals,
                  bool allow_unversioned,
                  bool sticky_depth
                      ) throw (ClientException)
  {
    Pool pool;
    Revisions resulting;
    svn_error_t * error;

    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_revisions = apr_array_make (apr_pool,
                      path.size(),
                      sizeof (svn_revnum_t));
    if (depth==DepthUnknown) {
        depth=DepthInfinity;
    }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_update3(&apr_revisions,path.array(pool),revision,internal::DepthToSvn(depth),sticky_depth,ignore_externals,allow_unversioned,*m_context,pool);
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(sticky_depth);
    Q_UNUSED(allow_unversioned);
    error = svn_client_update2(&apr_revisions,path.array(pool),revision,recurse,ignore_externals,*m_context,pool);
#endif
    if (error!=NULL) {
        throw ClientException(error);
    }
    for (int i = 0; i < apr_revisions->nelts; ++i)
    {
      svn_revnum_t * _rev =
        &APR_ARRAY_IDX (apr_revisions, i, svn_revnum_t);

      resulting.push_back((*_rev));
    }
    return resulting;
  }

  svn::Revision
  Client_impl::commit (const Targets & targets, const QString& message,
                       svn::Depth depth,bool keep_locks,
                       const svn::StringArray&changelist,
                       const PropertiesMap&revProps,
                       bool keep_changelist) throw (ClientException)
  {
    Pool pool;

    m_context->setLogMessage (message);
    svn_commit_info_t *commit_info = NULL;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error =
            svn_client_commit4 (
                &commit_info,
                targets.array (pool),
                internal::DepthToSvn(depth),
                keep_locks,
                keep_changelist,
                changelist.array(pool),
                map2hash(revProps,pool),
                *m_context,
                pool);
#else
    Q_UNUSED(changelist);
    Q_UNUSED(keep_changelist);
    Q_UNUSED(revProps);
    bool recurse = depth==DepthInfinity;

    svn_error_t * error =
      svn_client_commit3
                        (&commit_info,
                         targets.array (pool),
                         recurse,
                         keep_locks,
                         *m_context,
                         pool);
#endif
    if (error != NULL) {
        throw ClientException (error);
    }

    if (commit_info && SVN_IS_VALID_REVNUM (commit_info->revision))
      return (commit_info->revision);

    return svn::Revision::UNDEFINED;
  }

  void
    Client_impl::copy(const Targets & srcPaths,
                    const Revision & srcRevision,
                    const Path & destPath,
                     bool asChild,bool makeParent,
                     const PropertiesMap&revProps
                     ) throw (ClientException)
    {
        if (srcPaths.size()<1)
        {
            throw ClientException("Wrong size of sources.");
        }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        Pool pool;
        svn_commit_info_t *commit_info = 0L;
        apr_array_header_t * sources = apr_array_make(pool,srcPaths.size(),sizeof(svn_client_copy_source_t *));
        for (size_t j=0;j<srcPaths.size();++j)
        {
            svn_client_copy_source_t* source = (svn_client_copy_source_t*)apr_palloc(pool, sizeof(svn_client_copy_source_t));
            source->path = apr_pstrdup(pool,srcPaths[j].path().TOUTF8());
            source->revision=srcRevision.revision();
            source->peg_revision=source->revision;
            APR_ARRAY_PUSH(sources, svn_client_copy_source_t *) = source;
        }
        svn_error_t * error =
                svn_client_copy4(&commit_info,
                    sources,
                    destPath.cstr(),
                    asChild,makeParent,map2hash(revProps,pool),*m_context,pool);
        if (error!=0){
            throw ClientException (error);
        }
#else
        Q_UNUSED(asChild);
        Q_UNUSED(makeParent);
        Q_UNUSED(revProps);
        for (size_t j=0;j<srcPaths.size();++j)
        {
            copy(srcPaths[j],srcRevision,destPath);
        }
#endif
    }

  void
  Client_impl::copy (const Path & srcPath,
                const Revision & srcRevision,
                const Path & destPath) throw (ClientException)
  {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
      copy(srcPath,srcRevision,destPath,true,false);
#else
      Pool pool;
    svn_commit_info_t *commit_info = NULL;
    svn_error_t * error =
        svn_client_copy2
                    (&commit_info,
                       srcPath.cstr (),
                       srcRevision.revision (),
                       destPath.cstr (),
                       *m_context,
                       pool);

    if(error != NULL)
      throw ClientException (error);
#endif
  }

  void
  Client_impl::move (const Path & srcPath,
                const Path & destPath,
                bool force) throw (ClientException)
  {
    Pool pool;
    svn_commit_info_t *commit_info = NULL;
    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_client_move4
#else
    svn_client_move3
#endif
                     (&commit_info,
                       srcPath.cstr (),
                       destPath.cstr (),
                       force,
                       *m_context,
                       pool);

    if(error != NULL)
      throw ClientException (error);
  }

  void
  Client_impl::mkdir (const Path & path,
                 const QString& message,
                 bool makeParent,
                 const PropertiesMap&revProps
                     ) throw (ClientException)
  {
    Targets targets(path.path());
    mkdir(targets,message,makeParent,revProps);
  }

  void
  Client_impl::mkdir (const Targets & targets,
                 const QString&msg,
                 bool makeParent,
                 const PropertiesMap&revProps
                     ) throw (ClientException)
  {
    Pool pool;
    m_context->setLogMessage(msg);

    svn_commit_info_t *commit_info = NULL;

    svn_error_t * error = 0;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_mkdir3
            (&commit_info,
              const_cast<apr_array_header_t*>(targets.array (pool)),
              makeParent,
              map2hash(revProps,pool),
              *m_context, pool);
#else
    Q_UNUSED(makeParent);
    Q_UNUSED(revProps);
    error = svn_client_mkdir2
            (&commit_info,
             const_cast<apr_array_header_t*>(targets.array (pool)),
            *m_context, pool);
#endif

    /* important! otherwise next op on repository uses that logmessage again! */
    m_context->setLogMessage(QString::null);

    if(error != NULL)
      throw ClientException (error);
  }

  void
  Client_impl::cleanup (const Path & path) throw (ClientException)
  {
    Pool subPool;
    apr_pool_t * apr_pool = subPool.pool ();

    svn_error_t * error =
      svn_client_cleanup (path.cstr (), *m_context, apr_pool);

    if(error != NULL)
      throw ClientException (error);
  }

  void Client_impl::resolve(const Path & path,Depth depth,const ConflictResult&resolution) throw (ClientException)
  {
    Pool pool;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    const svn_wc_conflict_result_t*aResult=resolution.result(pool);
    svn_error_t*error=svn_client_resolve(path.cstr(),internal::DepthToSvn(depth),aResult->choice,*m_context,pool);

#else
    Q_UNUSED(resolution);
    bool recurse=depth==DepthInfinity;
    svn_error_t * error =
      svn_client_resolved (path.cstr (),
                           recurse,
                           *m_context,
                           pool);
#endif
    if(error != NULL) {
        throw ClientException (error);
    }
  }

  svn_revnum_t
  Client_impl::doExport (const Path & srcPath,
              const Path & destPath,
              const Revision & revision,
              const Revision & peg,
              bool overwrite,
              const QString&native_eol,
              bool ignore_externals,
              svn::Depth depth) throw (ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;
    const char*_neol;
    if (native_eol==QString::null) {
        _neol = (const char*)0;
    } else {
        _neol = native_eol.TOUTF8();
    }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error =
            svn_client_export4(&revnum,
                                srcPath.cstr(),
                                destPath.cstr(),
                                peg.revision(),
                                revision.revision(),
                                overwrite,
                                ignore_externals,
                                internal::DepthToSvn(depth),
                                _neol,
                                *m_context,
                                pool);
#else
    bool recurse = depth==svn::DepthInfinity;
    svn_error_t * error =
      svn_client_export3(&revnum,
                        srcPath.cstr(),
                        destPath.cstr(),
                        peg.revision(),
                        revision.revision(),
                        overwrite,
                        ignore_externals,
                        recurse,
                        _neol,
                         *m_context,
                         pool);
#endif
    if(error != NULL)
      throw ClientException (error);
    return revnum;
  }

  svn_revnum_t
  Client_impl::doSwitch (
                         const Path & path, const QString& url,
                         const Revision & revision,
                         Depth depth,
                         const Revision & peg,
                         bool sticky_depth,
                         bool ignore_externals,
                         bool allow_unversioned
                        ) throw (ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;
    svn_error_t * error = 0;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_switch2(
                               &revnum,
                               path.cstr(),
                               url.TOUTF8(),
                               peg.revision(),
                               revision.revision(),
                               internal::DepthToSvn(depth),
                               sticky_depth,
                               ignore_externals,
                               allow_unversioned,
                               *m_context,
                               pool
                              );
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(peg);
    Q_UNUSED(sticky_depth);
    Q_UNUSED(ignore_externals);
    Q_UNUSED(allow_unversioned);
    error = svn_client_switch (&revnum,
                         path.cstr(),
                         url.TOUTF8(),
                         revision.revision (),
                         recurse,
                         *m_context,
                         pool);
#endif
    if(error != NULL) {
        throw ClientException (error);
    }
    return revnum;
  }

  void
  Client_impl::import (const Path & path,
                  const QString& url,
                  const QString& message,
                  svn::Depth depth,
                  bool no_ignore,bool no_unknown_nodetype,
                  const PropertiesMap&revProps
                      ) throw (ClientException)

  {
    svn_commit_info_t *commit_info = NULL;
    Pool pool;

    m_context->setLogMessage (message);
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error =
        svn_client_import3(&commit_info,path.cstr (),url.TOUTF8(),
            internal::DepthToSvn(depth),no_ignore,no_unknown_nodetype,
                                 map2hash(revProps,pool),
                                 *m_context,pool);
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(revProps);
    Q_UNUSED(no_unknown_nodetype);

    svn_error_t * error =
        svn_client_import2(&commit_info,
                        path.cstr (),
                        url.TOUTF8(),
                        !recurse,
                        no_ignore,
                        *m_context,
                        pool);
#endif
    /* important! otherwise next op on repository uses that logmessage again! */
    m_context->setLogMessage(QString::null);

    if(error != NULL)
      throw ClientException (error);
  }

  void
  Client_impl::merge (const Path & path1, const Revision & revision1,
                 const Path & path2, const Revision & revision2,
                 const Path & localPath,
                 bool force,
                 Depth depth,
                 bool notice_ancestry,
                 bool dry_run,
                 bool record_only,
                 const StringArray&merge_options
                     ) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error = 0;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_merge3(path1.cstr (),
                    revision1.revision (),
                    path2.cstr (),
                    revision2.revision (),
                    localPath.cstr (),
                    internal::DepthToSvn(depth),
                    !notice_ancestry,
                    force,
                    record_only,
                    dry_run,
                    merge_options.array(pool),
                    *m_context,
                    pool);
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(record_only);
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4))
    error = svn_client_merge2(path1.cstr (),
                    revision1.revision (),
                    path2.cstr (),
                    revision2.revision (),
                    localPath.cstr (),
                    recurse,
                    !notice_ancestry,
                    force,
                    dry_run,
                    merge_options.array(pool),
                    *m_context,
                    pool);
#else
    Q_UNUSED(merge_options);
    error = svn_client_merge(path1.cstr (),
                        revision1.revision (),
                        path2.cstr (),
                        revision2.revision (),
                        localPath.cstr (),
                        recurse,
                        !notice_ancestry,
                        force,
                        dry_run,
                        *m_context,
                        pool);
#endif
#endif

    if(error != 0) {
        throw ClientException (error);
    }
  }

  void
  Client_impl::relocate (const Path & path,
                    const QString& from_url,
                    const QString& to_url,
                    bool recurse) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_relocate (path.cstr (),
                         from_url.TOUTF8(),
                         to_url.TOUTF8(),
                         recurse,
                         *m_context,
                         pool);

    if(error != NULL)
      throw ClientException (error);
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
