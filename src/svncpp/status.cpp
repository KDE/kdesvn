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

// svncpp
#include "svncpp/status.hpp"

//#include <assert.h>

namespace svn
{
  Status::Status (const Status & src)
    : m_status (0), m_Path("")
  {
    if( &src != this )
    {
      init (src.m_Path, src.m_status);
    }
  }

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  Status::Status (const QString&path, svn_wc_status2_t * status)
    : m_status (0), m_Path("")
  {
    init (path, status);
  }

  Status::Status (const char*path, svn_wc_status2_t * status)
    : m_status (0), m_Path("")
  {
    init(QString::fromUtf8(path),status);
  }

#else
  Status::Status (const QString&path, svn_wc_status_t * status)
    : m_status (0), m_Path ("")
  {
    init (path, status);
  }

  Status::Status (const char*path, svn_wc_status_t * status)
    : m_status (0), m_Path("")
  {
    init(QString::fromUtf8(path),status);
  }
#endif

  Status::~Status ()
  {
  }

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  void Status::init (const QString&path, const svn_wc_status2_t * status)
  {
    m_Path = path;
    m_status = (svn_wc_status2_t *)
      apr_pcalloc (m_pool, sizeof (svn_wc_status2_t));

    if (!status)
    {
      m_isVersioned = false;
      m_hasReal = false;
    }
    else
    {
      m_isVersioned = status->text_status > svn_wc_status_unversioned;
      m_hasReal = m_isVersioned && status->text_status!=svn_wc_status_ignored;
      // now duplicate the contents
      if (status->entry)
      {
        m_status->entry = svn_wc_entry_dup (status->entry, m_pool);
      }
      m_status->text_status = status->text_status;
      m_status->prop_status = status->prop_status;
      m_status->locked = status->locked;
      m_status->copied = status->copied;
      m_status->switched = status->switched;
      m_status->repos_text_status = status->repos_text_status;
      m_status->repos_prop_status = status->repos_prop_status;
      if (status->repos_lock) {
        m_status->repos_lock = svn_lock_dup(status->repos_lock,m_pool);
        m_Lock.init(m_status->repos_lock->creation_date,
                    m_status->repos_lock->expiration_date,
                    m_status->repos_lock->owner,
                    m_status->repos_lock->comment,
                    m_status->repos_lock->comment);
      }
    }
  }
#endif

  void Status::init (const QString&path, const svn_wc_status_t * status)
  {
    m_Path = path;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    m_status = (svn_wc_status2_t *)
      apr_pcalloc (m_pool, sizeof (svn_wc_status2_t));
#else
    m_status = (svn_wc_status_t *)
      apr_pcalloc (m_pool, sizeof (svn_wc_status_t));
#endif
    if (!status)
    {
      m_isVersioned = false;
      m_hasReal = false;
    }
    else
    {
      m_isVersioned = status->text_status > svn_wc_status_unversioned;
      m_hasReal = m_isVersioned && status->text_status!=svn_wc_status_ignored;
      // now duplicate the contents
      if (status->entry)
      {
        m_status->entry = svn_wc_entry_dup (status->entry, m_pool);
      }
      m_status->text_status = status->text_status;
      m_status->prop_status = status->prop_status;
      m_status->locked = status->locked;
      m_status->copied = status->copied;
      m_status->switched = status->switched;
      m_status->repos_text_status = status->repos_text_status;
      m_status->repos_prop_status = status->repos_prop_status;
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
      m_status->repos_lock = svn_lock_create(m_pool);
#endif
    }
  }

  Status &
  Status::operator=(const Status & status)
  {
    if (this == &status)
      return *this;

    init (status.m_Path, status.m_status);
    return *this;
  }
}
