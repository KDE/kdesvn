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
#include "diff_data.hpp"

#include <qfile.h>
#include <qstringlist.h>

#include <apr_xlate.h>

namespace svn
{
  QByteArray
          Client_impl::diff_peg (const Path & tmpPath, const Path & path,
                             const Revision & revision1, const Revision & revision2, const Revision& peg_revision,
                             const bool recurse, const bool ignoreAncestry,
                             const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)
    {
        return diff_peg(tmpPath,path,
                    revision1,revision2,peg_revision,
                    recurse,ignoreAncestry,noDiffDeleted,ignore_contenttype,
                    QStringList());
    }

  QByteArray
  Client_impl::diff_peg (const Path & tmpPath, const Path & path,
                const Revision & revision1, const Revision & revision2, const Revision& peg_revision,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted,const bool ignore_contenttype,
                const QStringList&extra) throw (ClientException)
  {
    Pool pool;
    svn_error_t * error;
    const apr_array_header_t * options;

    // svn_client_diff needs an options array, even if it is empty
    options = list2array(extra,pool);
    DiffData ddata(tmpPath,path,revision1,path,revision2);

    error = svn_client_diff_peg3(
                                 options,
                                 path.cstr(),
                                 peg_revision,ddata.r1().revision(),ddata.r2().revision(),
                                 recurse?1:0,ignoreAncestry,noDiffDeleted,ignore_contenttype,
                                 APR_LOCALE_CHARSET,
                                 ddata.outFile(),ddata.errFile(),
                                 *m_context,
                                 pool
                                );
    if (error != NULL)
    {
        throw ClientException (error);
    }
    return ddata.content();
  }

  QByteArray
  Client_impl::diff (const Path & tmpPath, const Path & path1,const Path&path2,
                     const Revision & revision1, const Revision & revision2,
                     const bool recurse, const bool ignoreAncestry,
                     const bool noDiffDeleted,const bool ignore_contenttype) throw (ClientException)
    {
        return diff(tmpPath,path1,path2,
                    revision1,revision2,
                    recurse,ignoreAncestry,noDiffDeleted,ignore_contenttype,
                    QStringList());
    }

  QByteArray
  Client_impl::diff (const Path & tmpPath, const Path & path1,const Path&path2,
                const Revision & revision1, const Revision & revision2,
                const bool recurse, const bool ignoreAncestry,
                const bool noDiffDeleted,const bool ignore_contenttype,const QStringList&extra) throw (ClientException)
  {

    Pool pool;
    svn_error_t * error;
    const apr_array_header_t * options;

    // svn_client_diff needs an options array, even if it is empty
    options = list2array(extra,pool);
    DiffData ddata(tmpPath,path1,revision1,path2,revision2);

    // run diff
    error = svn_client_diff3 (options,
                             path1.cstr (), ddata.r1().revision (),
                             path2.cstr (), ddata.r2().revision (),
                             recurse?1:0, ignoreAncestry, noDiffDeleted, ignore_contenttype,
                             APR_LOCALE_CHARSET,
                             ddata.outFile(),ddata.errFile(),
                             *m_context,
                             pool);
    if (error != NULL)
    {
        throw ClientException (error);
    }
    return ddata.content();

  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
