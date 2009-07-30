/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht (ral@alwins-world.de)
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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif
// svncpp
#include "svnqt/client_impl.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "src/svnqt/client_annotate_parameter.hpp"

// Subversion api
#include "svn_client.h"



namespace svn
{
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    static svn_error_t *
            annotateReceiver(void *baton,
                              apr_int64_t line_no,
                              svn_revnum_t revision,
                              const char *author,
                              const char *date,
                              svn_revnum_t merge_revision,
                              const char *merge_author,
                              const char *merge_date,
                              const char *merge_path,
                              const char *line,
                              apr_pool_t *)
    {
        AnnotatedFile * entries = (AnnotatedFile *) baton;
        entries->push_back (AnnotateLine(line_no, revision,author,
                            date,line,merge_revision,
                            merge_author,merge_date,merge_path));
        return NULL;
    }
#else
  static svn_error_t *
  annotateReceiver(void *baton,
                    apr_int64_t line_no,
                    svn_revnum_t revision,
                    const char *author,
                    const char *date,
                    const char *line,
                    apr_pool_t *)
  {
    AnnotatedFile * entries = (AnnotatedFile *) baton;
    entries->push_back (
      AnnotateLine (line_no, revision,
                    author?author:"",
                    date?date:"",
                    line?line:""));

    return NULL;
  }
#endif

  void
  Client_impl::annotate (AnnotatedFile&target,const AnnotateParameter&params) throw (ClientException)
  {
    Pool pool;
    svn_error_t *error;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_blame4(
                params.path().cstr(),
                params.pegRevision().revision(),
                params.revisionRange().first,
                params.revisionRange().second,
                params.diffOptions().options(pool),
                params.ignoreMimeTypes(),
                params.includeMerged(),
                annotateReceiver,
                &target,
                *m_context, // client ctx
                pool);
#elif ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
    error = svn_client_blame3(
                params.path().cstr(),
                params.pegRevision().revision(),
                params.revisionRange().first,
                params.revisionRange().second,
                params.diffOptions().options(pool),
                params.ignoreMimeTypes(),
                annotateReceiver,
                &target,
                *m_context, // client ctx
                pool);
#endif
    if (error != NULL)
    {
      throw ClientException (error);
    }
  }
}
