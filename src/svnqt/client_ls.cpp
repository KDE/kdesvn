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
#include "svn_path.h"
#include "svn_sorts.h"
//#include "svn_utf.h"

#include "svnqt/dirent.hpp"
#include "svnqt/exception.hpp"
#include "svnqt/svnqt_defines.hpp"

static int
compare_items_as_paths (const svn_sort__item_t *a, const svn_sort__item_t *b)
{
  return svn_path_compare_paths ((const char *)a->key, (const char *)b->key);
}

namespace svn
{

  DirEntries
  Client_impl::list_simple(const Path& _p,
          const Revision& revision,
          const Revision& peg,
          bool recurse) throw (ClientException)
  {
    Pool pool;

    apr_hash_t * hash;
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

    DirEntries entries;

    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *>
        (apr_hash_get (hash, entryname, item->klen));

      entries.push_back (DirEntry(QString::FROMUTF8(entryname), dirent));
    }

    return entries;
  }

  DirEntries
  Client_impl::list_locks(const Path& pathOrUrl,
        const Revision& revision,
        const Revision& peg,
        bool recurse) throw (ClientException)
  {
    Pool pool;

    apr_hash_t * hash;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
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
#else
    QString url = pathOrUrl;
    url+=QString::FROMUTF8("/");
    bool _det = true;

    svn_error_t * error =
      svn_client_ls2 (&hash,
                     pathOrUrl.cstr(),
                     peg,
                     revision,
                     recurse,
                     *m_context,
                     pool);
#endif

    if (error != 0)
      throw ClientException (error);

    apr_array_header_t *
      array = svn_sort__hash (
        hash, compare_items_as_paths, pool);

    DirEntries entries;

    for (int i = 0; i < array->nelts; ++i)
    {
      const char *entryname;
      svn_dirent_t *dirent;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
      svn_lock_t * lockent;
#endif
      svn_sort__item_t *item;

      item = &APR_ARRAY_IDX (array, i, svn_sort__item_t);

      entryname = static_cast<const char *>(item->key);

      dirent = static_cast<svn_dirent_t *>
        (apr_hash_get (hash, entryname, item->klen));
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
      lockent = static_cast<svn_lock_t *>
        (apr_hash_get(lock_hash,entryname,item->klen));
      entries.push_back (DirEntry(QString::FROMUTF8(entryname), dirent,lockent));
#else
      if (!_det) {
        entries.push_back (DirEntry(QString::FROMUTF8(entryname),dirent));
      } else {
        try {
            InfoEntries infoEntries = info(url+entryname,false,revision,Revision(Revision::UNDEFINED));
            entries.push_back(DirEntry(QString::FROMUTF8(entryname),dirent,infoEntries[0].lockEntry()));
        } catch (ClientException) {
            _det = false;
            entries.push_back(DirEntry(QString::FROMUTF8(entryname),dirent));
        }
      }
#endif
    }

    return entries;
  }

  DirEntries
  Client_impl::list(const Path& pathOrUrl,
                const Revision& revision,
                const Revision& peg,
                bool recurse,bool retrieve_locks) throw (ClientException)
  {
      if (!retrieve_locks) {
          return list_simple(pathOrUrl,revision,peg,recurse);
      } else {
          return list_locks(pathOrUrl,revision,peg,recurse);
      }
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
