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
#ifndef SVNQT_STATUS_H
#define SVNQT_STATUS_H

// subversion api
#include <svn_wc.h>

// svncpp
#include <svnqt/svnqttypes.h>
#include <svnqt/entry.h>
#include <svnqt/pool.h>
#include <svnqt/lock_entry.h>
#include <svnqt/dirent.h>
#include <svnqt/info_entry.h>
#include <svnqt/svnqt_defines.h>

namespace svn
{
/**
 * Subversion status API. This class wraps around
 * @a svn_wc_status_t.
 *
 * @see svn_wc.h
 * @see svn_wc_status_t
 */
class Status_private;

class SVNQT_EXPORT Status
{
public:
    /**
     * copy constructor
     */
    Status(const Status &src);

    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     */
    explicit Status(const QString &path = QString());
    /**
     * default constructor
     *
     * @param path path for this status entry
     * @param status status entry
     */
    explicit Status(const char *path, const svn_client_status_t *status);
    /**
     * converting constructor
     */
    Status(const QString &path, const DirEntry &src);
    /**
     * converting constructor
     */
    Status(const QString &path, const InfoEntry &src);

    /**
     * destructor
     */
    virtual ~Status();

    /**
     * @return path of status entry
     */
    const QString &
    path() const;

    /**
     * @return entry for this path
     * @retval entry.isValid () = false item is not versioned
     */
    const Entry &
    entry() const;
    /**
     * @return The status of the node, based on the restructuring changes and if the
     * node has no restructuring changes the text and prop status
     */
    svn_wc_status_kind
    nodeStatus() const;

    /**
     * @return file status property enum of the "textual" component.
     */
    svn_wc_status_kind
    textStatus() const;

    /**
     * @return file status property enum of the "property" component.
     */
    svn_wc_status_kind
    propStatus() const;

    /**
     * @retval TRUE if under version control
     */
    bool
    isVersioned() const;

    /**
     * @retval TRUE if under version control and not ignored
     */
    bool
    isRealVersioned()const;

    /**
     * @retval TRUE if under version control and local modified
     */
    bool
    isModified()const;

    /**
     * @retval TRUE if locked
     */
    bool
    isLocked() const;

    /**
     * @retval TRUE if copied
     */
    bool
    isCopied() const;

    /**
     * @retval TRUE if switched
     */
    bool
    isSwitched() const;
    /**
     * @return the entry's text status in the repository
     */
    svn_wc_status_kind
    reposTextStatus() const;
    /**
     * @return the entry's prop status in the repository
     */
    svn_wc_status_kind
    reposPropStatus() const;

    const LockEntry &
    lockEntry() const;

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
    Status_private *m_Data;
};
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
