/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include "ktranslateurl.h"

#include <kurl.h>

namespace helpers
{
namespace KTranslateUrl
{

QString makeKdeUrl(const QString &_proto)
{
    QString proto;
    if (_proto.startsWith(QLatin1String("svn+"))) {
        proto = QLatin1Char('k') + _proto;
    } else if (_proto == QLatin1String("svn")) {
        proto = QLatin1String("ksvn");
    } else {
        proto = QLatin1String("ksvn+") + _proto;
    }
    return proto;
}

QUrl string2Uri(const QString &what)
{
    KUrl uri(what);
    if (uri.scheme() == QLatin1String("file")) {
        if (what.startsWith(QLatin1String("file:"))) {
            uri.setScheme(QLatin1String("ksvn+file"));
        } else {
            uri.setScheme(QString());
        }
    } else {
        uri.setScheme(makeKdeUrl(uri.scheme()));
    }
    return uri;
}

}   // namespace KTranslateUrl

}   // namespace helpers
