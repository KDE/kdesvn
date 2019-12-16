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


// svncpp
#include "client_impl.h"

// subversion api
#include <svn_client.h>
#include <svn_path.h>
#include <svn_sorts.h>

#include "dirent.h"
#include "exception.h"
#include "svnqt_defines.h"
#include "helper.h"

namespace svn
{

struct ListBaton {
    ContextWP context;
    DirEntries dirEntries;
};

static svn_error_t *s_list_func
(void *baton, const char *path, const svn_dirent_t *dirent, const svn_lock_t *lock, const char *abs_path, apr_pool_t *)
{
    Q_UNUSED(abs_path);
    if (!baton || !path || !dirent) {
        return nullptr;
    }
    /* check every loop for cancel of operation */
    ListBaton *l_baton = static_cast<ListBaton *>(baton);
    ContextP l_context = l_baton->context;
    if (l_context.isNull()) {
        return SVN_NO_ERROR;
    }
    svn_client_ctx_t *ctx = l_context->ctx();
    if (ctx && ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }
    l_context->contextAddListItem(&l_baton->dirEntries, dirent, lock, QString::fromUtf8(path));
    return nullptr;
}

DirEntries
Client_impl::list(const Path &pathOrUrl,
                  const Revision &revision,
                  const Revision &peg,
                  Depth depth, bool retrieve_locks)
{

    ListBaton _baton;
    Pool pool;
    // todo svn 1.8: svn_client_list3
    _baton.context = m_context;
    svn_error_t *error = svn_client_list2(pathOrUrl.cstr(),
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
    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.dirEntries;
}
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
