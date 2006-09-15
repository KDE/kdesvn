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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif
// svncpp
#include "svnqt/client_impl.hpp"

// Subversion api
#include "svn_client.h"
#include "svn_path.h"

#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/status.hpp"
#include "svnqt/svnqt_defines.hpp"

#include <qfile.h>

#include <apr_xlate.h>

namespace svn
{
  /**
   * a quick way to create error messages
   */
  static void
  fail (apr_pool_t *pool, apr_status_t status, const char *fmt, ...)
  {
    va_list ap;
    char *msg;
    svn_error_t * error;

    va_start (ap, fmt);
    msg = apr_pvsprintf (pool, fmt, ap);
    va_end (ap);

    error = svn_error_create (status, NULL, msg);
    throw ClientException (error);
  }

  /**
   * closes and deletes temporary files that diff has been using
   */
  static void
  diffCleanup (apr_file_t * outfile, const char * outfileName,
               apr_file_t * errfile, const char * errfileName,
               apr_pool_t *pool)
  {
    if (outfile != NULL)
      apr_file_close (outfile);

    if (errfile != NULL)
      apr_file_close (errfile);

    if (outfileName != NULL)
      svn_error_clear (svn_io_remove_file (outfileName, pool));

    if (errfileName != NULL)
      svn_error_clear (svn_io_remove_file (errfileName, pool));
  }

  QByteArray
  Client_impl::diff (const Path & tmpPath, const Path & path,
                const Revision & revision1, const Revision & revision2,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)
  {
    return diff(tmpPath,path,path,revision1,revision2,recurse,ignoreAncestry,noDiffDeleted,ignore_contenttype);
  }

  QByteArray
  Client_impl::diff (const Path & tmpPath, const Path & path1,const Path&path2,
                const Revision & revision1, const Revision & revision2,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)
  {

    Pool pool;
    svn_error_t * error;
    apr_status_t status;
    apr_file_t * outfile = 0L;
    const char * outfileName = 0L;
    apr_file_t * errfile = 0L;
    const char * errfileName = 0L;
    apr_array_header_t * options;
    bool working_copy_present = false;
    bool url_is_present = false;
    Revision r1,r2;
    r1 = revision1;
    r2 = revision2;

    if (svn_path_is_url(path1.cstr())) {
        url_is_present = true;
    } else {
        working_copy_present = true;
    }
    if (svn_path_is_url(path2.cstr())) {
        url_is_present = true;
    } else {
        working_copy_present = true;
    }

    if (revision1.revision()->kind==svn_opt_revision_unspecified && working_copy_present) {
        r1 = svn_opt_revision_base;
    }
    if (revision2.revision()->kind==svn_opt_revision_unspecified) {
        r2 = working_copy_present?svn_opt_revision_working : svn_opt_revision_head;
    }
    // svn_client_diff needs an options array, even if it is empty
    options = apr_array_make (pool, 0, 0);

    // svn_client_diff needs a temporary file to write diff output to
    error = svn_io_open_unique_file (&outfile, &outfileName,
                                     tmpPath.path().TOUTF8(),
                                     ".tmp",
                                     FALSE, pool);

    if (error != NULL)
    {
      diffCleanup (outfile, outfileName, errfile, errfileName, pool);
      throw ClientException (error);
    }

    // and another one to write errors to
    error = svn_io_open_unique_file (&errfile, &errfileName,
                                     tmpPath.path().TOUTF8(), ".error.tmp",
                                     FALSE, pool);

    if (error != NULL)
    {
      diffCleanup (outfile, outfileName, errfile, errfileName, pool);
      throw ClientException (error);
    }

    // run diff
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
    error = svn_client_diff3 (options,
                             path1.cstr (), r1.revision (),
                             path2.cstr (), r2.revision (),
                             recurse?1:0, ignoreAncestry, noDiffDeleted, ignore_contenttype,
                             APR_LOCALE_CHARSET,
                             outfile, errfile,
                             *m_context,
                             pool);
#else
    error = svn_client_diff2 (options,
                             path1.cstr (), r1.revision (),
                             path2.cstr (), r2.revision (),
                             recurse?1:0, ignoreAncestry, noDiffDeleted, ignore_contenttype,
                             outfile, errfile,
                             *m_context,
                             pool);
#endif

    if (error != NULL)
    {
      diffCleanup (outfile, outfileName, errfile, errfileName, pool);
      throw ClientException (error);
    }

    status = apr_file_close (outfile);
    if (status)
    {
      diffCleanup (outfile, outfileName, errfile, errfileName, pool);
      fail (pool, status, "failed to close '%s'", outfileName);
    }

    QFile fi(outfileName);
#if QT_VERSION < 0x040000
    if (!fi.open(IO_ReadOnly|IO_Raw)) {
#else
    if (!fi.open(QIODevice::ReadOnly)) {
#endif
        diffCleanup (outfile, outfileName, errfile, errfileName, pool);
        fail(pool,0,fi.errorString().TOUTF8() + "'%s'",outfileName);
    }

    QByteArray res = fi.readAll();
    fi.close();

    diffCleanup (outfile, outfileName, errfile, errfileName, pool);
    return res;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
