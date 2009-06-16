/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2008 by Rajko Albrecht (ral@alwins-world.de)
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
#include "svnqt/helper.hpp"

// Subversion api
#include "svn_client.h"
#include "svn_sorts.h"
#include "svn_path.h"
//#include "svn_utf.h"

#include "svnqt/dirent.hpp"
#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/status.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/info_entry.hpp"
#include "svnqt/url.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "svnqt/context_listener.hpp"

namespace svn
{

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    static svn_error_t *
    logReceiver2(
                 void*baton,
                 svn_log_entry_t *log_entry,
                 apr_pool_t * pool
                )
    {
        Q_UNUSED(pool);
        Client_impl::sBaton * l_baton = (Client_impl::sBaton*)baton;
        LogEntries * entries =
                (LogEntries *) l_baton->m_data;
        QLIST<QLONG>*rstack=
                (QLIST<QLONG>*)l_baton->m_revstack;
        Context*l_context = l_baton->m_context;
        svn_client_ctx_t*ctx = l_context->ctx();
        if (ctx&&ctx->cancel_func) {
            SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
        }
        if (! SVN_IS_VALID_REVNUM(log_entry->revision))
        {
            if (rstack&&rstack->size()>0) {
                rstack->pop_front();
            }
            return SVN_NO_ERROR;
        }
        entries->insert (entries->begin (), LogEntry (log_entry));
        if (rstack) {
            entries->first().m_MergedInRevisions=(*rstack);
            if (log_entry->has_children) {
                rstack->push_front(log_entry->revision);
            }
        }
        return SVN_NO_ERROR;
    }
#else
  static svn_error_t *
  logReceiver (
                 void *baton,
                 apr_hash_t * changedPaths,
                 svn_revnum_t rev,
                 const char *author,
                 const char *date,
                 const char *msg,
                 apr_pool_t * pool
              )
  {
    Client_impl::sBaton * l_baton = (Client_impl::sBaton*)baton;
    LogEntries * entries =
      (LogEntries *) l_baton->m_data;

    /* check every loop for cancel of operation */
    Context*l_context = l_baton->m_context;
    svn_client_ctx_t*ctx = l_context->ctx();
    if (ctx&&ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }
    entries->insert (entries->begin (), LogEntry (rev, author, date, msg));
    if (changedPaths != NULL)
    {
      LogEntry &entry = entries->front ();

      for (apr_hash_index_t *hi = apr_hash_first (pool, changedPaths);
           hi != NULL;
           hi = apr_hash_next (hi))
      {
        const void *pv;
        void *val;
        apr_hash_this (hi, &pv, NULL, &val);

        svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);
        const char* path = reinterpret_cast<const char*>(pv);

        entry.changedPaths.push_back (
              LogChangePathEntry (path,
                                  log_item->action,
                                  log_item->copyfrom_path,
                                  log_item->copyfrom_rev) );

      }
    }

    return SVN_NO_ERROR;
  }
#endif

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
  static svn_error_t *
          logMapReceiver2(
                       void*baton,
                       svn_log_entry_t *log_entry,
                       apr_pool_t * pool
                      )
  {
      Q_UNUSED(pool);
      Client_impl::sBaton * l_baton = (Client_impl::sBaton*)baton;
      LogEntriesMap * entries =
              (LogEntriesMap *) l_baton->m_data;
      Context*l_context = l_baton->m_context;
      QLIST<QLONG>*rstack=
              (QLIST<QLONG>*)l_baton->m_revstack;
      svn_client_ctx_t*ctx = l_context->ctx();
      if (ctx&&ctx->cancel_func) {
          SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
      }
      if (! SVN_IS_VALID_REVNUM(log_entry->revision))
      {
          if (rstack&&rstack->size()>0) {
              rstack->pop_front();
          }
          return SVN_NO_ERROR;
      }
      (*entries)[log_entry->revision]=LogEntry (log_entry);
      /// @TODO insert it into last logentry
      if (rstack) {
          (*entries)[log_entry->revision].m_MergedInRevisions=(*rstack);
          if (log_entry->has_children) {
              rstack->push_front(log_entry->revision);
          }
      }
      return SVN_NO_ERROR;
  }
