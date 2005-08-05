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
#include "svncpp/entry.hpp"
#include "svncpp/pool.hpp"
#include "svncpp/lock_entry.hpp"

namespace svn
{
  /**
   * Subversion status API. This class wraps around
   * @a svn_wc_status_t.
   *
   * @see svn_wc.hpp
   * @see svn_wc_status_t
   */
  class Status
  {
  public:
    /**
     * copy constructor
     */
    Status (const Status & src);

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     */
    Status (const char *path = NULL, svn_wc_status2_t * status = NULL);
#else
    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     * @deprecated
     */
    Status (const char *path = NULL, svn_wc_status_t * status = NULL);
#endif
    /**
     * destructor
     */
    virtual ~Status ();

    /**
     * @return path of status entry
     */
    const char *
    path () const
    {
      return m_path->data;
    }

    /**
     * @return entry for this path
     * @retval entry.isValid () = false item is not versioned
     */
    const Entry
    entry () const
    {
      return Entry (m_status->entry);
    }

    /**
     * @return file status property enum of the "textual" component.
     */
    const svn_wc_status_kind
    textStatus () const
    {
      return m_status->text_status;
    }

    /**
     * @return file status property enum of the "property" component.
     */
    const svn_wc_status_kind
    propStatus () const
    {
      return m_status->prop_status;
    }

    /**
     * @retval TRUE if under version control
     */
    const bool
    isVersioned () const
    {
      return m_isVersioned;
    }

    /**
     * @retval TRUE if under version control and not ignored
     */
    const bool
    isRealVersioned()const
    {
      return m_hasReal;
    }
    /**
     * @retval TRUE if locked
     */
    const bool
    isLocked () const
    {
      return m_status->locked != 0;
    }

    /**
     * @retval TRUE if copied
     */
    const bool
    isCopied () const
    {
      return m_status->copied != 0;
    }

    /**
     * @retval TRUE if switched
     */
    const bool
    isSwitched () const
    {
      return m_status->switched != 0;
    }

    /**
     * @return the entry's text status in the repository
     */
    const svn_wc_status_kind
    reposTextStatus () const
    {
      return m_status->repos_text_status;
    }

    /**
     * @return the entry's prop status in the repository
     */
    const svn_wc_status_kind
    reposPropStatus () const
    {
      return m_status->repos_prop_status;
    }

    const LockEntry&
    lockEntry () const
    {
        return m_Lock;
    }
    /**
     * @return svn_wc_status2_t value
     * @since subversion 1.2
     */
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    operator svn_wc_status2_t * () const
    {
      return m_status;
    }
#else
    /**
     * @return svn_wc_status_t value
     * @deprecated Only if build against subversion prior 1.2
     */
    operator svn_wc_status_t * () const
    {
      return m_status;
    }
#endif
    /**
     * assignment operator
     */
    Status &
    operator = (const Status &);
  private:
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    svn_wc_status2_t * m_status;
#else
    svn_wc_status_t * m_status;
#endif
    svn_string_t * m_path;
    Pool m_pool;
    bool m_isVersioned;
    bool m_hasReal;
    LockEntry m_Lock;

    /**
     * Initialize structures
     *
     * @param path
     * @param status if NULL isVersioned will be false
     * @deprecated
     */
    void
    init (const char *path, const svn_wc_status_t * status);

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    /**
     * Initialize structures
     *
     * @param path
     * @param status if NULL isVersioned will be false
     */
    void
    init (const char *path, const svn_wc_status2_t * status);
#endif
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
