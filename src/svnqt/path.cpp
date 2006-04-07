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
#include "path.hpp"
#include "pool.hpp"
#include "url.hpp"
#include "svncpp_defines.hpp"


namespace svn
{
  Path::Path (const char * path)
  {
    init(QString::fromUtf8(path));
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
      m_path = QString::fromUtf8(int_path);
    }
  }

  const QString &
  Path::path () const
  {
    return m_path;
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
      m_path = QString::fromUtf8(newPath);
    }
    else
    {
      svn_stringbuf_t * pathStringbuf =
          svn_stringbuf_create (m_path.TOUTF8(), pool);

      svn_path_add_component (pathStringbuf,
                              component.TOUTF8());

      m_path = QString::fromUtf8(pathStringbuf->data);
    }
  }


  void
  Path::addComponent (const char* component)
  {
    addComponent (QString::fromUtf8(component));
  }


  void
  Path::split (QString & dirpath, QString & basename) const
  {
    Pool pool;

    const char * cdirpath;
    const char * cbasename;

    svn_path_split (m_path.TOUTF8(), &cdirpath, &cbasename, pool);
    dirpath = QString::fromUtf8(cdirpath);
    basename = QString::fromUtf8(cbasename);
  }


  void
  Path::split (QString & dir, QString & filename, QString & ext) const
  {
    QString basename;

    // first split path into dir and filename+ext
    split (dir, basename);

    // next search for last .
#if QT_VERSION < 0x040000
    int pos = basename.findRev(QChar('.'));
#else
    int pos = basename.lastIndexOf(QChar('.'));
#endif

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

  /* ===================================================================
   * The next two Fixed_* functions are copies of the APR
   * apr_temp_dir_get functionality with a fix applied.
   * This should turn up in APR release 0.9.5 or 1.0, but
   * for now is reproduced here.
   *
   * TODO: Remove this section!
   */
#include "apr_env.h"

#define test_tempdir    Fixed_test_tempdir
#define apr_temp_dir_get    Fixed_apr_temp_dir_get

  static char global_temp_dir[APR_PATH_MAX+1] = { 0 };

  /* Try to open a temporary file in the temporary dir, write to it,
    and then close it. */
  static int Fixed_test_tempdir(const char *temp_dir, apr_pool_t *p)
  {
      apr_file_t *dummy_file;
      // This is the only actual fix - adding the ".XXXXXX"!
      const char *path = apr_pstrcat(p, temp_dir, "/apr-tmp.XXXXXX", NULL);

      if (apr_file_mktemp(&dummy_file, (char *)path, 0, p) == APR_SUCCESS) {
          if (apr_file_putc('!', dummy_file) == APR_SUCCESS) {
              if (apr_file_close(dummy_file) == APR_SUCCESS) {
                  apr_file_remove(path, p);
                  return 1;
              }
          }
      }
      return 0;
  }

  static apr_status_t Fixed_apr_temp_dir_get(const char **temp_dir, apr_pool_t *p)
  {
    apr_status_t apr_err;
    const char *try_dirs[] = { "/tmp", "/usr/tmp", "/var/tmp" };
    const char *try_envs[] = { "TMP", "TEMP", "TMPDIR" };
    char *cwd;
    size_t i;

    /* Our goal is to find a temporary directory suitable for writing
       into.  We'll only pay the price once if we're successful -- we
       cache our successful find.  Here's the order in which we'll try
       various paths:

          $TMP
          $TEMP
          $TMPDIR
          "/tmp"
          "/var/tmp"
          "/usr/tmp"
          `pwd`

       NOTE: This algorithm is basically the same one used by Python
       2.2's tempfile.py module.  */

    /* Try the environment first. */
    for (i = 0; i < (sizeof(try_envs) / sizeof(const char *)); i++) {
        char *value;
        apr_err = apr_env_get(&value, try_envs[i], p);
        if ((apr_err == APR_SUCCESS) && value) {
            apr_size_t len = strlen(value);
            if (len && (len < APR_PATH_MAX) && test_tempdir(value, p)) {
                memcpy(global_temp_dir, value, len + 1);
                goto end;
            }
        }
    }

    /* Next, try a set of hard-coded paths. */
    for (i = 0; i < (sizeof(try_dirs) / sizeof(const char *)); i++) {
        if (test_tempdir(try_dirs[i], p)) {
            memcpy(global_temp_dir, try_dirs[i], strlen(try_dirs[i]) + 1);
            goto end;
        }
    }

    /* Finally, try the current working directory. */
    if (APR_SUCCESS == apr_filepath_get(&cwd, APR_FILEPATH_NATIVE, p)) {
        if (test_tempdir(cwd, p)) {
            memcpy(global_temp_dir, cwd, strlen(cwd) + 1);
            goto end;
        }
    }

end:
    if (global_temp_dir[0]) {
        *temp_dir = apr_pstrdup(p, global_temp_dir);
        return APR_SUCCESS;
    }
    return APR_EGENERAL;
  }

  /* ===================================================================
   * End of inserted fixed APR code
   */

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


  unsigned int
  Path::length () const
  {
    return m_path.length ();
  }


  QString
  Path::native () const
  {
    Pool pool;

    return QString::fromUtf8(svn_path_local_style (m_path.TOUTF8(), pool));
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
