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

// Apache Portable Runtime
#include "apr_xlate.h"

// Subversion api
#include "svn_auth.h"
#include "svn_config.h"
#include "svn_subst.h"
//#include "svn_utf.h"

// svncpp
#include "apr.h"
#include "context.h"
#include "context_listener.h"
#include "contextdata.h"

namespace svn
{
  Context::Context (const QString &configDir)
    : ref_count()
  {
    m = new ContextData (configDir);
  }

  Context::Context (const Context & src)
    : ref_count()
  {
    m = new ContextData (src.m->configDir());
    setLogin (src.getUsername (), src.getPassword ());
  }

  Context::~Context ()
  {
    delete m;
  }

  void
  Context::setAuthCache (bool value)
  {
    m->setAuthCache (value);
  }

  void
  Context::setLogin (const QString& username, const QString& password)
  {
    m->setLogin (username, password);
  }

  Context::operator svn_client_ctx_t * ()const
  {
    return m->ctx();
  }

  svn_client_ctx_t *
  Context::ctx ()const
  {
    return m->ctx();
  }

  void
  Context::setLogMessage (const QString& msg)
  {
    m->setLogMessage (msg);
  }

  const QString&
  Context::getUsername () const
  {
    return m->getUsername ();
  }

  const QString&
  Context::getPassword () const
  {
    return m->getPassword ();
  }

  const QString&
  Context::getLogMessage () const
  {
    return m->getLogMessage ();
  }

  void
  Context::setListener (ContextListener * listener)
  {
    m->setListener(listener);
  }

  ContextListener *
  Context::getListener () const
  {
    return m->getListener();
  }

  void
  Context::reset ()
  {
    m->reset();
  }

  bool Context::contextAddListItem(DirEntries*entries, const svn_dirent_t*dirent,const svn_lock_t*lock,const QString&path)
  {
    return m->contextAddListItem(entries,dirent,lock,path);
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
