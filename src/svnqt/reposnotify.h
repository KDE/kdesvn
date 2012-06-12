/*
    svnqt - a QT/C++ wrapper for Subversion library
    Copyright (C) 2012  Rajko Albrecht <ral@alwins-world.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef REPOSNOTIFY_H
#define REPOSNOTIFY_H

#include "svnqt/svnqt_defines.h"

#include <qstring.h>

struct svn_repos_notify_t;

namespace svn {
namespace repository {
    class ReposNotifyData;

    class SVNQT_EXPORT ReposNotify
    {
        ReposNotifyData*m_data;

    public:
        ReposNotify(const svn_repos_notify_t*notify);
        virtual ~ReposNotify();
        
        operator const QString&()const;
    };
}
}
#endif // REPOSNOTIFY_H
