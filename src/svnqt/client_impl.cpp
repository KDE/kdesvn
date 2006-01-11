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
#include "svn_opt.h"
#include "svncpp_defines.hpp"

namespace svn
{
  Client_impl::Client_impl (Context * context)
	: Client()
  {
    setContext (context);
  }

  Client_impl::~Client_impl ()
  {
  }

  const Context *
  Client_impl::getContext () const
  {
    return m_context;
  }

  void
  Client_impl::setContext (Context * context)
  {
    m_context = context;
  }


  void
  Client_impl::url2Revision(const QString&revstring,
        Revision&start,Revision&end)
  {
    Pool pool;
    int n = svn_opt_parse_revision(start,end,revstring.TOUTF8(),pool);

    if (n<0) {
        start = Revision::UNDEFINED;
        end = Revision::UNDEFINED;
    }
  }

    void Client_impl::url2Revision(const QString&revstring,Revision&start) {
        if (revstring=="WORKING") {
            start = Revision::WORKING;
        } else if (revstring=="BASE"){
            start = Revision::BASE;
        } else {
            Revision end;
            url2Revision(revstring,start,end);
        }
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
