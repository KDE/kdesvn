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
//#include "svn_io.h"

#include "exception.hpp"
#include "pool.hpp"
#include "status.hpp"
#include "svnqt_defines.hpp"
#include "svnstream.hpp"
#include "svnfilestream.hpp"

namespace svn
{
  QByteArray
  Client_impl::cat(const Path & path,
                const Revision & revision,
                const Revision & peg_revision) throw (ClientException)
  {
    svn::stream::SvnByteStream buffer(*m_context);
    svn_error_t * error = internal_cat(path,revision,peg_revision,buffer);
    if (error != 0)
      throw ClientException (error);

    return buffer.content();
  }

  void
  Client_impl::get (const Path & path,
        const QString  & target,
        const Revision & revision,
        const Revision & peg_revision) throw (ClientException)
  {
    svn::stream::SvnFileOStream buffer(target,*m_context);
    svn_error_t * error = internal_cat(path,revision,peg_revision,buffer);
    if (error != 0)
      throw ClientException (error);
  }

  svn_error_t * Client_impl::internal_cat(const Path & path,
            const Revision & revision,
            const Revision & peg_revision,
            svn::stream::SvnStream&buffer)
  {
    Pool pool;
    return svn_client_cat2 (buffer,
                             path.path().TOUTF8(),
                             peg_revision.revision (),
                             revision.revision (),
                             *m_context,
                             pool);
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
