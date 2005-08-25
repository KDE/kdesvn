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

// Subversion api
#include "svn_client.h"
#include "svn_sorts.h"
//#include "svn_utf.h"

// svncpp
#include "svncpp/client.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/exception.hpp"
#include "svncpp/pool.hpp"
#include "svncpp/status.hpp"
#include "svncpp/targets.hpp"
#include "svncpp/info_entry.hpp"
#include "svncpp/url.hpp"

#include <kdebug.h>

namespace svn
{
  static svn_error_t *
  logReceiver (void *baton,
                   apr_hash_t * changedPaths,
                   svn_revnum_t rev,
                   const char *author,
                   const char *date,
                   const char *msg,
                   apr_pool_t * pool)
  {
    LogEntries * entries =
      (LogEntries *) baton;
    entries->insert (entries->begin (), LogEntry (rev, author, date, msg));
    if (changedPaths != NULL)
    {
      LogEntry &entry = entries->front ();

      for (apr_hash_index_t *hi = apr_hash_first (pool, changedPaths);
           hi != NULL;
           hi = apr_hash_next (hi))
      {
        char *path;
        void *val;
        apr_hash_this (hi, (const void **) &path, NULL, &val);

        svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);

        entry.changedPaths.push_back (
              LogChangePathEntry (path,
                                  log_item->action,
                                  log_item->copyfrom_path,
                                  log_item->copyfrom_rev) );

      }
    }

    return NULL;
  }

  struct StatusEntriesBaton {
    apr_pool_t* pool;
    apr_hash_t* hash;
  };

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  static svn_error_t * InfoEntryFunc(void*baton,
                            const char*path,
                            const svn_info_t*info,
                            apr_pool_t *pool)
  {
    StatusEntriesBaton* seb = (StatusEntriesBaton*)baton;
    path = apr_pstrdup (seb->pool, path);
    InfoEntry*e = new InfoEntry(info,path);
    apr_hash_set (seb->hash, path, APR_HASH_KEY_STRING, e);
    return NULL;
  }
#endif

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  static void StatusEntriesFunc (void *baton,
                                 const char *path,
                                 svn_wc_status2_t *status)
#else
  static void StatusEntriesFunc (void *baton,
                                 const char *path,
                                 svn_wc_status_t *status)
