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
#include "client.hpp"
#include "client_impl.hpp"
#include "svn_opt.h"
#include "svnqt_defines.hpp"

#include "svninit.hpp"

#include <svn_cmdline.h>

namespace svn
{

namespace internal {
    SvnInit::SvnInit()
    {
        svn_cmdline_init("svnqt",0);
    }

    void SvnInit::initsvn()
    {
        //with that trick wie make sure that initialize is called only once!
        static SvnInit sInit;
    }
}

  Client::Client()
  {
  }

  Client::~Client ()
  {
  }

  Client*Client::getobject(Context * context,int subtype)
  {
    svn::internal::SvnInit::initsvn();
    switch(subtype) {
      case 0:
       return new Client_impl(context);
       break;
      default:
       break;
    }
    return 0L;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
