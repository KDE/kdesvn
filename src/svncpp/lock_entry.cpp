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

// stl
#include <string>

// svncpp
#include "svncpp/lock_entry.hpp"
#include "svncpp/pool.hpp"

// subversion api
#include "svn_time.h"
#include "svn_version.h"


namespace svn
{
  LockEntry::LockEntry ()
    : date(0),exp(0),owner(""),comment(""),token(""),locked(false)
  {
  }

  LockEntry::LockEntry (
    const apr_time_t lock_time,
    const apr_time_t expiration_time,
    const char * lock_owner,
    const char * lock_comment,
    const char * lock_token)
    : date(lock_time),exp(expiration_time),owner(lock_owner?lock_owner:""),comment(lock_comment?lock_comment:""),token(lock_token?lock_token:""),
      locked(lock_token?true:false)
  {
  }
  const std::string&LockEntry::Comment()const
  {
    return comment;
  }
  const std::string&LockEntry::Owner()const
  {
    return owner;
  }
  const std::string&LockEntry::Token()const
  {
    return token;
  }
  const apr_time_t LockEntry::Date()const
  {
    return date;
  }
  const bool LockEntry::Locked()const
  {
    return locked;
  }
  void LockEntry::init(const svn_wc_entry_t * src)
  {
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    if (src) {
      date = src->lock_creation_date;
      locked = src->lock_token?true:false;
      token = (src->lock_token?src->lock_token:"");
      comment = (src->lock_comment?src->lock_comment:"");
      owner = (src->lock_owner?src->lock_owner:"");
    } else {
#else
      date = 0;
      owner = "";
      comment = "";
      token = "";
      locked = false;
#endif
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    }
#endif
    exp = 0;
  }

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  void LockEntry::init(svn_lock_t* src)
  {
    if (src) {
      date = src->creation_date;
      locked = src->token?true:false;
      token = (src->token?src->token:"");
      comment = (src->comment?src->comment:"");
      owner = (src->owner?src->owner:"");
    } else {
      date = 0;
      exp = 0;
      owner = "";
      comment = "";
      token = "";
      locked = false;
    }

  }
#endif

  void LockEntry::init(
    const apr_time_t lock_time,
    const apr_time_t expiration_time,
    const char * lock_owner,
    const char * lock_comment,
    const char * lock_token)
  {
    date = lock_time;
    exp = expiration_time;
    locked = lock_token?true:false;
    token = lock_token?lock_token:"";
    owner = lock_owner?lock_owner:"";
    comment = lock_comment?lock_comment:"";
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
