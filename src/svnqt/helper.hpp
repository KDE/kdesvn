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
                svn::Depth _input;
            public:
                DepthToSvn(const svn::Depth&val):_input(val){}

                svn_depth_t operator()()
                {
                    switch (_input) {
                        case DepthUnknown:
                            return svn_depth_unknown;
                        case DepthExclude:
                            return svn_depth_exclude;
                        case DepthEmpty:
                            return svn_depth_empty;
                        case DepthFiles:
                            return svn_depth_files;
                        case DepthImmediates:
                            return svn_depth_immediates;
                        case DepthInfinity:
                        default:
                            return svn_depth_infinity;
                    }
                }
        };
#endif
    }
}
#endif
