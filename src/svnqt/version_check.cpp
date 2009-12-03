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
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "version_check.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <svn_version.h>
#include <svn_client.h>

#include <qstring.h>

namespace svn {
    static const svn_version_t Linkedtag = {
        SVN_VER_MAJOR,
        SVN_VER_MINOR,
        SVN_VER_PATCH,
        SVN_VER_NUMTAG
    };

    static QString curr_version_string;

    bool Version::client_version_compatible()
    {
        return svn_ver_compatible(svn_client_version(),&Linkedtag);
    }

    const QString Version::linked_version()
    {
        return QString( SVN_VERSION );
    }

    const QString Version::running_version()
    {
        if (curr_version_string.length()==0) {
            curr_version_string =
                QString("%1.%2.%3.%4").arg(svn_client_version()->major).arg(svn_client_version()->minor)
                    .arg(svn_client_version()->patch).arg(svn_client_version()->tag);
        }
        return curr_version_string;
    }

    int Version::version_major()
    {
      return svn_client_version()->major;
    }

    int Version::version_minor()
    {
      return svn_client_version()->minor;
    }
}