#endif
  {
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    svn_wc_status2_t* stat;
#else
    svn_wc_status_t* stat;
#endif
      StatusEntriesBaton* seb = (StatusEntriesBaton*)baton;

      path = apr_pstrdup (seb->pool, path);
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
      stat = svn_wc_dup_status2 (status, seb->pool);
#else
      stat = svn_wc_dup_status (status, seb->pool);
#endif
      apr_hash_set (seb->hash, path, APR_HASH_KEY_STRING, stat);
  }

  static StatusEntries
  localStatus (const QString& path,
               const bool descend,
               const bool get_all,
               const bool update,
               const bool no_ignore,
               Context * context)
  {
    svn_error_t *error;
    StatusEntries entries;
    apr_hash_t *status_hash;
    svn_revnum_t revnum;
    Revision rev (Revision::HEAD);
    Pool pool;
    StatusEntriesBaton baton;

    status_hash = apr_hash_make (pool);
    baton.hash = status_hash;
    baton.pool = pool;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    error = svn_client_status2 (
      &revnum,      // revnum
      path.utf8(),         // path
      rev,
      StatusEntriesFunc, // status func
      &baton,        // status baton
      descend,       // recurse (true) or immediate childs
      get_all,       // get all not only interesting
      update,        // check for updates
      no_ignore,     // hide ignored files or not
      false,       /// @todo first shot - should get a parameter
      *context,    //client ctx
      pool);
#else
    error = svn_client_status (
      &revnum,      // revnum
      path.utf8(),         // path
      rev,
      StatusEntriesFunc, // status func
      &baton,        // status baton
      descend,
      get_all,
      update,
      no_ignore,
      *context,    //client ctx
      pool);
#endif

    if (error!=NULL)
    {
      throw ClientException (error);
    }

    apr_array_header_t *statusarray =
      svn_sort__hash (status_hash, svn_sort_compare_items_as_paths,
                            pool);
    int i;

    /* Loop over array, printing each name/status-structure */
    for (i = 0; i < statusarray->nelts; ++i)
    {
      const svn_sort__item_t *item;
      const char *filePath;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
      svn_wc_status2_t *status = NULL;
#else
      svn_wc_status_t *status = NULL;
#endif

      item = &APR_ARRAY_IDX (statusarray, i, const svn_sort__item_t);
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
      status = (svn_wc_status2_t *) item->value;
#else
      status = (svn_wc_status_t *) item->value;
#endif

      filePath = (const char *) item->key;
      entries.push_back (Status(filePath, status));
    }
    return entries;
  }

  static Status
  dirEntryToStatus (const QString& path, const DirEntry & dirEntry)
  {
    Pool pool;
    svn_wc_entry_t * e =
      static_cast<svn_wc_entry_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_entry_t)));

    QString url = path;
    url += "/";
    url += dirEntry.name();

    e->name = apr_pstrdup(pool,dirEntry.name().utf8());
    e->revision = dirEntry.createdRev ();
    e->url = apr_pstrdup(pool,url.utf8());
    e->kind = dirEntry.kind ();
    e->schedule = svn_wc_schedule_normal;
    e->text_time = dirEntry.time ();
    e->prop_time = dirEntry.time ();
    e->cmt_rev = dirEntry.createdRev ();
    e->cmt_date = dirEntry.time ();
    e->cmt_author = dirEntry.lastAuthor ();

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    svn_wc_status2_t * s =
      static_cast<svn_wc_status2_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_status2_t)));
#else
    svn_wc_status_t * s =
      static_cast<svn_wc_status_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_status_t)));
#endif
    s->entry = e;
    s->text_status = svn_wc_status_normal;
    s->prop_status = svn_wc_status_normal;
    s->locked = 0;
    s->switched = 0;
    s->repos_text_status = svn_wc_status_normal;
    s->repos_prop_status = svn_wc_status_normal;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    s->repos_lock = 0;
