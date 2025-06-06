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

// Subversion api
#include <svn_client.h>

#include "exception.h"
#include "pool.h"
#include "status.h"
#include "svnfilestream.h"
#include "svnqt_defines.h"
#include "svnstream.h"

namespace svn
{
QByteArray Client_impl::cat(const Path &path, const Revision &revision, const Revision &peg_revision)
{
    svn::stream::SvnByteStream buffer(*m_context);
    svn_error_t *error = internal_cat(path, revision, peg_revision, buffer);
    if (error != nullptr) {
        throw ClientException(error);
    }

    return buffer.content();
}

void Client_impl::cat(svn::stream::SvnStream &buffer, const Path &path, const Revision &revision, const Revision &peg_revision)
{
    svn_error_t *error = internal_cat(path, revision, peg_revision, buffer);
    if (error != nullptr) {
        throw ClientException(error);
    }
}

void Client_impl::get(const Path &path, const QString &target, const Revision &revision, const Revision &peg_revision)
{
    svn::stream::SvnFileOStream buffer(target, *m_context);
    svn_error_t *error = internal_cat(path, revision, peg_revision, buffer);
    if (error != nullptr) {
        throw ClientException(error);
    }
}

svn_error_t *Client_impl::internal_cat(const Path &path, const Revision &revision, const Revision &peg_revision, svn::stream::SvnStream &buffer)
{
    Pool pool;
    Pool scratch_pool;
    apr_hash_t *props = nullptr;
    return svn_client_cat3(&props, buffer, path.path().toUtf8(), peg_revision.revision(), revision.revision(), true, *m_context, pool, scratch_pool);
}

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
