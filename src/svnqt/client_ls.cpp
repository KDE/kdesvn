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

  DirEntries
  Client_impl::list(const Path& pathOrUrl,
                const Revision& revision,
                const Revision& peg,
                Depth depth,bool retrieve_locks) throw (ClientException)
  {

      sBaton _baton;
      Pool pool;
      DirEntries entries;
      _baton.m_data = &entries;
      _baton.m_context=m_context;
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
      if (error != 0) {
          throw ClientException (error);
      }
      return entries;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
