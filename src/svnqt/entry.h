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
#ifndef SVNQT_ENTRY_H
#define SVNQT_ENTRY_H

// svncpp
#include <svnqt/pool.h>
#include <svnqt/lock_entry.h>
#include <svnqt/dirent.h>
#include <svnqt/info_entry.h>
#include <svnqt/svnqt_defines.h>
#include <svnqt/svnqttypes.h>

// subversion api
#include <svn_wc.h>

#include <QString>

namespace svn
{
class Entry_private;
/**
 * C++ API for Subversion.
 * This class wraps around @a svn_wc_entry_t.
 */
class SVNQT_EXPORT Entry
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
    explicit Entry(const svn_client_status_t *src = nullptr);

    /**
     * copy constructor
     */
    Entry(const Entry &src);

    /**
     * converting constructr
     */
    Entry(const QString &url, const DirEntry &src);
    /**
     * converting constructr
     */
    Entry(const QString &url, const InfoEntry &src);

    /**
     * destructor
     */
    virtual ~Entry();

    /**
     * returns whether this is a valid=versioned
     * entry.
     *
     * @return is entry valid
     * @retval true valid entry
     * @retval false invalid or unversioned entry
     */
    bool isValid() const;
    /**
     * @return entry's name
     */
    const QString &
    name() const;
    /**
     * @return base revision
     */
    svn_revnum_t
    revision() const;
    /**
     * @return url in repository
     */
    const QUrl &url() const;

    /**
     * @return canonical repository url
     */
    const QUrl &repos() const;
    /**
     * @return repository uuid
     */
    const QString &
    uuid() const;
    /**
     * @return node kind (file, dir, ...)
     */
    svn_node_kind_t
    kind() const;

    /**
     * @return TRUE if copied
     */
    bool
    isCopied() const;

    /**
     * @return last revision this was changed
     */
    svn_revnum_t
    cmtRev() const;

    /**
     * @return last date this was changed
     */
    const DateTime &
    cmtDate() const;

    /**
     * @return last commit author of this file
     */
    const QString &
    cmtAuthor() const;

    /**
     * @return lock for that entry
     * @since subversion 1.2
     */
    const LockEntry &
    lockEntry()const;

    /**
     * @return true if entry is marked as dir
     */
    bool isDir()const;
    /**
     * assignment operator
     */
    Entry &
    operator = (const Entry &);
    bool isFile()const;

private:
    Entry_private *m_Data;
};

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
