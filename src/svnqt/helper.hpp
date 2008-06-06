
/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef __HELPER_HPP
#define __HELPER_HPP

#include "svnqttypes.hpp"
#include "revision.hpp"
#include <svn_types.h>

namespace svn
{
    namespace internal
    {
        class DepthToSvn
        {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
            protected:
                svn_depth_t _value;
            public:
                DepthToSvn(const svn::Depth&val):_value(svn_depth_unknown)
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
                        default:
                            _value =  svn_depth_infinity;
                            break;
                    }
                }

                operator svn_depth_t ()
                {
                    return _value;
                }
#endif
        };

        class RevisionRangesToHash
        {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
            protected:
                RevisionRanges m_ranges;
            public:
                RevisionRangesToHash(const RevisionRanges&_input):m_ranges(_input){}

                apr_array_header_t*array(const Pool&pool)
                {
                    apr_array_header_t*ranges=apr_array_make(pool,0,sizeof(svn_opt_revision_range_t *));
                    svn_opt_revision_range_t *range;

                    for (unsigned long j=0;j<m_ranges.count();++j)
                    {
                        range = (svn_opt_revision_range_t *)apr_palloc(pool, sizeof(*range));
                        range->start= *m_ranges[j].first.revision();
                        range->end  = *m_ranges[j].second.revision();
                        APR_ARRAY_PUSH(ranges,svn_opt_revision_range_t *) = range;
                    }
                    return ranges;
                }
#endif
        };
    }
}
#endif
