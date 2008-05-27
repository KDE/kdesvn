
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
#include <svn_types.h>

namespace svn
{
    namespace internal
    {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        class DepthToSvn
        {
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

                svn_depth_t operator()()
                {
                    return _value;
                }
        };
#endif
    }
}
#endif