#else
  static svn_error_t *
  logMapReceiver (void *baton,
                   apr_hash_t * changedPaths,
                   svn_revnum_t rev,
                   const char *author,
                   const char *date,
                   const char *msg,
                   apr_pool_t * pool
                 )
  {
    Client_impl::sBaton * l_baton = (Client_impl::sBaton*)baton;
    LogEntriesMap * entries =
      (LogEntriesMap *) l_baton->m_data;

    /* check every loop for cancel of operation */
    Context*l_context = l_baton->m_context;
    svn_client_ctx_t*ctx = l_context->ctx();
    if (ctx&&ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }
    (*entries)[rev]=LogEntry(rev, author, date, msg);
    if (changedPaths != NULL)
    {
      LogEntry &entry = (*entries)[rev];

      for (apr_hash_index_t *hi = apr_hash_first (pool, changedPaths);
           hi != NULL;
           hi = apr_hash_next (hi))
      {
        const void *pv;
        void *val;
        apr_hash_this (hi, &pv, NULL, &val);

        svn_log_changed_path_t *log_item = reinterpret_cast<svn_log_changed_path_t *> (val);
        const char* path = reinterpret_cast<const char*>(pv);

        entry.changedPaths.push_back (
              LogChangePathEntry (path,
                                  log_item->action,
                                  log_item->copyfrom_path,
                                  log_item->copyfrom_rev) );

      }
    }

    return NULL;
  }
#endif

  struct StatusEntriesBaton {
    StatusEntries entries;
    apr_pool_t* pool;
    Context*m_Context;
    StatusEntriesBaton():entries() {
        pool = 0;
        m_Context = 0;
    }
  };

  struct InfoEntriesBaton {
    InfoEntries entries;
    apr_pool_t* pool;
    Context*m_Context;
    InfoEntriesBaton():entries() {
        pool = 0;
        m_Context = 0;
    }
  };

  static svn_error_t * InfoEntryFunc(void*baton,
                            const char*path,
                            const svn_info_t*info,
                            apr_pool_t *)
  {
    InfoEntriesBaton* seb = (InfoEntriesBaton*)baton;
    if (seb->m_Context) {
        /* check every loop for cancel of operation */
        Context*l_context = seb->m_Context;
        svn_client_ctx_t*ctx = l_context->ctx();
        if (ctx&&ctx->cancel_func) {
            SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
        }
    }
    seb->entries.push_back(InfoEntry(info,path));
    return NULL;
  }

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6)) || (SVN_VER_MAJOR > 1)
  static svn_error_t* StatusEntriesFunc (void *baton,
                                 const char *path,
                                 svn_wc_status2_t *status,
                                 apr_pool_t *pool)
  {
        // use own pool - the parameter will cleared between loops!
        Q_UNUSED(pool);
        StatusEntriesBaton* seb = (StatusEntriesBaton*)baton;
        if (seb->m_Context) {
            /* check every loop for cancel of operation */
            Context*l_context = seb->m_Context;
            svn_client_ctx_t*ctx = l_context->ctx();
            if (ctx&&ctx->cancel_func) {
                SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
            }
        }

        seb->entries.push_back (StatusPtr(new Status(path,status)));
        return NULL;
  }
#else
  static void StatusEntriesFunc (void *baton,
                                 const char *path,
                                 svn_wc_status2_t *status)
  {
      StatusEntriesBaton* seb = (StatusEntriesBaton*)baton;
      seb->entries.push_back (StatusPtr(new Status(path,status)));
  }
