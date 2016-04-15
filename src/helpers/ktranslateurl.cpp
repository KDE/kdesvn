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

#include <QFileInfo>
#include <QUrl>
#include "helpers/kdesvn_debug.h"

namespace helpers
{
namespace KTranslateUrl
{

QString makeKdeUrl(const QString &proto)
{
    if (proto.startsWith(QLatin1String("svn+"))) {
        return QLatin1Char('k') + proto;
    }
    if (proto == QLatin1String("svn")) {
        return QLatin1String("ksvn");
    }
    return QLatin1String("ksvn+") + proto;
}

QUrl string2Uri(const QString &what)
{
    // if the file is available in the local fs -> QUrl::fromLocalFile
    QFileInfo fi(what);
    if (fi.isAbsolute() || fi.exists()) {
        return QUrl::fromLocalFile(fi.absoluteFilePath());
    }
    // not a local file, assume a url (which can also be a local file)
    QUrl uri(what);
    if (!uri.isLocalFile()) {
        uri.setScheme(makeKdeUrl(uri.scheme()));
    }
    qCDebug(KDESVN_LOG) << "string2Uri(" << what << ") -> " << uri.toString() << ", local: " << uri.isLocalFile();
    return uri;
}

}   // namespace KTranslateUrl

}   // namespace helpers
