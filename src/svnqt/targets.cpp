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

#include "targets.h"

// subversion api
#include <svn_types.h>

// apr api
#include <apr_pools.h>
#include <apr_strings.h>

// svncpp
#include "path.h"
#include "pool.h"
#include "svnqt_defines.h"

#include <QStringList>
#include <QUrl>

namespace svn
{
Targets::Targets(const svn::Paths &targets)
    : m_targets(targets)
{
}

Targets::Targets(const QString &target)
{
    if (!target.isEmpty()) {
        m_targets.push_back(target);
    }
}

Targets::Targets(const Path &target)
{
    if (!target.cstr().isEmpty()) {
        m_targets.push_back(target);
    }
}

apr_array_header_t *
Targets::array(const Pool &pool) const
{
    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_targets =
        apr_array_make(apr_pool,
                       m_targets.size(),
                       sizeof(const char *));

    for (const svn::Path &tgt : m_targets) {
        const QByteArray s = tgt.path().toUtf8();
        char *t2 = apr_pstrndup(apr_pool, s.data(), s.size());
        (*((const char **) apr_array_push(apr_targets))) = t2;
    }

    return apr_targets;
}

const Path Targets::target(Paths::size_type which) const
{
    if (m_targets.size() > which) {
        return m_targets[which];
    } else {
        return Path();
    }
}

svn::Targets Targets::fromStringList(const QStringList &paths)
{
    svn::Paths ret;
    ret.reserve(paths.size());
    for (const QString &path : paths)
        ret.push_back(svn::Path(path));
    return svn::Targets(ret);
}

svn::Targets Targets::fromUrlList(const QList<QUrl> &urls, UrlConversion conversion)
{
    svn::Paths ret;
    ret.reserve(urls.size());
    const bool preferLocalFile = conversion == UrlConversion::PreferLocalPath;
    for (const QUrl &url : urls)
        ret.push_back(svn::Path((preferLocalFile && url.isLocalFile()) ? url.toLocalFile() : url.url()));
    return svn::Targets(ret);
}

}
