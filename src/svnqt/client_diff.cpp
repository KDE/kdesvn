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
#include <svn_path.h>
// apr
#include <apr_xlate.h>

#include "exception.h"
#include "pool.h"
#include "status.h"
#include "svnqt_defines.h"
#include "helper.h"
#include "diff_data.h"
#include "client_parameter.h"

namespace svn
{
QByteArray
Client_impl::diff_peg(const DiffParameter &options)
{
    Pool pool;
    svn_error_t *error;
    const apr_array_header_t *_options;

    // svn_client_diff needs an options array, even if it is empty
    _options = options.extra().array(pool);
    DiffData ddata(options.tmpPath(), options.path1(), options.rev1(), options.path1(), options.rev2());

#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    error = svn_client_diff_peg6(
#else
    error = svn_client_diff_peg5(
#endif
                _options,
                options.path1().cstr(),
                options.peg(),
                ddata.r1().revision(),
                ddata.r2().revision(),
                options.relativeTo().length() > 0 ? options.relativeTo().cstr() : QByteArray(/*0*/),
                internal::DepthToSvn(options.depth()),
                options.ignoreAncestry(),
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                false, // TODO: no_diff_added
#endif
                options.noDiffDeleted(),
                options.copies_as_adds(),
                options.ignoreContentType(),
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                false, // TODO: ignore_properties
                false, // TODO: properties_only
#endif
                options.git_diff_format(),
                APR_LOCALE_CHARSET,
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                ddata.outStream(),
                ddata.errStream(),
#else
                ddata.outFile(),
                ddata.errFile(),
#endif
                options.changeList().array(pool),
                *m_context,
                pool
            );

    if (error != nullptr) {
        throw ClientException(error);
    }
    return ddata.content();
}

QByteArray
Client_impl::diff(const DiffParameter &options)
{

    Pool pool;
    svn_error_t *error;
    const apr_array_header_t *_options;

    // svn_client_diff needs an options array, even if it is empty
    if (options.extra().isNull()) {
        _options = apr_array_make(pool, 0, 0);
    } else {
        _options = options.extra().array(pool);
    }
    DiffData ddata(options.tmpPath(), options.path1(), options.rev1(), options.path2(), options.rev2());

#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    error = svn_client_diff6(_options,
#else
    error = svn_client_diff5(_options,
#endif
                             options.path1().cstr(),
                             ddata.r1().revision(),
                             options.path2().cstr(),
                             ddata.r2().revision(),
                             options.relativeTo().length() > 0 ? options.relativeTo().cstr() : QByteArray(/*0*/),
                             internal::DepthToSvn(options.depth()),
                             options.ignoreAncestry(),
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                             false, // TODO: no_diff_added
#endif
                             options.noDiffDeleted(),
                             options.copies_as_adds(),
                             options.ignoreContentType(),
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                             false, // TODO: ignore_properties
                             false, // TODO: properties_only
#endif
                             options.git_diff_format(),
                             APR_LOCALE_CHARSET,
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
                             ddata.outStream(),
                             ddata.errStream(),
#else
                             ddata.outFile(),
                             ddata.errFile(),
#endif
                             options.changeList().array(pool),
                             *m_context,
                             pool);

    if (error != nullptr) {
        throw ClientException(error);
    }
    return ddata.content();

}
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
