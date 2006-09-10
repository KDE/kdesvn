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

#ifndef _SVNCPP_DIRENT_HPP_
#define _SVNCPP_DIRENT_HPP_

// subversion api
#include "svn_client.h"
#include "lock_entry.hpp"
#include "svnqt_defines.hpp"

#include <qstring.h>

namespace svn
{
  class DirEntry_Data;

  class SVNQT_EXPORT DirEntry
  {
  public:
    /**
     * default constructor
     */
    DirEntry ();

    /**
     * constructor for existing @a svn_dirent_t entries
     */
    DirEntry (const QString& name, svn_dirent_t * dirEntry);
    /**
     * constructor for existing @a svn_dirent_t entries
     */
    DirEntry (const QString& name, svn_dirent_t * dirEntry,svn_lock_t*lockEntry);

    DirEntry (const QString& name, svn_dirent_t * dirEntry,const LockEntry&lockEntry);
    /**
     * copy constructor
     */
    DirEntry (const DirEntry & src);

    /**
     * destructor
     */
    ~DirEntry ();

    /**
     * assignment operator
     */
    DirEntry &
    operator = (const DirEntry &);

    const QString&
    name () const;

    svn_node_kind_t
    kind () const;

    svn_filesize_t
    size () const;

    bool
    hasProps () const;

    svn_revnum_t
    createdRev () const;

    apr_time_t
    time () const;

    const QString&
    lastAuthor () const;

    //! The assigned lock entry
    /*!
     * returns the assigned lock entry if set
     * \return a valid or an empty lock
     */
    const LockEntry&
    lockEntry() const;

    //! initialize and convert the internal lock entry
    /*!
     * This method should not needed to call outside the lib, it may just used
     * inside svn::Client::ls.
     * \param aLock the subversion lock description to convert.
     */
    void
    setLock(svn_lock_t*aLock);


  private:
    DirEntry_Data * m;

  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
