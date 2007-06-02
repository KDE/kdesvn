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
#ifndef __VERSION_CHECK_HPP
#define __VERSION_CHECK_HPP

#include "svnqt/svnqt_defines.hpp"

class QString;

#define SVNQT_MAJOR 0
#define SVNQT_MINOR 8
#define SVNQT_PATCH 0

#define SVNQT_VERSIONSTRING "0.8.0"

namespace svn {
    class SVNQT_EXPORT Version {

    public:
        Version(){}
        ~Version(){}

        static bool client_version_compatible();
        static const QString linked_version();
        static const QString running_version();

        static int version_major();
        static int version_minor();
    };
}

namespace svnqt {
    class SvnqtVersion {

    public:
        struct SvnqtVersionTag{
            int _major;
            int _minor;
            int _patch;
        };

        SvnqtVersion();
        static int version_major();
        static int version_minor();
        static int version_patch();
        static bool compatible(const SvnqtVersionTag&running);
    };
}

#endif
