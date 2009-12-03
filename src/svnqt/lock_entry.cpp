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

// svncpp
#include "lock_entry.h"
#include "pool.h"

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
    : date(lock_time),exp(expiration_time),
    owner(lock_owner?QString::FROMUTF8(lock_owner):""),
    comment(lock_comment?QString::FROMUTF8(lock_comment):""),
    token(lock_token?QString::FROMUTF8(lock_token):""),
      locked(lock_token?true:false)
  {
  }
  const QString&LockEntry::Comment()const
  {
    return comment;
  }
  const QString&LockEntry::Owner()const
  {
    return owner;
  }
  const QString&LockEntry::Token()const
  {
    return token;
  }
  const DateTime&LockEntry::Date()const
  {
    return date;
  }
  const DateTime&LockEntry::Expiration()const
  {
    return exp;
  }
  bool LockEntry::Locked()const
  {
    return locked;
  }
  void LockEntry::init(const svn_wc_entry_t * src)
  {
    if (src) {
      date = src->lock_creation_date;
      locked = src->lock_token?true:false;
      token = (src->lock_token?QString::FROMUTF8(src->lock_token):"");
      comment = (src->lock_comment?QString::FROMUTF8(src->lock_comment):"");
      owner = (src->lock_owner?QString::FROMUTF8(src->lock_owner):"");
    } else {
      date = 0;
      owner = "";
      comment = "";
      token = "";
      locked = false;
    }
    exp = 0;
  }

  void LockEntry::init(const svn_lock_t* src)
  {
    if (src) {
      date = src->creation_date;
      locked = src->token?true:false;
      token = (src->token?QString::FROMUTF8(src->token):"");
      comment = (src->comment?QString::FROMUTF8(src->comment):"");
      owner = (src->owner?QString::FROMUTF8(src->owner):"");
    } else {
      date = 0;
      exp = 0;
      owner = "";
      comment = "";
      token = "";
      locked = false;
    }

  }

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
    token = lock_token?QString::FROMUTF8(lock_token):"";
    owner = lock_owner?QString::FROMUTF8(lock_owner):"";
    comment = lock_comment?QString::FROMUTF8(lock_comment):"";
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
