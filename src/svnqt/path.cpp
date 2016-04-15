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

#include "path.h"
#include "helper.h"

// subversion api
#include <svn_path.h>
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,6,0)
#include <svn_dirent_uri.h>
#endif

// apr api
#include <apr_file_io.h>

// svncpp
#include "pool.h"
#include "url.h"
#include "svnqt_defines.h"
#include "revision.h"
#include "exception.h"

#include <QUrl>
#include <QDir>

namespace svn
{
Path::Path(const QString &path)
{
    init(path);
}

Path::Path(const QUrl &path)
{
    init(path.toString());
}

Path::Path(const Path &path)
    : m_path(path.m_path)
{
}

void
Path::init(const QString &path)
{
    Pool pool;

    if (path.isEmpty()) {
        m_path.clear();
    } else {
        QByteArray int_path = path.toUtf8();

        if (Url::isValid(path)) {
            if (!svn_path_is_uri_safe(int_path)) {
                int_path = svn_path_uri_encode(int_path, pool);
            }
        } else {
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,6,0)
            int_path = svn_dirent_internal_style(int_path, pool.pool());
#else
            int_path = svn_path_internal_style(int_path, pool.pool());
#endif
        }

        m_path = QString::fromUtf8(int_path);
        /* the following block is a problem and thats why commented out: since a while subversion raises
         * an assert because of wrong url if replacing the @ sign with entity and kdesvn dies.
         * So using the scheme on ubuntu that it just don't display the content of such a folder/file.
         */
        /*
         if (Url::isValid(m_path) && m_path.indexOf("@")!=-1 ) {
          /// @todo make sure that "@" is never used as revision parameter
          QUrl uri = m_path;
          m_path = uri.path();
          m_path.replace('@',"%40");
          m_path = uri.scheme()+"://"+uri.authority()+m_path;
        }
        */

        while (m_path.endsWith(QLatin1Char('/'))) {
            m_path.chop(1);
        }
    }
}

bool Path::isUrl()const
{
    return Url::isValid(m_path);
}

const QString &
Path::path() const
{
    return m_path;
}

const QByteArray
Path::cstr() const
{
    return m_path.toUtf8();
}

Path &
Path::operator=(const Path &path)
{
    if (this == &path) {
        return *this;
    }
    m_path = path.path();
    return *this;
}

bool
Path::isset() const
{
    return !m_path.isEmpty();
}

void
Path::addComponent(const QString &_component)
{
    Pool pool;
    QString component = _component;
    while (component.endsWith(QLatin1Char('/'))) {
        component.chop(1);
    }
    if (Url::isValid(m_path)) {
        const char *newPath =
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,6,0)
            svn_path_url_add_component2(m_path.toUtf8(), component.toUtf8(), pool);
#else
            svn_path_url_add_component(m_path.toUtf8(), component.toUtf8(), pool);
#endif
        m_path = QString::fromUtf8(newPath);
    } else {
        svn_stringbuf_t *pathStringbuf =
            svn_stringbuf_create(m_path.toUtf8(), pool);

        svn_path_add_component(pathStringbuf,
                               component.toUtf8());
        m_path = QString::fromUtf8(pathStringbuf->data);
    }
}

void
Path::removeLast()
{
    Pool pool;
    if (m_path.length() <= 1) {
        m_path.clear();
    }
    svn_stringbuf_t *pathStringbuf =
        svn_stringbuf_create(m_path.toUtf8(), pool);
    svn_path_remove_component(pathStringbuf);
    m_path = QString::fromUtf8(pathStringbuf->data);
}

void
Path::parsePeg(const QString &pathorurl, Path &_path, svn::Revision &_peg)
{
    const QByteArray _buf = pathorurl.toUtf8();
    const char *truepath = 0;
    svn_opt_revision_t pegr;

    Pool pool;
    svn_error_t *error = svn_opt_parse_path(&pegr, &truepath, _buf, pool);
    if (error != 0) {
        throw ClientException(error);
    }
    //qDebug("Path: %s",truepath);
    _peg = svn::Revision(&pegr);
    _path = Path(QString::fromUtf8(truepath));
}

unsigned int
Path::length() const
{
    return m_path.length();
}

QString
Path::native() const
{
    if (isUrl()) {
        return m_path;
    }
    Pool pool;
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,6,0)
    return QString::fromUtf8(svn_dirent_local_style(m_path.toUtf8(), pool));
#else
    return QString::fromUtf8(svn_path_local_style(m_path.toUtf8(), pool));
#endif
}

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