#endif
    return Status (url, s);
  }

  static StatusEntries
  remoteStatus (Client * client,
                const QString& path,
                const bool descend,
                const bool get_all,
                const bool update,
                const bool no_ignore,
                Revision revision,
                Context * context)
  {
    DirEntries dirEntries = client->list(path, revision, descend);
    DirEntries::const_iterator it;

    StatusEntries entries;

    for (it = dirEntries.begin (); it != dirEntries.end (); it++)
    {
      const DirEntry & dirEntry = *it;

      entries.push_back (dirEntryToStatus (path, dirEntry));
    }

    return entries;
  }

  StatusEntries
  Client::status (const QString& path,
                  const bool descend,
                  const bool get_all,
                  const bool update,
                  const bool no_ignore,
                  Revision revision) throw (ClientException)
  {
    if (Url::isValid (path))
      return remoteStatus (this, path, descend, get_all, update,
                           no_ignore,revision,m_context);
    else
      return localStatus (path, descend, get_all, update,
                          no_ignore, m_context);
  }

  static Status
  localSingleStatus (const QString& path, Context * context,bool update=false)
  {
    svn_error_t *error;
    apr_hash_t *status_hash;
    Pool pool;
    StatusEntriesBaton baton;
    svn_revnum_t revnum;
    Revision rev (Revision::HEAD);

    status_hash = apr_hash_make( pool );
    baton.hash = status_hash;
    baton.pool = pool;

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    error = svn_client_status2 (
      &revnum,      // revnum
      path.utf8(),         // path
      rev,
      StatusEntriesFunc, // status func
      &baton,        // status baton
      false,
      true,
      update,
      false,
      false,
      *context,    //client ctx
      pool);
#else
    error = svn_client_status (
      &revnum,      // revnum
      path.utf8(),         // path
      rev,
      StatusEntriesFunc, // status func
      &baton,        // status baton
      false,
      true,
      update,
      false,
      *context,    //client ctx
      pool);
#endif

    if (error != NULL)
    {
      throw ClientException (error);
    }

    apr_array_header_t *statusarray =
      svn_sort__hash (status_hash, svn_sort_compare_items_as_paths,
                            pool);
    const svn_sort__item_t *item;
    const char *filePath;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    svn_wc_status2_t *status = NULL;
#else
    svn_wc_status_t *status = NULL;
#endif

    item = &APR_ARRAY_IDX (statusarray, 0, const svn_sort__item_t);
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    status = (svn_wc_status2_t *) item->value;
#else
    status = (svn_wc_status_t *) item->value;
#endif
    filePath = (const char *) item->key;

    return Status (filePath, status);
  };

  static Status
  remoteSingleStatus (Client * client, const QString& path,Revision revision, Context * context)
  {
    DirEntries dirEntries = client->list (path, revision, false);

    if (dirEntries.size () == 0)
      return Status ();
    else
      return dirEntryToStatus (path, dirEntries [0]);
  }

  Status
  Client::singleStatus (const QString& path,bool update,Revision revision) throw (ClientException)
  {
    if (Url::isValid (path))
      return remoteSingleStatus (this, path,revision, m_context);
    else
      return localSingleStatus (path, m_context,update);
  }

  const LogEntries *
  Client::log (const QString& path, const Revision & revisionStart,
               const Revision & revisionEnd, bool discoverChangedPaths,
               bool strictNodeHistory ) throw (ClientException)
  {
    Targets target(path);
    Pool pool;
    LogEntries * entries = new LogEntries ();
    svn_error_t *error;
    error = svn_client_log (
      target.array (pool),
      revisionStart.revision (),
      revisionEnd.revision (),
      discoverChangedPaths ? 1 : 0,
      strictNodeHistory ? 1 : 0,
      logReceiver,
      entries,
      *m_context, // client ctx
      pool);

    if (error != NULL)
    {
      delete entries;
      throw ClientException (error);
    }

    return entries;
  }

  Entry
  Client::info (const QString& path )
  {
    Pool pool;
    svn_wc_adm_access_t *adm_access;

    svn_error_t *error
      = svn_wc_adm_probe_open (&adm_access, NULL, path.utf8(), FALSE,
                                    FALSE, pool);
    if (error != NULL)
      throw ClientException (error);

    const svn_wc_entry_t *entry;
    error = svn_wc_entry (&entry, path.utf8(), adm_access, FALSE, pool);
    if (error != NULL)
      throw ClientException (error);

    // entry may be NULL
    return Entry( entry );
  }

  InfoEntries
  Client::info2(const QString& path,
                bool rec,
                const Revision & rev,
                const Revision & peg_revision) throw (ClientException)
  {
    InfoEntries ientries;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    Pool pool;
    svn_error_t *error = NULL;
    StatusEntriesBaton baton;
    apr_hash_t *status_hash;

    status_hash = apr_hash_make (pool);
    baton.hash = status_hash;
    baton.pool = pool;

    error =
      svn_client_info(path.utf8(),
                      peg_revision.revision (),
                      rev.revision (),
                      &InfoEntryFunc,
                      &baton,
                      rec,
                      *m_context,    //client ctx
                      pool);

    apr_array_header_t *statusarray =
      svn_sort__hash (status_hash, svn_sort_compare_items_as_paths,
                            pool);
    int i;

    /* Loop over array, printing each name/status-structure */
    for (i=0; i< statusarray->nelts; ++i)
    {
      const svn_sort__item_t *item;
      InfoEntry*e = NULL;
      item = &APR_ARRAY_IDX (statusarray, i, const svn_sort__item_t);
      e = (InfoEntry *) item->value;
      ientries.push_back(*e);
      delete e;
    }
    if (error != NULL)
      throw ClientException (error);
#endif
    return ientries;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
