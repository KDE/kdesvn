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
#include "svn_path.h"
//#include "svn_utf.h"

// svncpp
#include "client.hpp"
#include "dirent.hpp"
#include "exception.hpp"
#include "pool.hpp"
#include "status.hpp"
#include "targets.hpp"
#include "info_entry.hpp"
#include "url.hpp"
#include "svncpp_defines.hpp"

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

  static void StatusEntriesFunc (void *baton,
                                 const char *path,
                                 svn_wc_status2_t *status)
  {
    svn_wc_status2_t* stat;
      StatusEntriesBaton* seb = (StatusEntriesBaton*)baton;

      path = apr_pstrdup (seb->pool, path);
      stat = svn_wc_dup_status2 (status, seb->pool);
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
    error = svn_client_status2 (
      &revnum,      // revnum
      path.TOUTF8(),         // path
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
      svn_wc_status2_t *status = NULL;

      item = &APR_ARRAY_IDX (statusarray, i, const svn_sort__item_t);
      status = (svn_wc_status2_t *) item->value;

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


    e->name = apr_pstrdup(pool,dirEntry.name().TOUTF8());
    e->url = apr_pstrdup(pool,url.TOUTF8());
    e->revision = dirEntry.createdRev ();
    e->kind = dirEntry.kind ();
    e->schedule = svn_wc_schedule_normal;
    e->text_time = dirEntry.time ();
    e->prop_time = dirEntry.time ();
    e->cmt_rev = dirEntry.createdRev ();
    e->cmt_date = dirEntry.time ();
#if QT_VERSION < 0x040000
    e->cmt_author = dirEntry.lastAuthor ();
#else
    e->cmt_author = dirEntry.lastAuthor().toLocal8Bit();
#endif

    svn_wc_status2_t * s =
      static_cast<svn_wc_status2_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_status2_t)));
    s->entry = e;
    s->text_status = svn_wc_status_normal;
    s->prop_status = svn_wc_status_normal;
    s->locked = 0;
    s->switched = 0;
    s->repos_text_status = svn_wc_status_normal;
    s->repos_prop_status = svn_wc_status_normal;
    s->repos_lock = 0;
    return Status (url, s);
  }

  static Status
  infoEntryToStatus(const QString&path,const InfoEntry&infoEntry)
  {
    Pool pool;
    svn_wc_entry_t * e =
      static_cast<svn_wc_entry_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_entry_t)));

    QString url = path;
    url += "/";
    url += infoEntry.Name();

    e->name = apr_pstrdup(pool,infoEntry.Name().TOUTF8());
    e->url = apr_pstrdup(pool,url.TOUTF8());
    e->revision = infoEntry.revision();
    e->kind = infoEntry.kind ();
    e->schedule = svn_wc_schedule_normal;
    e->text_time = infoEntry.textTime ();
    e->prop_time = infoEntry.propTime ();
    e->cmt_rev = infoEntry.cmtRev ();
    e->cmt_date = infoEntry.cmtDate();

#if QT_VERSION < 0x040000
    e->cmt_author = infoEntry.cmtAuthor();
#else
    e->cmt_author = infoEntry.cmtAuthor().toLocal8Bit();
#endif
    svn_wc_status2_t * s =
      static_cast<svn_wc_status2_t *> (
        apr_pcalloc (pool, sizeof (svn_wc_status2_t)));
    s->entry = e;
    s->text_status = svn_wc_status_normal;
    s->prop_status = svn_wc_status_normal;
    s->locked = infoEntry.lockEntry().Locked();
    if (s->locked) {
        svn_lock_t*l =
            static_cast<svn_lock_t *> (
            apr_pcalloc (pool, sizeof (svn_lock_t)));
        l->token = apr_pstrdup(pool,infoEntry.lockEntry().Token().TOUTF8());
        l->path = apr_pstrdup(pool,path.TOUTF8());
        l->owner = apr_pstrdup(pool,infoEntry.lockEntry().Owner().TOUTF8());
        l->comment = apr_pstrdup(pool,infoEntry.lockEntry().Comment().TOUTF8());
        l->creation_date = infoEntry.lockEntry().Date();
        l->expiration_date = infoEntry.lockEntry().Expiration();
    } else {
        s->repos_lock = 0;
    }
    s->switched = 0;
    s->repos_text_status = svn_wc_status_normal;
    s->repos_prop_status = svn_wc_status_normal;

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

    error = svn_client_status2 (
      &revnum,      // revnum
      path.TOUTF8(),         // path
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

    if (error != NULL)
    {
      throw ClientException (error);
    }

    apr_array_header_t *statusarray =
      svn_sort__hash (status_hash, svn_sort_compare_items_as_paths,
                            pool);
    const svn_sort__item_t *item;
    const char *filePath;
    svn_wc_status2_t *status = NULL;

    item = &APR_ARRAY_IDX (statusarray, 0, const svn_sort__item_t);
    status = (svn_wc_status2_t *) item->value;
    filePath = (const char *) item->key;

    return Status (filePath, status);
  };

  static Status
  remoteSingleStatus (Client * client, const QString& path,Revision revision, Context * context)
  {
    InfoEntries infoEntries = client->info(path,false,revision,Revision(Revision::UNDEFINED));
    if (infoEntries.size () == 0)
      return Status ();
    else
      return infoEntryToStatus (path, infoEntries [0]);
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
               bool strictNodeHistory,int limit) throw (ClientException)
  {
    Targets target(path);
    Pool pool;
    LogEntries * entries = new LogEntries ();
    svn_error_t *error;

    error = svn_client_log2 (
      target.array (pool),
      revisionStart.revision (),
      revisionEnd.revision (),
      limit,
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

  InfoEntries
  Client::info(const QString& path,
                bool rec,
                const Revision & rev,
                const Revision & peg_revision) throw (ClientException)
  {
    InfoEntries ientries;
    Pool pool;
    svn_error_t *error = NULL;
    StatusEntriesBaton baton;
    apr_hash_t *status_hash;

    status_hash = apr_hash_make (pool);
    baton.hash = status_hash;
    baton.pool = pool;
    svn_opt_revision_t pegr;
    const char *truepath;
    error = svn_opt_parse_path (&pegr, &truepath,
                                 path.TOUTF8(),
                                 pool);
    if (error != NULL)
      throw ClientException (error);

    if ((svn_path_is_url (path.TOUTF8())) && (pegr.kind == svn_opt_revision_unspecified))
        pegr.kind = svn_opt_revision_head;

    error =
      svn_client_info(truepath,
                      &pegr,
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
    return ientries;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
