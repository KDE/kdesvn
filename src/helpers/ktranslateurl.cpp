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

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kdesktopfile.h>

#include <qstringlist.h>
#include <qdir.h>

namespace helpers
{
namespace KTranslateUrl
{
static bool parseURL(const KUrl &url, QString &name, QString &path);
static KUrl findSystemBase(const QString &name);

KUrl translateSystemUrl(const KUrl &_url)
{
    QString proto = _url.protocol();

    if (proto != QLatin1String("system")) {
        return _url;
    }
    KGlobal::dirs()->addResourceType("system_entries",
                                     KStandardDirs::kde_default("data") + "systemview");
    QString name, path;
    if (!parseURL(_url, name, path)) {
        return _url;
    }
    KUrl res = findSystemBase(name);
    if (!res.isValid()) {
        return _url;
    }
    res.addPath(path);
    res.setQuery(_url.query());
    return res;
}

bool parseURL(const KUrl &url, QString &name, QString &path)
{
    const QString url_path = url.path();
    int i = url_path.indexOf(QLatin1Char('/'), 1);
    if (i > 0) {
        name = url_path.mid(1, i - 1);
        path = url_path.mid(i + 1);
    } else {
        name = url_path.mid(1);
        path.clear();
    }

    return !name.isEmpty();
}

KUrl findSystemBase(const QString &filename)
{
    const QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");

    QStringList::ConstIterator dirpath = dirList.constBegin();
    QStringList::ConstIterator end = dirList.constEnd();
    for (; dirpath != end; ++dirpath) {
        QDir dir(*dirpath);
        if (!dir.exists()) {
            continue;
        }

        QStringList filenames
            = dir.entryList(QDir::Files | QDir::Readable);

        QStringList::ConstIterator name = filenames.constBegin();
        QStringList::ConstIterator endf = filenames.constEnd();

        for (; name != endf; ++name) {
            const QString fn = filename + QLatin1String(".desktop");
            if (*name == fn) {
                KDesktopFile desktop(*dirpath + fn);
                if (desktop.readUrl().isEmpty()) {
                    KUrl url;
                    url.setPath(desktop.readPath());
                    return url;
                }
                return desktop.readUrl();
            }
        }
    }

    return KUrl();
}

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

KUrl string2Uri(const QString &what)
{
    KUrl uri(what);
    if (uri.protocol() == QLatin1String("file")) {
        if (what.startsWith(QLatin1String("file:"))) {
            uri.setProtocol(QLatin1String("ksvn+file"));
        } else {
            uri.setProtocol(QString());
        }
    } else {
        uri.setProtocol(makeKdeUrl(uri.protocol()));
    }
    return uri;
}

}   // namespace KTranslateUrl

}   // namespace helpers
