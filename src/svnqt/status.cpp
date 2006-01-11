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
#include "status.hpp"

//#include <assert.h>

namespace svn
{
  class Status_private
  {
  public:
    Status_private();
    virtual ~Status_private();
    /**
     * Initialize structures
     *
     * @param path
     * @param status if NULL isVersioned will be false
     */
    void
    init (const QString&path, const svn_wc_status2_t * status);
    void
    init (const QString&path,const Status_private&src);
    void
    init(const QString&url,const DirEntry&src);
    void
    init(const QString&url,const InfoEntry&src);

    QString m_Path;
    bool m_isVersioned;
    bool m_hasReal;
    LockEntry m_Lock;
    Entry m_entry;

    svn_wc_status_kind _text_status,_prop_status,_repos_text_status,_repos_prop_status;
    bool _copied,_switched;
  };

  Status_private::Status_private()
    :m_Path(),m_isVersioned(false),m_hasReal(false)
  {
  }

  Status_private::~ Status_private()
  {
  }

  void Status_private::init (const QString&path, const svn_wc_status2_t * status)
  {
    m_Path = path;

    if (!status)
    {
      m_isVersioned = false;
      m_hasReal = false;
      m_entry=Entry();
      m_Lock = LockEntry();
    }
    else
    {
      m_isVersioned = status->text_status > svn_wc_status_unversioned;
      m_hasReal = m_isVersioned &&
                          status->text_status!=svn_wc_status_ignored;
      // now duplicate the contents
      if (status->entry)
      {
        m_entry = Entry(status->entry);
      } else {
        m_entry=Entry();
      }
      _text_status = status->text_status;
      _prop_status = status->prop_status;
      _copied = status->copied!=0;
      _switched = status->switched!=0;
      _repos_text_status = status->repos_text_status;
      _repos_prop_status = status->repos_prop_status;
      if (status->repos_lock) {
        m_Lock.init(status->repos_lock->creation_date,
                    status->repos_lock->expiration_date,
                    status->repos_lock->owner,
                    status->repos_lock->comment,
                    status->repos_lock->comment);
      } else {
        m_Lock=LockEntry();
      }
    }
  }

  void
  Status_private::init (const QString&path,const Status_private&src)
  {
    m_Path=path;
    m_Lock=src.m_Lock;
    m_entry=src.m_entry;
    m_isVersioned=src.m_isVersioned;
    m_hasReal=src.m_hasReal;
    _text_status=src._text_status;
    _prop_status=src._prop_status;
    _repos_text_status=src._repos_text_status;
    _repos_prop_status=src._repos_prop_status;
    _copied=src._copied;
    _switched=src._switched;
  }

  void Status_private::init(const QString&url,const DirEntry&src)
  {
    m_entry=Entry(url,src);
    m_Path=url;
    _text_status = svn_wc_status_normal;
    _prop_status = svn_wc_status_normal;
    m_Lock=LockEntry();
    _switched = false;
    _repos_text_status = svn_wc_status_normal;
    _repos_prop_status = svn_wc_status_normal;
    m_isVersioned=true;
    m_hasReal=true;
  }

  void Status_private::init(const QString&url,const InfoEntry&src)
  {
    m_entry=Entry(url,src);
    m_Path=url;
    m_Lock=src.lockEntry();
    _text_status = svn_wc_status_normal;
    _prop_status = svn_wc_status_normal;
    _repos_text_status = svn_wc_status_normal;
    _repos_prop_status = svn_wc_status_normal;
    m_isVersioned=true;
    m_hasReal=true;
  }

  Status::Status (const Status & src)
    : m_Data(new Status_private())
  {
    if( &src != this )
    {
        if (src.m_Data) {
             m_Data->init (src.m_Data->m_Path, *(src.m_Data));
        } else {
            m_Data->init(src.m_Data->m_Path,0);
        }
    }
  }

  Status::Status (const QString&path, svn_wc_status2_t * status)
    : m_Data(new Status_private())
  {
    m_Data->init(path, status);
  }

  Status::Status (const char*path, svn_wc_status2_t * status)
    : m_Data(new Status_private())
  {
    m_Data->init(QString::fromUtf8(path),status);
  }

  Status::Status(const QString&url,const DirEntry&src)
    : m_Data(new Status_private())
  {
    m_Data->init(url,src);
  }

  Status::Status(const QString&url,const InfoEntry&src)
    : m_Data(new Status_private())
  {
    m_Data->init(url,src);
  }

  Status::~Status ()
  {
    delete m_Data;
  }

  Status &
  Status::operator=(const Status & status)
  {
    if (this == &status)
      return *this;
    if (status.m_Data) {
        m_Data->init (status.m_Data->m_Path, *(status.m_Data));
    } else {
        m_Data->init(status.m_Data->m_Path,0);
    }
    return *this;
  }

  const LockEntry&
  Status::lockEntry () const
  {
      return m_Data->m_Lock;
  }
    const svn_wc_status_kind
    Status::reposPropStatus () const
    {
      return m_Data->_repos_prop_status;
    }
    const svn_wc_status_kind
    Status::reposTextStatus () const
    {
      return m_Data->_repos_text_status;
    }
    const bool
    Status::isSwitched () const
    {
      return m_Data->_switched != 0;
    }
    const bool
    Status::isCopied () const
    {
      return m_Data->_copied;
    }

    const bool
    Status::isLocked () const
    {
      return m_Data->m_Lock.Locked();
    }

    const bool
    Status::isRealVersioned()const
    {
      return m_Data->m_hasReal;
    }

    const bool
    Status::isVersioned () const
    {
      return m_Data->m_isVersioned;
    }

    const svn_wc_status_kind
    Status::propStatus () const
    {
      return m_Data->_prop_status;
    }

    const svn_wc_status_kind
    Status::textStatus () const
    {
      return m_Data->_text_status;
    }

    const Entry&
    Status::entry () const
    {
      return m_Data->m_entry;
    }

    const QString&
    Status::path () const
    {
      return m_Data->m_Path;
    }
}
