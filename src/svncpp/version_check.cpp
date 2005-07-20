#include "svncpp/version_check.hpp"

#include <svn_version.h>
#include <svn_client.h>

#include <string>
#include <sstream>

namespace svn {
    static const svn_version_t Linkedtag = {
        SVN_VER_MAJOR,
        SVN_VER_MINOR,
        SVN_VER_PATCH,
        SVN_VER_NUMTAG
    };

    static std::string curr_version_string;

    bool Version::client_version_compatible()
    {
        return svn_ver_compatible(svn_client_version(),&Linkedtag);
    }

    const char*Version::linked_version()
    {
        return SVN_VERSION;
    }
    const char*Version::running_version()
    {
        if (curr_version_string.size()==0) {
            std::ostringstream so;
            so << svn_client_version()->major << "."
                << svn_client_version()->minor << "."
                << svn_client_version()->patch
                << svn_client_version()->tag;
            curr_version_string = so.str();
        }
        return curr_version_string.c_str();
    }
}
