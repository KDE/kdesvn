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
#ifndef _SVNCPP_ENTRY_HPP_
#define _SVNCPP_ENTRY_HPP_

// subversion api
#include "svn_wc.h"

// svncpp
#include "pool.hpp"
#include "lock_entry.hpp"
#include "dirent.hpp"
#include "info_entry.hpp"

#include <qstring.h>

namespace svn
{
    class Entry_private;
  /**
   * C++ API for Subversion.
   * This class wraps around @a svn_wc_entry_t.
   */
  class Entry
  {
  public:
    /**
     * default constructor. if @a src is set,
     * copy its contents.
     *
     * If @a src is not set (=0) this will be
     * a non-versioned entry. This can be checked
     * later with @a isValid ().
     *
     * @param src another entry to copy from
     */
    Entry (const svn_wc_entry_t * src = 0);

    /**
     * copy constructor
     */
    Entry (const Entry & src);

    /**
     * converting constructr
     */
    Entry (const QString&url,const DirEntry&src);
    /**
     * converting constructr
     */
    Entry (const QString&url,const InfoEntry&src);

    /**
     * destructor
     */
    virtual ~Entry ();

    /**
     * returns whether this is a valid=versioned
     * entry.
     *
     * @return is entry valid
     * @retval true valid entry
     * @retval false invalid or unversioned entry
     */
    bool isValid () const;
    /**
     * @return entry's name
     */
    const QString&
    name () const;
    /**
     * @return base revision
     */
    const svn_revnum_t
    revision () const;
    /**
     * @return url in repository
     */
    const QString&
    url () const;

    /**
     * @return canonical repository url
     */
    const QString&
    repos () const;
    /**
     * @return repository uuid
     */
    const QString&
    uuid () const;
    /**
     * @return node kind (file, dir, ...)
     */
    const svn_node_kind_t
    kind () const;
    /**
     * @return scheduling (add, delete, replace)
     */
    const svn_wc_schedule_t
    schedule () const;
    /**
     * @return TRUE if copied
     */
    const bool
    isCopied () const;
    /**
     * @return true if deleted
     */
    const bool
    isDeleted () const;
    /**
     * @return true if deleted
     */
    const bool
    isAbsent () const;
    /**
     * @return copyfrom location
     */
    const QString&
    copyfromUrl () const;
    /**
     * @return copyfrom revision
     */
    const svn_revnum_t
    copyfromRev () const;
    /**
     * @return old version of conflicted file
     */
    const QString&
    conflictOld () const;
    /**
     * @return new version of conflicted file
     */
    const QString&
    conflictNew () const;
    /**
     * @return working version of conflicted file
     */
    const QString&
    conflictWrk () const;
    /**
     * @return property reject file
     */
    const QString&
    prejfile () const;
    /**
     * @return last up-to-date time for text contents
     * @retval 0 no information available
     */
    const apr_time_t
    textTime () const;
    /**
     * @return last up-to-date time for properties
     * @retval 0 no information available
     */
    const apr_time_t
    propTime()const;

    /**
     * @return base64 encoded checksum
     * @retval NULL for backwards compatibility
     */
    const QString&
    checksum () const;

    /**
     * @return last revision this was changed
     */
    const svn_revnum_t
    cmtRev () const;

    /**
     * @return last date this was changed
     */
    const apr_time_t
    cmtDate () const;

    /**
     * @return last commit author of this file
     */
    const QString&
    cmtAuthor () const;

    /**
     * @return lock for that entry
     * @since subversion 1.2
     */
    const LockEntry&
    lockEntry()const;

    /**
     * assignment operator
     */
    Entry &
    operator = (const Entry &);

  private:
    Entry_private*m_Data;
 };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
