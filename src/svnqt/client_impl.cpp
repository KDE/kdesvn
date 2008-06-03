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
#include "svn_opt.h"
#include "svnqt/svnqt_defines.hpp"

#include <qmap.h>
#include <qstringlist.h>
namespace svn
{

  Client_impl::Client_impl (ContextP context)
	: Client()
  {
    setContext (context);
  }

  Client_impl::~Client_impl ()
  {
  }

  const ContextP
  Client_impl::getContext () const
  {
    return m_context;
  }

  void
  Client_impl::setContext (ContextP context)
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

    void Client_impl::url2Revision(const QString&revstring,Revision&start)
    {
        if (revstring=="WORKING") {
            start = Revision::WORKING;
        } else if (revstring=="BASE"){
            start = Revision::BASE;
        } else if (revstring=="START"){
            start = Revision::START;
        } else {
            Revision end;
            url2Revision(revstring,start,end);
        }
  }

  apr_hash_t * Client_impl::map2hash(const PropertiesMap&aMap,const Pool&pool)
  {
      if (aMap.count()==0) {
          return 0;
      }
      apr_hash_t * hash = apr_hash_make(pool);
      PropertiesMap::ConstIterator it;
      const char*propval;
      const char*propname;
      QByteArray s,n;
      for (it=aMap.begin();it!=aMap.end();++it) {
          s=it.data().TOUTF8();
          n=it.key().TOUTF8();
          propval=apr_pstrndup(pool,s,s.size());
          propname=apr_pstrndup(pool,n,n.size());
          apr_hash_set(hash,propname,APR_HASH_KEY_STRING,propval);
      }
      return hash;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
