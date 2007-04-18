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
#ifndef _SVNCPP_STATUS_HPP_
#define _SVNCPP_STATUS_HPP_

// subversion api
#include "svn_wc.h"

// svncpp
#include "svnqt/entry.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/lock_entry.hpp"
#include "svnqt/dirent.hpp"
#include "svnqt/info_entry.hpp"
#include "svnqt/svnqt_defines.hpp"

namespace svn
{
  /**
   * Subversion status API. This class wraps around
   * @a svn_wc_status_t.
   *
   * @see svn_wc.hpp
   * @see svn_wc_status_t
   */
  class Status_private;

  class SVNQT_EXPORT Status
  {
  public:
    /**
     * copy constructor
     */
    Status (const Status & src);

    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     */
    Status (const QString&path=QString::null, svn_wc_status2_t * status = NULL);
    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     */
    Status (const char*path, svn_wc_status2_t * status = NULL);
    /**
     * converting constructor
     */
    Status(const QString&path,const DirEntry&src);
    /**
     * converting constructor
     */
    Status(const QString&path,const InfoEntry&src);

    /**
     * destructor
     */
    virtual ~Status ();

    /**
     * @return path of status entry
     */
    const QString&
    path () const;

    /**
     * @return entry for this path
     * @retval entry.isValid () = false item is not versioned
     */
    const Entry&
    entry () const;
    /**
     * @return file status property enum of the "textual" component.
     */
    const svn_wc_status_kind
    textStatus () const;

    /**
     * @return file status property enum of the "property" component.
     */
    const svn_wc_status_kind
    propStatus () const;

    /**
     * @retval TRUE if under version control
     */
    const bool
    isVersioned () const;

    /**
     * @retval TRUE if under version control and not ignored
     */
    const bool
    isRealVersioned()const;

    /**
     * @retval TRUE if under version control and local modified
     */
    const bool
    isModified()const;

    /**
     * @retval TRUE if locked
     */
    const bool
    isLocked () const;

    /**
     * @retval TRUE if copied
     */
    const bool
    isCopied () const;

    /**
     * @retval TRUE if switched
     */
    const bool
    isSwitched () const;
    /**
     * @return the entry's text status in the repository
     */
    const svn_wc_status_kind
    reposTextStatus () const;
    /**
     * @return the entry's prop status in the repository
     */
    const svn_wc_status_kind
    reposPropStatus () const;

    const LockEntry&
    lockEntry () const;

    bool
    validReposStatus()const;

    bool
    validLocalStatus()const;


    /**
     * assignment operator
     */
    Status &
    operator = (const Status &);
  private:
    Status_private*m_Data;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
