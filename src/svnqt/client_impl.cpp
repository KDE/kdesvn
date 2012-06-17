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
#include "svnqt/exception.h"
#include "svnqt/helper.h"

#include "svn_opt.h"
#include "svn_ra.h"

#include <qmap.h>
#include <qstringlist.h>
namespace svn
{

  Client_impl::Client_impl (const ContextP&context)
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
  Client_impl::setContext (const ContextP&context)
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
      return svn::internal::Map2Hash(aMap).hash(pool);
  }

  bool Client_impl::RepoHasCapability(const Path&repository,Capability capability)
  {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        svn_error_t* error = 0;
        Pool pool;

        svn_ra_session_t*session=0;
        error = svn_client_open_ra_session(&session,
                                            repository.cstr(),
                                            *m_context,
                                            pool);
        if (error != 0) {
            throw ClientException(error);
        }
        if (!session) {
            return false;
        }
        const char*capa=0;
        switch (capability) {
            case CapabilityMergeinfo:
                capa = SVN_RA_CAPABILITY_MERGEINFO;
                break;
            case CapabilityDepth:
                capa = SVN_RA_CAPABILITY_DEPTH;
                break;
            case CapabilityCommitRevProps:
                capa = SVN_RA_CAPABILITY_COMMIT_REVPROPS;
                break;
            case CapabilityLogRevProps:
                capa = SVN_RA_CAPABILITY_LOG_REVPROPS;
                break;
            default:
                return false;
        }
        svn_boolean_t has=0;
        error = svn_ra_has_capability(session,&has,capa,pool);
        if (error != 0) {
            throw ClientException(error);
        }
        return has;
#else
      Q_UNUSED(repository);
      Q_UNUSED(capability);
      return false;
#endif

  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
