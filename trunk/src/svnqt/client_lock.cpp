/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
// svncpp
#include "client_impl.h"

// subversion api
#include "svn_client.h"

#include "svnqt/exception.h"
#include "svnqt/pool.h"
#include "svnqt/targets.h"
#include "svnqt/svnqt_defines.h"

namespace svn
{

  void
  Client_impl::lock (const Targets & targets,
    const QString& message,
    bool steal_lock)  throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_lock(const_cast<apr_array_header_t*> (targets.array (pool)),
                      message.TOUTF8(),
                      steal_lock,
                      *m_context,
                      pool);
    if(error != NULL)
       throw ClientException (error);
  }

  void
  Client_impl::unlock (const Targets&targets,
            bool break_lock)  throw (ClientException)
  {
    Pool pool;
    svn_error_t * error =
      svn_client_unlock(const_cast<apr_array_header_t*> (targets.array (pool)),
                        break_lock,
                        *m_context,
                        pool);
    if(error != NULL)
       throw ClientException (error);
  }
}
