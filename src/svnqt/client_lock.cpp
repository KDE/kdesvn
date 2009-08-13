/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
// svncpp
#include "client_impl.hpp"

// subversion api
#include "svn_client.h"

#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/svnqt_defines.hpp"

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
