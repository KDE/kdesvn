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
#include "entry.hpp"


namespace svn
{
    class Entry_private
    {
    public:
        Entry_private();
        Entry_private(const Entry_private&src);
        virtual ~Entry_private();

        bool m_valid;
        LockEntry m_Lock;

        QString _name,_url,_repos,_uuid,_copyfrom_url,_conflict_old,_conflict_new,_conflict_wrk,_prejfile,_checksum,_cmt_author;
        bool _copied,_deleted,_absent,_incomplete;
        svn_revnum_t _revision,_copyfrom_rev,_cmt_rev;
        svn_node_kind_t _kind;
        svn_wc_schedule_t _schedule;
        apr_time_t _text_time,_prop_time,_cmt_date;

        /**
        * initializes the members
        */
        void
        init (const svn_wc_entry_t * src);
        void
        init(const Entry_private&src);
        void
        init(const QString&url,const DirEntry&src);
        void
        init(const QString&url,const InfoEntry&src);
    };

    Entry_private::Entry_private()
        : m_valid (false),m_Lock()
    {
    }

    Entry_private::Entry_private(const Entry_private&src)
        : m_valid (false),m_Lock()
    {
        init(src);
    }

    Entry_private::~Entry_private()
    {
    }

    void
    Entry_private::init (const svn_wc_entry_t * src)
    {
        if (src) {
            // copy & convert the contents of src
            _name = QString::fromUtf8(src->name);
            _revision = src->revision;
            _url = QString::fromUtf8(src->url);
            _repos = QString::fromUtf8(src->repos);
            _uuid = QString::fromUtf8(src->uuid);
            _kind = src->kind;
            _schedule = src->schedule;
            _copied = src->copied!=0;
            _deleted = src->deleted!=0;
            _absent = src->absent!=0;
            _incomplete = src->incomplete!=0;
            _copyfrom_url=QString::fromUtf8(src->copyfrom_url);
            _copyfrom_rev = src->copyfrom_rev;
            _conflict_old = QString::fromUtf8(src->conflict_old);
            _conflict_new = QString::fromUtf8(src->conflict_new);
            _conflict_wrk = QString::fromUtf8(src->conflict_wrk);
            _prejfile = QString::fromUtf8(src->prejfile);
            _text_time = src->text_time;
            _prop_time = src->prop_time;
            _checksum = QString::fromUtf8(src->checksum);
            _cmt_rev = src->cmt_rev;
            _cmt_date = src->cmt_date;
            _cmt_author = QString::fromUtf8(src->cmt_author);
            m_Lock.init(src);
            m_valid = true;
        } else {
            m_valid = false;
            m_Lock=LockEntry();
            _name=
            _url=_repos=_uuid=_copyfrom_url=_conflict_old=_conflict_new=_conflict_wrk=_prejfile=_checksum=_cmt_author= QString::null;
            _copied=_deleted=_absent=_incomplete = false;
            _kind = svn_node_unknown;
            _schedule=svn_wc_schedule_normal;
            _text_time=_prop_time=_cmt_date=0;
        }
    }
    void
    Entry_private::init(const Entry_private&src)
    {
        _name=src._name;
        _url=src._url;
        _repos=src._repos;
        _uuid=src._uuid;
        _copyfrom_url=src._copyfrom_url;
        _conflict_old=src._conflict_old;
        _conflict_new=src._conflict_new;
        _conflict_wrk=src._conflict_wrk;
        _prejfile=src._prejfile;
        _checksum=src._checksum;
        _cmt_author=src._cmt_author;
        _copied=src._copied;
        _deleted=src._deleted;
        _absent=src._absent;
        _incomplete=src._incomplete;
        _revision=src._revision;
        _copyfrom_rev=src._copyfrom_rev;
        _cmt_rev=src._cmt_rev;
        _kind=src._kind;
        _schedule=src._schedule;
        _text_time=src._text_time;
        _prop_time=src._prop_time;
        _cmt_date=src._cmt_date;
        _kind = src._kind;
        m_Lock=src.m_Lock;
        m_valid=src.m_valid;
    }

