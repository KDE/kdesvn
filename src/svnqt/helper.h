
/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#ifndef SVNQT_HELPER_H
#define SVNQT_HELPER_H

#include "svnqttypes.h"
#include "pool.h"
#include "revision.h"
#include <svn_string.h>
#include <svn_types.h>
#include <svn_version.h>

#include <iostream>

#define SVN_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#ifdef OVERRIDE_SVN_API_VERSION
#define SVN_API_VERSION OVERRIDE_SVN_API_VERSION
#else
#define SVN_API_VERSION (SVN_VERSION_CHECK(SVN_VER_MAJOR, SVN_VER_MINOR, SVN_VER_PATCH))
#endif

namespace svn
{
namespace internal
{
class DepthToSvn
{
protected:
    svn_depth_t _value;
public:
    explicit DepthToSvn(const svn::Depth val): _value(svn_depth_unknown)
    {
        switch (val) {
        case DepthUnknown:
            _value = svn_depth_unknown;
            break;
        case DepthExclude:
            _value =  svn_depth_exclude;
            break;
        case DepthEmpty:
            _value =  svn_depth_empty;
            break;
        case DepthFiles:
            _value =  svn_depth_files;
            break;
        case DepthImmediates:
            _value =  svn_depth_immediates;
            break;
        case DepthInfinity:
            _value =  svn_depth_infinity;
            break;
        }
    }

    operator svn_depth_t() const
    {
        return _value;
    }
};

class RevisionRangesToHash
{
protected:
    RevisionRanges m_ranges;
public:
    explicit RevisionRangesToHash(const RevisionRanges &_input): m_ranges(_input) {}

    apr_array_header_t *array(const Pool &pool)
    {
        apr_array_header_t *ranges = apr_array_make(pool, m_ranges.size(), sizeof(svn_opt_revision_range_t *));
        svn_opt_revision_range_t *range;

        for (const RevisionRange &rr : qAsConst(m_ranges)) {
            range = (svn_opt_revision_range_t *)apr_palloc(pool, sizeof(*range));
            range->start = *rr.first.revision();
            range->end  = *rr.second.revision();
            APR_ARRAY_PUSH(ranges, svn_opt_revision_range_t *) = range;
        }
        return ranges;
    }
};

class Map2Hash
{
    PropertiesMap _map;
public:
    explicit Map2Hash(const PropertiesMap &aMap): _map(aMap) {}

    apr_hash_t *hash(const Pool &pool)const
    {
        if (_map.count() == 0) {
            return nullptr;
        }
        apr_hash_t *hash = apr_hash_make(pool);
        for (auto it = _map.begin(); it != _map.end(); ++it) {
            const QByteArray s = it.value().toUtf8();
            const QByteArray n = it.key().toUtf8();
            const char *propval = apr_pstrndup(pool, s, s.size());
            const char *propname = apr_pstrndup(pool, n, n.size());
            apr_hash_set(hash, propname, APR_HASH_KEY_STRING, propval);
        }
        return hash;
    }
};

class Hash2Map
{
    PropertiesMap _map;
public:
    Hash2Map(apr_hash_t *hash, apr_pool_t *pool)
        : _map()
    {
        if (hash != nullptr) {
            apr_hash_index_t *hi;
            for (hi = apr_hash_first(pool, hash); hi;
                    hi = apr_hash_next(hi)) {
                const void *key;
                void *val;

                apr_hash_this(hi, &key, nullptr, &val);
                const char *_k = (const char *)key;
                const char *_v = ((const svn_string_t *)val)->data;

                _map[ QString::fromUtf8(_k)] =
                    QString::fromUtf8(_v);
            }
        }
    }

    operator const PropertiesMap &()const
    {
        return _map;
    }
};
}
}
#endif
