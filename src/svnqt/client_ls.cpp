/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
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
#include "svnqt/client_impl.h"

// subversion api
#include "svn_client.h"
#include "svn_path.h"
#include "svn_sorts.h"
//#include "svn_utf.h"

#include "svnqt/dirent.h"
#include "svnqt/exception.h"
#include "svnqt/svnqt_defines.h"

#include "svnqt/helper.h"

namespace svn
{

  DirEntries
  Client_impl::list_simple(const Path& _p,
          const Revision& revision,
          const Revision& peg,
          bool recurse) throw (ClientException)
  {
    DirEntries entries;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
    Q_UNUSED(_p);
    Q_UNUSED(revision);
    Q_UNUSED(peg);
    Q_UNUSED(recurse);
#else
    Pool pool;
    apr_hash_t * hash = 0;
    /* don't want the lock hashs, so we simply use ls2 on svn 1.3, too.
     * there is that method a cast to ls3 with lock_hash == 0
     */
    svn_error_t * error =
      svn_client_ls2 (&hash,
                     _p.cstr(),
                     peg.revision(),
                     revision.revision(),
                     recurse,
                     *m_context,
                     pool);

    if (error != 0)
      throw ClientException (error);

    apr_array_header_t *
      array = svn_sort__hash (
        hash, compare_items_as_paths, pool);

    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *>
        (apr_hash_get (hash, entryname, item->klen));
      m_context->contextAddListItem(&entries,dirent,0,QString::FROMUTF8(entryname));
      //entries.push_back (new DirEntry(QString::FROMUTF8(entryname), dirent));
    }
#endif

    return entries;
  }

  DirEntries
  Client_impl::list_locks(const Path& pathOrUrl,
        const Revision& revision,
        const Revision& peg,
        bool recurse) throw (ClientException)
  {
    DirEntries entries;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
    Q_UNUSED(pathOrUrl);
    Q_UNUSED(revision);
    Q_UNUSED(peg);
    Q_UNUSED(recurse);
#else
    Pool pool;

    apr_hash_t * hash;
    apr_hash_t * lock_hash;

    svn_error_t * error =
      svn_client_ls3 (&hash,
                      &lock_hash,
                     pathOrUrl.cstr(),
                     peg,
                     revision,
                     recurse,
                     *m_context,
                     pool);

    if (error != 0)
      throw ClientException (error);

    apr_array_header_t *
      array = svn_sort__hash (
        hash, compare_items_as_paths, pool);


    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
      svn_lock_t * lockent;
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *>
        (apr_hash_get (hash, entryname, item->klen));
      lockent = static_cast<svn_lock_t *>
        (apr_hash_get(lock_hash,entryname,item->klen));
      m_context->contextAddListItem(&entries,dirent,lockent,QString::FROMUTF8(entryname));
      //entries.push_back (new DirEntry(QString::FROMUTF8(entryname), dirent,lockent));
    }
#endif
    return entries;
  }

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
  static svn_error_t * s_list_func
          (void * baton,const char*path,const svn_dirent_t*dirent,const svn_lock_t*lock,const char* abs_path,apr_pool_t*)
  {
      Q_UNUSED(abs_path);
      if (!baton || !path || !dirent) {
          return 0;
      }
      /* check every loop for cancel of operation */
      Client_impl::sBaton * l_baton = (Client_impl::sBaton*)baton;
      Context*l_context = l_baton->m_context;
      DirEntries*entries = static_cast<DirEntries*>(l_baton->m_data);
      svn_client_ctx_t*ctx = l_context->ctx();
      if (ctx&&ctx->cancel_func) {
          SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
      }
      l_context->contextAddListItem(entries,dirent,lock,QString::FROMUTF8(path));
      //entries->push_back(new DirEntry(QString::FROMUTF8(path),dirent,lock));
      return 0;
  }
#endif

  DirEntries
  Client_impl::list(const Path& pathOrUrl,
                const Revision& revision,
                const Revision& peg,
                Depth depth,bool retrieve_locks) throw (ClientException)
  {

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
      sBaton _baton;
      Pool pool;
      DirEntries entries;
      _baton.m_data = &entries;
      _baton.m_context=m_context;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
      svn_error_t * error = svn_client_list2(pathOrUrl.cstr(),
                                            peg,
                                            revision,
                                            svn::internal::DepthToSvn(depth),
                                            SVN_DIRENT_ALL,
                                            retrieve_locks,
                                            s_list_func,
                                            &_baton,
                                            *m_context,
                                            pool
                                           );
#else
      bool recurse = depth==DepthInfinity;
      svn_error_t * error = svn_client_list(pathOrUrl.cstr(),
                                            peg,
                                            revision,
                                            recurse,
                                            SVN_DIRENT_ALL,
                                            retrieve_locks,
                                            s_list_func,
                                            &_baton,
                                            *m_context,
                                            pool
                                           );
#endif
      if (error != 0) {
          throw ClientException (error);
      }
      return entries;
#else
      if (!retrieve_locks) {
          return list_simple(pathOrUrl,revision,peg,depth==DepthInfinity);
      } else {
          return list_locks(pathOrUrl,revision,peg,depth==DepthInfinity);
      }
#endif
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
