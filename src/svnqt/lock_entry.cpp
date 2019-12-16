/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * https://kde.org/applications/development/org.kde.kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
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
#include <svn_time.h>
#include <svn_version.h>

namespace svn
{
LockEntry::LockEntry()
    : date(0), exp(0), locked(false)
{
}

LockEntry::LockEntry(
    const apr_time_t lock_time,
    const apr_time_t expiration_time,
    const char *lock_owner,
    const char *lock_comment,
    const char *lock_token)
    : date(lock_time), exp(expiration_time),
      owner(lock_owner ? QString::fromUtf8(lock_owner) : QString()),
      comment(lock_comment ? QString::fromUtf8(lock_comment) : QString()),
      token(lock_token ? QString::fromUtf8(lock_token) : QString()),
      locked(lock_token ? true : false)
{
}
const QString &LockEntry::Comment()const
{
    return comment;
}
const QString &LockEntry::Owner()const
{
    return owner;
}
const QString &LockEntry::Token()const
{
    return token;
}
const DateTime &LockEntry::Date()const
{
    return date;
}
const DateTime &LockEntry::Expiration()const
{
    return exp;
}
bool LockEntry::Locked()const
{
    return locked;
}
void LockEntry::init(const svn_wc_entry_t *src)
{
    if (src) {
        date = DateTime(src->lock_creation_date);
        locked = src->lock_token ? true : false;
        token = (src->lock_token ? QString::fromUtf8(src->lock_token) : QString());
        comment = (src->lock_comment ? QString::fromUtf8(src->lock_comment) : QString());
        owner = (src->lock_owner ? QString::fromUtf8(src->lock_owner) : QString());
    } else {
        date = DateTime();
        owner.clear();
        comment.clear();
        token.clear();
        locked = false;
    }
    exp = DateTime();
}

void LockEntry::init(const svn_lock_t *src)
{
    if (src) {
        date = DateTime(src->creation_date);
        locked = src->token ? true : false;
        token = (src->token ? QString::fromUtf8(src->token) : QString());
        comment = (src->comment ? QString::fromUtf8(src->comment) : QString());
        owner = (src->owner ? QString::fromUtf8(src->owner) : QString());
    } else {
        date = DateTime();
        owner.clear();
        comment.clear();
        token.clear();
        locked = false;
    }
    exp = DateTime();
}

void LockEntry::init(
    const apr_time_t lock_time,
    const apr_time_t expiration_time,
    const char *lock_owner,
    const char *lock_comment,
    const char *lock_token)
{
    date = DateTime(lock_time);
    exp = DateTime(expiration_time);
    locked = lock_token ? true : false;
    token = lock_token ? QString::fromUtf8(lock_token) : QString();
    owner = lock_owner ? QString::fromUtf8(lock_owner) : QString();
    comment = lock_comment ? QString::fromUtf8(lock_comment) : QString();
}
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
