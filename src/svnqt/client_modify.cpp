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

namespace svn
{
  svn_revnum_t
  Client_impl::checkout (const Path& url, const Path & destPath,
              const Revision & revision,
              const Revision & peg,
              bool recurse,
              bool ignore_externals) throw (ClientException)
  {
    Pool subPool;
    svn_revnum_t revnum = 0;
    Path up(url);
    svn_error_t * error =
      svn_client_checkout2(&revnum,
                           up.cstr(),
                           destPath.cstr(),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           ignore_externals,
                           *m_context,
                           subPool);

    if(error != NULL)
      throw ClientException (error);
    return revnum;
  }

  void
  Client_impl::remove (const Path & path,
                  bool force) throw (ClientException)
  {
    Targets targets (path.path());
    remove(targets,force);
  }

  void
  Client_impl::remove (const Targets & targets,
                  bool force) throw (ClientException)
  {
    Pool pool;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_commit_info_t *commit_info = NULL;
#else
    svn_client_commit_info_t *commit_info = NULL;
#endif

    svn_error_t * error =

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
      svn_client_delete2
#else
      svn_client_delete
#endif
                    (&commit_info,
                         const_cast<apr_array_header_t*> (targets.array (pool)),
                         force,
                         *m_context,
                         pool);
    if(error != NULL)
      throw ClientException (error);
  }

  void
  Client_impl::revert (const Targets & targets,
                  bool recurse) throw (ClientException)
  {
    Pool pool;

    svn_error_t * error =
      svn_client_revert ((targets.array (pool)),
                         recurse,
                         *m_context,
                         pool);

    if(error != NULL)
      throw ClientException (error);
  }

  void
  Client_impl::add (const Path & path,
               bool recurse,bool force, bool no_ignore) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
      svn_client_add3 (path.cstr (),
                      recurse,
                      force,
                      no_ignore,
                      *m_context,
                      pool);
#else
      svn_client_add2 (path.cstr (),
                      recurse,
                      force,
                      *m_context,
                      pool);
#endif
    if(error != NULL)
      throw ClientException (error);
  }

  Revisions
  Client_impl::update (const Targets & path,
                  const Revision & revision,
                  bool recurse,
                  bool ignore_externals) throw (ClientException)
  {
    Pool pool;
    Revisions resulting;
    svn_error_t * error;

    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_revisions = apr_array_make (apr_pool,
                      path.size(),
                      sizeof (svn_revnum_t));
    error = svn_client_update2(&apr_revisions,path.array(pool),revision,recurse,ignore_externals,*m_context,pool);
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
                  bool recurse,bool keep_locks) throw (ClientException)
  {
    Pool pool;

    m_context->setLogMessage (message);

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_commit_info_t *commit_info = NULL;
#else
    svn_client_commit_info_t *commit_info = NULL;
#endif

    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
      svn_client_commit3
#else
      svn_client_commit2
#endif
                        (&commit_info,
                         targets.array (pool),
                         recurse,
                         keep_locks,
                         *m_context,
                         pool);

    if (error != NULL)
      throw ClientException (error);

    if (commit_info && SVN_IS_VALID_REVNUM (commit_info->revision))
      return (commit_info->revision);

    return svn::Revision::UNDEFINED;
  }

  void
    Client_impl::copy(const Targets & srcPaths,
                    const Revision & srcRevision,
                    const Path & destPath,
                     bool asChild,bool makeParent) throw (ClientException)
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
                    asChild,makeParent,*m_context,pool);
        if (error!=0){
            throw ClientException (error);
        }
#else
        Q_UNUSED(asChild);
        Q_UNUSED(makeParent);
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
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_commit_info_t *commit_info = NULL;
#else
    svn_client_commit_info_t *commit_info = NULL;
#endif
    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
        svn_client_copy2
#else
        svn_client_copy
#endif
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
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_commit_info_t *commit_info = NULL;
#else
    svn_client_commit_info_t *commit_info = NULL;
#endif
    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
    svn_client_move4
#elif (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_client_move3
#else
    svn_client_move2
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
                 const QString& message) throw (ClientException)
  {
    Targets targets(path.path());
    mkdir(targets,message);
  }

  void
  Client_impl::mkdir (const Targets & targets,
                 const QString&msg) throw (ClientException)
  {
    Pool pool;
    m_context->setLogMessage(msg);

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_commit_info_t *commit_info = NULL;
#else
    svn_client_commit_info_t *commit_info = NULL;
#endif

    svn_error_t * error =
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    svn_client_mkdir2
#else
    svn_client_mkdir
#endif
            (&commit_info,
                const_cast<apr_array_header_t*>
                (targets.array (pool)),
                    *m_context, pool);

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

  void
  Client_impl::resolved (const Path & path,
                    bool recurse) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_resolved (path.cstr (),
                           recurse,
                           *m_context,
                           pool);

    if(error != NULL)
      throw ClientException (error);
  }

  svn_revnum_t
  Client_impl::doExport (const Path & srcPath,
              const Path & destPath,
              const Revision & revision,
              const Revision & peg,
              bool overwrite,
              const QString&native_eol,
              bool ignore_externals,
              bool recurse) throw (ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;
    const char*_neol;
    if (native_eol==QString::null) {
        _neol = (const char*)0;
    } else {
        _neol = native_eol.TOUTF8();
    }
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
    if(error != NULL)
      throw ClientException (error);
    return revnum;
  }

  svn_revnum_t
  Client_impl::doSwitch (const Path & path,
                    const QString& url,
                    const Revision & revision,
                    bool recurse) throw (ClientException)
  {
    Pool pool;
    svn_revnum_t revnum = 0;
    svn_error_t * error =
      svn_client_switch (&revnum,
                         path.cstr(),
                         url.TOUTF8(),
                         revision.revision (),
                         recurse,
                         *m_context,
                         pool);

    if(error != NULL)
      throw ClientException (error);
    return revnum;
  }

  void
  Client_impl::import (const Path & path,
                  const QString& url,
                  const QString& message,
                  bool recurse) throw (ClientException)
    {
        import(path,url,message,recurse,false);
    }

  void
  Client_impl::import (const Path & path,
                  const QString& url,
                  const QString& message,
                  bool recurse,
                  bool no_ignore) throw (ClientException)

  {
#if (SVN_VER_MAJOR == 1) && (SVN_VER_MINOR < 3)
      Q_UNUSED(no_ignore);
      svn_client_commit_info_t *commit_info = NULL;
#else
      svn_commit_info_t *commit_info = NULL;
#endif
    Pool pool;

    m_context->setLogMessage (message);

    svn_error_t * error =
#if (SVN_VER_MAJOR == 1) && (SVN_VER_MINOR < 3)
        svn_client_import (&commit_info,
                        path.cstr (),
                        url.TOUTF8(),
                        !recurse,
                        *m_context,
                        pool);
#else
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
                 const Path & localPath, bool force,
                 bool recurse,
                 bool notice_ancestry,
                 bool dry_run) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_merge (path1.cstr (),
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

    if(error != NULL)
      throw ClientException (error);
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
