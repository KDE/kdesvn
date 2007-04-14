/* 
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
 * http://www.alwins-world.de/wiki/programs/kdesvn
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

#ifndef _SVNCPP_LOCK_ENTRY_H_
#define _SVNCPP_LOCK_ENTRY_H_

#include "svnqt/svnqt_defines.hpp"

#include <qstring.h>

// apr
#include "apr_time.h"

// subversion api
#include "svn_types.h"
#include "svn_wc.h"

namespace svn
{
  class SVNQT_EXPORT LockEntry
  {
  public:
    LockEntry ();

    LockEntry (const apr_time_t lock_time,
              const apr_time_t expiration_time,
              const char * lock_owner,
              const char * lock_comment,
              const char * lock_token);

    void init(const svn_wc_entry_t * src);

    void init(const apr_time_t lock_time,
              const apr_time_t expiration_time,
              const char * lock_owner,
              const char * lock_comment,
              const char * lock_token);
    void init(svn_lock_t*);
    const QString&Comment()const;
    const QString&Owner()const;
    const QString&Token()const;
    const apr_time_t Date()const;
    const apr_time_t Expiration()const;
    const bool Locked()const;

  protected:
    apr_time_t date;
    apr_time_t exp;
    QString owner;
    QString comment;
    QString token;
    bool locked;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

