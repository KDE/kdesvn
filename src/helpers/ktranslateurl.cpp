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

namespace helpers {

KTranslateUrl::KTranslateUrl()
{
}


KTranslateUrl::~KTranslateUrl()
{
}

KUrl KTranslateUrl::translateSystemUrl(const KUrl&_url)
{
    QString proto = _url.protocol();
    KUrl res;
    QString name,path;

    if (proto!="system") {
        return _url;
    }
    KGlobal::dirs()->addResourceType("system_entries",
    KStandardDirs::kde_default("data") + "systemview");
    QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");
    if (!parseURL(_url,name,path)) {
        return _url;
    }
    res = findSystemBase(name);
    if (!res.isValid()) {
        return _url;
    }
    res.addPath(path);
    res.setQuery(_url.query());
    return res;
}

bool KTranslateUrl::parseURL(const KUrl&url,QString&name,QString&path)
{
    QString url_path = url.path();
    int i = url_path.indexOf('/',1);
    if (i > 0)
    {
        name = url_path.mid(1, i-1);
        path = url_path.mid(i+1);
    }
    else
    {
        name = url_path.mid(1);
        path.clear();
    }

    return !name.isEmpty();
}

KUrl KTranslateUrl::findSystemBase(const QString&filename)
{
    QStringList dirList = KGlobal::dirs()->resourceDirs("system_entries");

    QStringList::ConstIterator dirpath = dirList.begin();
    QStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        QDir dir = *dirpath;
        if (!dir.exists()) continue;

        QStringList filenames
                = dir.entryList( QDir::Files | QDir::Readable );


        KIO::UDSEntry entry;

        QStringList::ConstIterator name = filenames.begin();
        QStringList::ConstIterator endf = filenames.end();

        for(; name!=endf; ++name)
        {
            if (*name==filename+".desktop")
            {
#if QT_VERSION < 0x040000
                KDesktopFile desktop(*dirpath+filename+".desktop", true);
#else
                KDesktopFile desktop(*dirpath+filename+".desktop");
#endif
#if QT_VERSION < 0x040000
                if ( desktop.readURL().isEmpty() )
#else
                if ( desktop.readUrl().isEmpty() )
#endif
                {
                    KUrl url;
                    url.setPath( desktop.readPath() );
                    return url;
                }
#if QT_VERSION < 0x040000
                return desktop.readURL();
#else
                return desktop.readUrl();
#endif
            }
        }
    }

    return KUrl();
}

}


/*!
    \fn helpers::KTranslateUrl::makeKdeUrl(const QString&inUrl)
 */
QString helpers::KTranslateUrl::makeKdeUrl(const QString&_proto)
{
    QString proto;
    if (_proto.startsWith("svn+")){
        proto = 'k'+_proto;
    } else if (_proto== QString("svn")){
        proto = "ksvn";
    } else {
        proto = "ksvn+"+_proto;
    }
    return proto;
}
