/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
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
#include "svnqt/client_impl.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/helper.h"
#include "src/svnqt/client_annotate_parameter.h"

// Subversion api
#include "svn_client.h"



namespace svn
{
    static svn_error_t *
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
            annotateReceiver(void *baton, 
                             svn_revnum_t start_revnum, 
                             svn_revnum_t end_revnum, 
                             apr_int64_t line_no,
                             svn_revnum_t revision,
                             apr_hash_t *rev_props, 
                             svn_revnum_t merge_revision,
                             apr_hash_t *merged_rev_props,
                             const char *merge_path,
                             const char *line, 
                             svn_boolean_t local_change,
                             apr_pool_t *pool)
    {
        AnnotatedFile * entries = (AnnotatedFile *) baton;
        PropertiesMap _map = svn::internal::Hash2Map(rev_props,pool);
        PropertiesMap _merge_map = svn::internal::Hash2Map(merged_rev_props,pool);
        entries->push_back (AnnotateLine(line_no, 
                                         revision,_map,
                                         line,
                                         merge_revision,_merge_map,merge_path,
                                         start_revnum,end_revnum,local_change
                                        )
                           );
        return NULL;
    }
#else
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
#endif
  void
  Client_impl::annotate (AnnotatedFile&target,const AnnotateParameter&params) throw (ClientException)
  {
    Pool pool;
    svn_error_t *error;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
    error = svn_client_blame5
#else
    error = svn_client_blame4
#endif
    
    (
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
    if (error != NULL)
    {
      throw ClientException (error);
    }
  }
}