#endif

  static StatusEntries
  localStatus (const Path& path,
               Depth depth,
               const bool get_all,
               const bool update,
               const bool no_ignore,
               const bool hide_externals,
               const StringArray & changelists,
               Context * context)
  {
    svn_error_t *error;
    StatusEntries entries;
    svn_revnum_t revnum;
    Revision rev (Revision::HEAD);
    Pool pool;
    StatusEntriesBaton baton;

    baton.pool = pool;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6)) || (SVN_VER_MAJOR > 1)
    error = svn_client_status4 (
#else
    error = svn_client_status3 (
#endif
        &revnum,           // revnum
        path.path().TOUTF8(),         // path
        rev,
        StatusEntriesFunc, // status func
        &baton,            // status baton
        internal::DepthToSvn(depth), // see svn::Depth
        get_all,           // get all not only interesting
        update,            // check for updates
        no_ignore,         // hide ignored files or not
        hide_externals,    // hide external
        changelists.array(pool),
        *context,          //client ctx
        pool);
#else
    Q_UNUSED(changelists);
    error = svn_client_status2 (
      &revnum,      // revnum
      path.path().TOUTF8(),         // path
      rev,
      StatusEntriesFunc, // status func
      &baton,        // status baton
      (depth==DepthInfinity), //recurse
      get_all,       // get all not only interesting
      update,        // check for updates
      no_ignore,     // hide ignored files or not
      hide_externals, // hide external
      *context,    //client ctx
      pool);
#endif

    Client_impl::checkErrorThrow(error);
    return baton.entries;
  }

  static StatusPtr
  dirEntryToStatus (const Path& path, DirEntryPtr dirEntry)
  {
    QString url = path.path();
    url += QString::FROMUTF8("/");
    url += dirEntry->name();
    return StatusPtr(new Status (url, dirEntry));
  }

  static StatusPtr
  infoEntryToStatus(const Path&,const InfoEntry&infoEntry)
  {
    return StatusPtr(new Status(infoEntry.url(),infoEntry));
  }

  static StatusEntries
  remoteStatus (Client * client,
                const Path& path,
                Depth depth,
                const bool ,
                const bool ,
                const bool ,
                const Revision revision,
                Context * ,
                bool detailed_remote)
  {
    DirEntries dirEntries = client->list(path, revision, revision, depth,detailed_remote);
    DirEntries::const_iterator it;

    StatusEntries entries;
    QString url = path.path();
    url+=QString::FROMUTF8("/");

    for (it = dirEntries.begin (); it != dirEntries.end (); ++it)
    {
        DirEntryPtr dirEntry = *it;
        if (dirEntry->name().isEmpty())
            continue;
        entries.push_back(dirEntryToStatus (path, dirEntry));
    }
    return entries;
  }

  StatusEntries
  Client_impl::status (const Path& path,
                  Depth depth,
                  const bool get_all,
                  const bool update,
                  const bool no_ignore,
                  const Revision revision,
                  bool detailed_remote,
                  const bool hide_externals,
                  const StringArray & changelists) throw (ClientException)
  {
    if (Url::isValid (path.path())) {
        return remoteStatus (this, path, depth, get_all, update,
                            no_ignore,revision,m_context,detailed_remote);
    } else {
        return localStatus (path, depth, get_all, update,
                            no_ignore, hide_externals,changelists, m_context);
    }
  }

  static StatusPtr
  localSingleStatus (const Path& path, Context * context,bool update=false)
  {
    svn_error_t *error;
    Pool pool;
    StatusEntriesBaton baton;
    svn_revnum_t revnum;
    Revision rev (Revision::HEAD);

    baton.pool = pool;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6)) || (SVN_VER_MAJOR > 1)
    error = svn_client_status4 (
#else
    error = svn_client_status3 (
#endif
        &revnum,           // revnum
        path.path().TOUTF8(),         // path
        rev,
        StatusEntriesFunc, // status func
        &baton,            // status baton
        svn_depth_empty, // not recurse
        true,           // get all not only interesting
        update,         // check for updates
        false,          // hide ignored files or not
        false,          // hide external
        0,
        *context,          //client ctx
        pool);
#else
    error = svn_client_status2 (
      &revnum,      // revnum
      path.path().TOUTF8(),         // path
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
#endif
    Client_impl::checkErrorThrow(error);

    return baton.entries[0];
  };

  static StatusPtr
  remoteSingleStatus (Client * client, const Path& path,const Revision revision, Context * )
  {
    InfoEntries infoEntries = client->info(path,DepthEmpty,revision,Revision(Revision::UNDEFINED));
    if (infoEntries.size () == 0)
      return StatusPtr(new Status());
    else
      return infoEntryToStatus (path, infoEntries [0]);
  }

  StatusPtr
  Client_impl::singleStatus (const Path& path,bool update,const Revision revision) throw (ClientException)
  {
    if (Url::isValid (path.path()))
      return remoteSingleStatus (this, path,revision, m_context);
    else
      return localSingleStatus (path, m_context,update);
  }

  bool
  Client_impl::log (const Path& path, const Revision & revisionStart,
       const Revision & revisionEnd,
       LogEntriesMap&log_target,
       const Revision & revisionPeg,
       bool discoverChangedPaths,
       bool strictNodeHistory,int limit,
       bool include_merged_revisions,
       const StringArray&revprops
                   ) throw (ClientException)
  {
    Targets target(path);
    Pool pool;
    sBaton l_baton;
    QLIST<QLONG> revstack;
    l_baton.m_context=m_context;
    l_baton.m_data = &log_target;
    l_baton.m_revstack = &revstack;
    svn_error_t *error;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_log4 (
                target.array (pool),
                revisionPeg.revision(),
                revisionStart.revision (),
                revisionEnd.revision (),
                limit,
                discoverChangedPaths ? 1 : 0,
                strictNodeHistory ? 1 : 0,
                include_merged_revisions?1:0,
                revprops.array(pool),
                logMapReceiver2,
                &l_baton,
                *m_context, // client ctx
                pool);
#elif ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
    Q_UNUSED(include_merged_revisions);
    Q_UNUSED(revprops);

    error = svn_client_log3 (
      target.array (pool),
      revisionPeg.revision(),
      revisionStart.revision (),
      revisionEnd.revision (),
      limit,
      discoverChangedPaths ? 1 : 0,
      strictNodeHistory ? 1 : 0,
      logMapReceiver,
      &l_baton,
      *m_context, // client ctx
      pool);
#else
    Q_UNUSED(include_merged_revisions);
    Q_UNUSED(revprops);
    Q_UNUSED(revisionPeg);

    error = svn_client_log2 (
      target.array (pool),
      revisionStart.revision (),
      revisionEnd.revision (),
      limit,
      discoverChangedPaths ? 1 : 0,
      strictNodeHistory ? 1 : 0,
      logMapReceiver,
      &l_baton,
      *m_context, // client ctx
      pool);
#endif
    checkErrorThrow(error);
    return true;
  }

  LogEntriesPtr
  Client_impl::log (const Path& path, const Revision & revisionStart,
                    const Revision & revisionEnd, const Revision & revisionPeg,
                    bool discoverChangedPaths,
                    bool strictNodeHistory,int limit,
                    bool include_merged_revisions,
                    const StringArray&revprops
                   ) throw (ClientException)
  {
    Targets target(path);
    Pool pool;
    LogEntriesPtr entries = LogEntriesPtr(new LogEntries ());
    QLIST<QLONG> revstack;
    sBaton l_baton;
    l_baton.m_context=m_context;
    l_baton.m_data = entries;
    l_baton.m_revstack = &revstack;

    svn_error_t *error;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_log4 (
        target.array (pool),
        revisionPeg.revision(),
        revisionStart.revision (),
        revisionEnd.revision (),
        limit,
        discoverChangedPaths ? 1 : 0,
        strictNodeHistory ? 1 : 0,
        include_merged_revisions?1:0,
        revprops.array(pool),
        logReceiver2,
        &l_baton,
        *m_context, // client ctx
        pool);
#elif ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
    Q_UNUSED(include_merged_revisions);
    Q_UNUSED(revprops);

    error = svn_client_log3 (
      target.array (pool),
      revisionPeg.revision(),
      revisionStart.revision (),
      revisionEnd.revision (),
      limit,
      discoverChangedPaths ? 1 : 0,
      strictNodeHistory ? 1 : 0,
      logReceiver,
      &l_baton,
      *m_context, // client ctx
      pool);
#else
    Q_UNUSED(include_merged_revisions);
    Q_UNUSED(revprops);
    Q_UNUSED(revisionPeg);

    error = svn_client_log2 (
      target.array (pool),
      revisionStart.revision (),
      revisionEnd.revision (),
      limit,
      discoverChangedPaths ? 1 : 0,
      strictNodeHistory ? 1 : 0,
      logReceiver,
      &l_baton,
      *m_context, // client ctx
      pool);
#endif
    checkErrorThrow(error);
    return entries;
  }

  InfoEntries
  Client_impl::info(const Path& _p,
                    Depth depth,
                const Revision & rev,
                const Revision & peg_revision,
                const StringArray&changelists
                    ) throw (ClientException)
  {

    Pool pool;
    svn_error_t *error = NULL;
    InfoEntriesBaton baton;

    baton.pool = pool;
    baton.m_Context=m_context;
    svn_opt_revision_t pegr;
    const char *truepath = 0;
    bool internal_peg = false;
    QByteArray _buf = _p.cstr();

    error = svn_opt_parse_path(&pegr, &truepath,
                                 _buf,
                                 pool);
    checkErrorThrow(error);
    if (peg_revision.kind() == svn_opt_revision_unspecified) {
        if ((svn_path_is_url (_p.cstr())) && (pegr.kind == svn_opt_revision_unspecified)) {
            pegr.kind = svn_opt_revision_head;
            internal_peg=true;
        }
    }

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error =
            svn_client_info2(truepath,
                        internal_peg?&pegr:peg_revision.revision(),
                        rev.revision (),
                        &InfoEntryFunc,
                        &baton,
                        internal::DepthToSvn(depth),
                        changelists.array(pool),
                        *m_context,    //client ctx
                        pool);
#else
    bool rec = depth==DepthInfinity;
    Q_UNUSED(changelists);
    error =
            svn_client_info(truepath,
                      internal_peg?&pegr:peg_revision.revision(),
                      rev.revision (),
                      &InfoEntryFunc,
                      &baton,
                      rec,
                      *m_context,    //client ctx
                      pool);
#endif

    checkErrorThrow(error);
    return baton.entries;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
