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

#ifndef SVNQT_POOL_H
#define SVNQT_POOL_H

// subversion api
#include <svn_pools.h>

namespace svn
{
/**
 * Class for encapsulation of apr/subversion pools
 */
class Pool
{
public:
    /**
     * creates a subpool new pool to an existing pool
     *
     * @param parent NULL -> global pool
     */
    explicit Pool(apr_pool_t *parent = nullptr);
    Pool &operator=(const Pool &) = delete;
    Pool(const Pool &) = delete;

    ~Pool();

    /**
     * @return apr handle to the pool
     */
    apr_pool_t *pool() const;

    /**
     * operator to return apr handle to the pool
     */
    operator apr_pool_t *() const
    {
        return m_pool;
    }

    /**
     * release pool and create a new one
     */
    void renew();

private:
    apr_pool_t *m_parent;
    apr_pool_t *m_pool;

    static bool s_initialized;
    static apr_pool_t *pool_create(apr_pool_t *parent);
};
}

#endif

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
