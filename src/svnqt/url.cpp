/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
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

#include "url.h"

// svncpp
#include "pool.h"
#include "svnqt_defines.h"
#include "helper.h"

#include <svn_dirent_uri.h>

#include <qglobal.h>

namespace svn
{

Url::Url(const QUrl &url)
    : m_url(url.toString(QUrl::RemoveQuery|QUrl::NormalizePathSegments))
{
}

Url::Url(const svn::Path &url)
    : m_url(url)
{
}

QByteArray Url::cstr() const
{
    return m_url.cstr();
}

/* static helpers */
bool Url::isLocal(const QString &url)
{
    static const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    static const QLatin1String stf("file://");
    static const QLatin1String stsf("svn+file://");
    static const QLatin1String stkf("ksvn+file://");
    if (
        url.startsWith(stf, cs) ||
        url.startsWith(QLatin1Char('/')) ||
        url.startsWith(stsf, cs) ||
        url.startsWith(stkf, cs)) {
        return true;
    }
    return false;
}

bool Url::isValid(const QString &url)
{
    static const std::vector<QLatin1String>
    VALID_SCHEMAS = {
        QLatin1String("http"), QLatin1String("https"), QLatin1String("file"),
        QLatin1String("svn"), QLatin1String("svn+ssh"), QLatin1String("svn+http"), QLatin1String("svn+https"), QLatin1String("svn+file"),
        QLatin1String("ksvn"), QLatin1String("ksvn+ssh"), QLatin1String("ksvn+http"), QLatin1String("ksvn+https"), QLatin1String("ksvn+file")
    };
    const QString urlTest(url);
    for (const QLatin1String &schema : VALID_SCHEMAS) {
        const QStringRef urlComp = urlTest.leftRef(schema.size());

        if (schema == urlComp) {
            return true;
        }
    }
    return false;
}

QString
Url::transformProtokoll(const QString &prot)
{
    const QString _prot = prot.toLower();
    if (_prot == QLatin1String("svn+http") ||
        _prot == QLatin1String("ksvn+http"))
        return QLatin1String("http");
    if (_prot == QLatin1String("svn+https") ||
        _prot == QLatin1String("ksvn+https"))
        return QLatin1String("https");
    if (_prot == QLatin1String("svn+file") ||
        _prot == QLatin1String("ksvn+file"))
        return QLatin1String("file");
    if (_prot == QLatin1String("ksvn+ssh"))
        return QLatin1String("svn+ssh");
    if (_prot == QLatin1String("ksvn"))
        return QLatin1String("svn");
    return _prot;
}
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
