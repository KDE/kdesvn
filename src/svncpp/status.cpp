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
    : m_status (0), m_path (0)
  {
    if( &src != this )
    {
      init (src.m_path->data, src.m_status);
    }
  }

  Status::Status (const char *path, svn_wc_status_t * status)
    : m_status (0), m_path (0)
  {
    init (path, status);
  }

  Status::~Status ()
  {
  }

#if 0
  static svn_wc_entry_t *
    wc_local_entry_dup (const svn_wc_entry_t *entry, apr_pool_t *pool)
    {
      svn_wc_entry_t *dupentry = (svn_wc_entry_t*)apr_pcalloc (pool, sizeof(svn_wc_entry_t));

        /* Perform a trivial copy ... */
      *dupentry = *entry;

  /* ...and then re-copy stuff that needs to be duped into our pool. */
  if (entry->name)
    dupentry->name = apr_pstrdup (pool, entry->name);
  if (entry->url)
    dupentry->url = apr_pstrdup (pool, entry->url);
  if (entry->repos)
    dupentry->repos = apr_pstrdup (pool, entry->repos);
  if (entry->uuid)
    dupentry->uuid = apr_pstrdup (pool, entry->uuid);
  if (entry->copyfrom_url)
    dupentry->copyfrom_url = apr_pstrdup (pool, entry->copyfrom_url);
  if (entry->conflict_old)
    dupentry->conflict_old = apr_pstrdup (pool, entry->conflict_old);
  if (entry->conflict_new)
    dupentry->conflict_new = apr_pstrdup (pool, entry->conflict_new);
  if (entry->conflict_wrk)
    dupentry->conflict_wrk = apr_pstrdup (pool, entry->conflict_wrk);
  if (entry->prejfile)
    dupentry->prejfile = apr_pstrdup (pool, entry->prejfile);
  if (entry->checksum)
    dupentry->checksum = apr_pstrdup (pool, entry->checksum);
  if (entry->cmt_author)
    dupentry->cmt_author = apr_pstrdup (pool, entry->cmt_author);
  if (entry->lock_token)
    dupentry->lock_token = apr_pstrdup (pool, entry->lock_token);
  if (entry->lock_owner)
    dupentry->lock_owner = apr_pstrdup (pool, entry->lock_owner);
  if (entry->lock_comment)
    dupentry->lock_comment = apr_pstrdup (pool, entry->lock_comment);

  return dupentry;

    }
#endif

  void Status::init (const char *path, const svn_wc_status_t * status)
  {
    if (path)
    {
      m_path = svn_string_create (path, m_pool);
    }
    else
    {
      m_path = svn_string_create ("", m_pool);
    }

    m_status = (svn_wc_status_t *)
      apr_pcalloc (m_pool, sizeof (svn_wc_status_t));
    if (!status)
    {
      m_isVersioned = false;
    }
    else
    {
      m_isVersioned = status->text_status > svn_wc_status_unversioned;
      // now duplicate the contents
      if (status->entry)
      {
        //m_status->entry = wc_local_entry_dup(status->entry, m_pool);
        m_status->entry = svn_wc_entry_dup (status->entry, m_pool);
      }
      m_status->text_status = status->text_status;
      m_status->prop_status = status->prop_status;
      m_status->locked = status->locked;
      m_status->copied = status->copied;
      m_status->switched = status->switched;
      m_status->repos_text_status = status->repos_text_status;
      m_status->repos_prop_status = status->repos_prop_status;
    }
  }

  Status &
  Status::operator=(const Status & status)
  {
    if (this == &status)
      return *this;

    init (status.m_path->data, status.m_status);
    return *this;
  }
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