    void Entry_private::init(const QString&url,const DirEntry&dirEntry)
    {
        init(0);
        _url = url;
        _name=dirEntry.name();
        _revision = dirEntry.createdRev();
        _kind = dirEntry.kind();
        _schedule = svn_wc_schedule_normal;
        _text_time = dirEntry.time ();
        _prop_time = dirEntry.time ();
        _cmt_rev = dirEntry.createdRev ();
        _cmt_date = dirEntry.time ();
        _cmt_author = dirEntry.lastAuthor ();
        m_Lock=dirEntry.lockEntry();
        m_valid = true;
    }

    void Entry_private::init(const QString&url,const InfoEntry&src)
    {
        init(0);
        _name = src.Name();
        _url = url;
        _revision = src.revision();
        _kind = src.kind ();
        _schedule = svn_wc_schedule_normal;
        _text_time = src.textTime ();
        _prop_time = src.propTime ();
        _cmt_rev = src.cmtRev ();
        _cmt_date = src.cmtDate();
        _cmt_author = src.cmtAuthor();
        m_Lock=src.lockEntry();
        m_valid = true;
    }

  Entry::Entry (const svn_wc_entry_t * src)
    : m_Data(new Entry_private())
  {
    m_Data->init (src);
  }

  Entry::Entry (const Entry & src)
    : m_Data(new Entry_private())
  {
    if (src.m_Data) {
        m_Data->init(*(src.m_Data));
    } else {
        m_Data->init(0);
    }
  }

  Entry::Entry (const QString&url,const DirEntry&src)
    : m_Data(new Entry_private())
  {
    m_Data->init(url,src);
  }

  Entry::Entry (const QString&url,const InfoEntry&src)
    : m_Data(new Entry_private())
  {
    m_Data->init(url,src);
  }

  Entry::~Entry ()
  {
  }

  Entry &
  Entry::operator = (const Entry & src)
  {
    if (this == &src)
      return *this;
    if (src.m_Data) {
        m_Data->init(*(src.m_Data));
    } else {
        m_Data->init(0);
    }
    return *this;
  }

  const LockEntry&
  Entry::lockEntry()const
  {
      return m_Data->m_Lock;
  }

  const QString&
  Entry::cmtAuthor () const
  {
    return m_Data->_cmt_author;
  }

  const apr_time_t
  Entry::cmtDate () const
  {
    return m_Data->_cmt_date;
  }

  const svn_revnum_t
  Entry::cmtRev () const
  {
    return m_Data->_cmt_rev;
  }
  const QString&
  Entry::checksum () const
  {
    return m_Data->_checksum;
  }

  const apr_time_t
  Entry::propTime () const
  {
    return m_Data->_prop_time;
  }

    const apr_time_t
    Entry::textTime () const
    {
      return m_Data->_text_time;
    }
    const QString&
    Entry::prejfile () const
    {
      return m_Data->_prejfile;
    }
    const QString&
    Entry::conflictWrk () const
    {
      return m_Data->_conflict_wrk;
    }

    const QString&
    Entry::conflictNew () const
    {
      return m_Data->_conflict_new;
    }
    const QString&
    Entry::conflictOld () const
    {
      return m_Data->_conflict_old;
    }
    const svn_revnum_t
    Entry::copyfromRev () const
    {
      return m_Data->_copyfrom_rev;
    }
    const QString&
    Entry::copyfromUrl () const
    {
      return m_Data->_copyfrom_url;
    }

    const bool
    Entry::isAbsent () const
    {
      return m_Data->_absent;
    }
    const bool
    Entry::isDeleted () const
    {
      return m_Data->_deleted != 0;
    }
    const bool
    Entry::isCopied () const
    {
      return m_Data->_copied != 0;
    }
    const svn_wc_schedule_t
    Entry::schedule () const
    {
      return m_Data->_schedule;
    }
    const svn_node_kind_t
    Entry::kind () const
    {
      return m_Data->_kind;
    }
    const QString&
    Entry::uuid () const
    {
      return m_Data->_uuid;
    }
    const QString&
    Entry::repos () const
    {
      return m_Data->_repos;
    }
    const QString&
    Entry::url () const
    {
      return m_Data->_url;
    }
    const svn_revnum_t
    Entry::revision () const
    {
      return m_Data->_revision;
    }
    const QString&
    Entry::name () const
    {
      return m_Data->_name;
    }

    bool Entry::isValid () const
    {
      return m_Data->m_valid;
    }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
