/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
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
#include "entry.h"

namespace svn
{
class SVNQT_NOEXPORT Entry_private
{
protected:
    void init_clean();
public:
    Entry_private();

    bool m_valid;
    LockEntry m_Lock;

    QUrl _url, _repos;
    QString _name, _uuid, _cmt_author;
    bool _copied;
    svn_revnum_t _revision, _cmt_rev;
    svn_node_kind_t _kind;
    DateTime _cmt_date;

    /**
    * initializes the members
    */
    void
    init(const svn_client_status_t *src);
    void
    init(const Entry_private &src);
    void
    init(const QString &url, const DirEntry &src);
    void
    init(const QString &url, const InfoEntry &src);
};

void Entry_private::init_clean()
{
    _name.clear();
    _url.clear();
    _repos.clear();
    _uuid.clear();
    _cmt_author.clear();
    _revision = _cmt_rev = SVN_INVALID_REVNUM;
    _kind = svn_node_unknown;
    _cmt_date = DateTime();
    _copied = false;
}

Entry_private::Entry_private()
    : m_valid(false), m_Lock()
{
    init_clean();
}

void
Entry_private::init(const svn_client_status_t *src)
{
    if (src) {
        // copy & convert the contents of src
        _name = QString::fromUtf8(src->local_abspath);
        _revision = src->revision;
        _repos = QUrl::fromEncoded(src->repos_root_url);
        _url = _repos;
        _url.setPath(_url.path() +  QLatin1Char('/') + QString::fromUtf8(src->repos_relpath));
        _uuid = QString::fromUtf8(src->repos_uuid);
        _kind = src->kind;
        _copied = src->copied != 0;
        _cmt_rev = src->changed_rev;
        _cmt_date = DateTime(src->changed_date);
        _cmt_author = QString::fromUtf8(src->changed_author);
        m_Lock.init(src->lock);
        m_valid = true;
    } else {
        init_clean();
    }
}

void
Entry_private::init(const Entry_private &src)
{
    _name = src._name;
    _url = src._url;
    _repos = src._repos;
    _uuid = src._uuid;
    _cmt_author = src._cmt_author;
    _copied = src._copied;
    _revision = src._revision;
    _cmt_rev = src._cmt_rev;
    _kind = src._kind;
    _cmt_date = src._cmt_date;
    m_Lock = src.m_Lock;
    m_valid = src.m_valid;
}

void Entry_private::init(const QString &url, const DirEntry &dirEntry)
{
    init_clean();
    _url = QUrl(url);
    if (!dirEntry.isEmpty()) {
        _name = dirEntry.name();
        _revision = dirEntry.createdRev();
        _kind = dirEntry.kind();
        _cmt_rev = dirEntry.createdRev();
        _cmt_date = dirEntry.time();
        _cmt_author = dirEntry.lastAuthor();
        m_Lock = dirEntry.lockEntry();
        m_valid = true;
    }
}

void Entry_private::init(const QString &url, const InfoEntry &src)
{
    init(nullptr);
    _name = src.Name();
    _url = QUrl(url);
    _revision = src.revision();
    _kind = src.kind();
    _cmt_rev = src.cmtRev();
    _cmt_date = src.cmtDate();
    _cmt_author = src.cmtAuthor();
    m_Lock = src.lockEntry();
    m_valid = true;
}

Entry::Entry(const svn_client_status_t *src)
    : m_Data(new Entry_private())
{
    m_Data->init(src);
}

Entry::Entry(const Entry &src)
    : m_Data(new Entry_private())
{
    if (src.m_Data) {
        m_Data->init(*(src.m_Data));
    } else {
        m_Data->init(nullptr);
    }
}

Entry::Entry(const QString &url, const DirEntry &src)
    : m_Data(new Entry_private())
{
    m_Data->init(url, src);
}

Entry::Entry(const QString &url, const InfoEntry &src)
    : m_Data(new Entry_private())
{
    m_Data->init(url, src);
}

Entry::~Entry()
{
    delete m_Data;
}

Entry &
Entry::operator = (const Entry &src)
{
    if (this == &src) {
        return *this;
    }
    if (src.m_Data) {
        m_Data->init(*(src.m_Data));
    } else {
        m_Data->init(nullptr);
    }
    return *this;
}

const LockEntry &
Entry::lockEntry()const
{
    return m_Data->m_Lock;
}

const QString &
Entry::cmtAuthor() const
{
    return m_Data->_cmt_author;
}

const DateTime &
Entry::cmtDate() const
{
    return m_Data->_cmt_date;
}

svn_revnum_t
Entry::cmtRev() const
{
    return m_Data->_cmt_rev;
}

bool
Entry::isCopied() const
{
    return m_Data->_copied != 0;
}

svn_node_kind_t
Entry::kind() const
{
    return m_Data->_kind;
}
const QString &
Entry::uuid() const
{
    return m_Data->_uuid;
}
const QUrl &
Entry::repos() const
{
    return m_Data->_repos;
}
const QUrl &
Entry::url() const
{
    return m_Data->_url;
}
svn_revnum_t
Entry::revision() const
{
    return m_Data->_revision;
}
const QString &
Entry::name() const
{
    return m_Data->_name;
}

bool Entry::isValid() const
{
    return m_Data->m_valid;
}
}

/*!
    \fn svn::Entry::isDir()
 */
bool svn::Entry::isDir() const
{
    return kind() == svn_node_dir;
}

/*!
    \fn svn::Entry::isFile()
 */
bool svn::Entry::isFile() const
{
    return kind() == svn_node_file;
}
