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
#include "client_impl.hpp"

// Subversion api
#include "svn_client.h"

#include "svncpp_defines.hpp"


namespace svn
{
  static svn_error_t *
  annotateReceiver (void *baton,
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

  void
  Client_impl::annotate (AnnotatedFile&target,const Path & path,
            const Revision & revisionStart,
            const Revision & revisionEnd,
            const Revision & peg) throw (ClientException)
  {
    Pool pool;
    svn_error_t *error;
    error = svn_client_blame2(
      path.path().TOUTF8(),
      peg.revision(),
      revisionStart.revision (),
      revisionEnd.revision (),
      annotateReceiver,
      &target,
      *m_context, // client ctx
      pool);

    if (error != NULL)
    {
      throw ClientException (error);
    }
  }
}
