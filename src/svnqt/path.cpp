/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
 * http://kdesvn.alwins-world.de
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
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


// subversion api
#include "svn_path.h"

// apr api
#include "apr_file_io.h"

// svncpp
#include "svnqt/path.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/url.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/exception.hpp"

#include <qurl.h>

namespace svn
{
  Path::Path (const char * path)
  {
    init(QString::FROMUTF8(path));
  }

  Path::Path (const QString & path)
  {
    init (path);
  }

  Path::Path (const Path & path)
    : m_path(path.m_path)
  {
  }

  void
  Path::init (const QString& path)
  {
    Pool pool;

    if (path.isEmpty()) {
      m_path = "";
    } else {
      const char * int_path = svn_path_internal_style (path.TOUTF8(), pool.pool () );
      if (Url::isValid(path) ) {
        if (!svn_path_is_uri_safe(int_path)) {
            int_path = svn_path_uri_encode(int_path,pool);
        }
      }
      m_path = QString::FROMUTF8(int_path);
      if (Url::isValid(path) && m_path.indexOf("@")!=-1 ) {
        /// @todo make sure that "@" is never used as revision paramter
        QUrl uri = m_path;
        m_path = uri.path();
        m_path.replace("@","%40");
        m_path = uri.scheme()+"://"+uri.authority()+m_path;
        if (m_path.endsWith("/")) {
            int_path = svn_path_internal_style (path.TOUTF8(), pool.pool () );
            m_path = QString::FROMUTF8(int_path);
        }
      }
    }
  }

  bool Path::isUrl()const
  {
      return Url::isValid(m_path);
  }

  const QString &
  Path::path () const
  {
    return m_path;
  }

  Path::operator const QString&()const
  {
    return m_path;
  }

  QString Path::prettyPath()const
  {
    if (!Url::isValid(m_path)) {
        return m_path;
    }
    Pool pool;
    const char * int_path = svn_path_uri_decode(m_path.TOUTF8(), pool.pool () );
    QString _p = QString::FROMUTF8(int_path);
    _p.replace("%40","@");
    return _p;
  }

  const QByteArray
  Path::cstr() const
  {
    return m_path.TOUTF8();
  }

  Path&
  Path::operator=(const Path & path)
  {
    if (this == &path)
      return *this;
    m_path = path.path();
    return *this;
  }

  bool
  Path::isset () const
  {
    return m_path.length () > 0;
  }

  void
  Path::addComponent (const QString& component)
  {
      Pool pool;

    if (Url::isValid (m_path))
    {
      const char * newPath =
          svn_path_url_add_component (m_path.TOUTF8(), component.TOUTF8(), pool);
      m_path = QString::FROMUTF8(newPath);
    }
    else
    {
      svn_stringbuf_t * pathStringbuf =
          svn_stringbuf_create (m_path.TOUTF8(), pool);

      svn_path_add_component (pathStringbuf,
                              component.TOUTF8());

      m_path = QString::FROMUTF8(pathStringbuf->data);
    }
  }


  void
  Path::addComponent (const char* component)
  {
    addComponent (QString::FROMUTF8(component));
  }


  void
  Path::removeLast()
  {
    Pool pool;
    if (m_path.length()<=1) {
        m_path=QString::FROMUTF8("");
    }
    svn_stringbuf_t*pathStringbuf=
            svn_stringbuf_create (m_path.TOUTF8(), pool);
    svn_path_remove_component(pathStringbuf);
    m_path = QString::FROMUTF8(pathStringbuf->data);
  }

  void
  Path::split (QString & dirpath, QString & basename) const
  {
    Pool pool;

    const char * cdirpath;
    const char * cbasename;

    svn_path_split (prettyPath().TOUTF8(), &cdirpath, &cbasename, pool);
    dirpath = QString::FROMUTF8(cdirpath);
    basename = QString::FROMUTF8(cbasename);
  }


  void
  Path::split (QString & dir, QString & filename, QString & ext) const
  {
    QString basename;

    // first split path into dir and filename+ext
    split (dir, basename);

    // next search for last .
    int pos = basename.lastIndexOf(QChar('.'));

    if (pos == -1)
    {
      filename = basename;
      ext = QString::fromLatin1("");
    }
    else
    {
      filename = basename.left(pos);
      ext = basename.mid(pos+1);
    }
  }

  Path
  Path::getTempDir ()
  {
    const char * tempdir = 0;
    Pool pool;

    if (apr_temp_dir_get (&tempdir, pool) != APR_SUCCESS)
    {
      tempdir = 0;
    }

    return tempdir;
  }

    void
    Path::parsePeg(const QString&pathorurl,Path&_path,svn::Revision&_peg)
    {
        const char *truepath = 0;
        svn_opt_revision_t pegr;
        svn_error_t *error = 0;
        QByteArray _buf = pathorurl.TOUTF8();

        Pool pool;
        error = svn_opt_parse_path(&pegr, &truepath,_buf,pool);
        if (error != 0) {
            throw ClientException (error);
        }
        qDebug("Path: %s",truepath);
        _peg = svn::Revision(&pegr);
        _path=Path(truepath);
    }

  unsigned int
  Path::length () const
  {
    return m_path.length ();
  }


  QString
  Path::native () const
  {
    Pool pool;

    return QString::FROMUTF8(svn_path_local_style (m_path.TOUTF8(), pool));
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
