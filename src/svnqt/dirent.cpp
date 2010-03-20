/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
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

// svncpp
#include "svnqt/dirent.h"
#include "svnqt/lock_entry.h"
#include "svnqt/svnqt_defines.h"

#include <qstring.h>

namespace svn
{
  class SVNQT_NOEXPORT DirEntry_Data
  {
  public:
    QString name;
    svn_node_kind_t kind;
    QLONG size;
    bool hasProps;
    svn_revnum_t createdRev;
    DateTime time;
    QString lastAuthor;
    LockEntry m_Lock;

    DirEntry_Data ()
      : kind (svn_node_unknown), size (0), hasProps(false),
        createdRev (0), time (0), m_Lock()
    {
    }

    DirEntry_Data (const QString& _name, const svn_dirent_t * dirEntry)
      : name (_name), kind (dirEntry->kind), size (dirEntry->size),
        hasProps (dirEntry->has_props != 0),
        createdRev (dirEntry->created_rev), time (dirEntry->time), m_Lock()
    {
      lastAuthor = dirEntry->last_author == 0 ? QString::fromLatin1("") : QString::FROMUTF8(dirEntry->last_author);
    }

    DirEntry_Data (const DirEntry & src)
    {
      init (src);
    }

    void
    init (const DirEntry & src)
    {
      name = src.name ();
      kind = src.kind ();
      size = src.size ();
      hasProps = src.hasProps ();
      createdRev = src.createdRev ();
      time = src.time ();
      lastAuthor = src.lastAuthor ();
      m_Lock = src.lockEntry();
    }
  };

  DirEntry::DirEntry ()
    : m (new DirEntry_Data ())
  {
  }

  DirEntry::DirEntry (const QString& name, const svn_dirent_t * dirEntry)
    : m (new DirEntry_Data (name, dirEntry))
  {
  }

  DirEntry::DirEntry (const QString& name, const svn_dirent_t * dirEntry,const svn_lock_t*lockEntry)
    : m (new DirEntry_Data (name, dirEntry))
  {
    setLock(lockEntry);
  }

  DirEntry::DirEntry (const QString& name, const svn_dirent_t * dirEntry,const LockEntry&lockEntry)
    : m (new DirEntry_Data (name, dirEntry))
  {
    m->m_Lock = lockEntry;
  }

  DirEntry::DirEntry (const DirEntry & src)
    : m (new DirEntry_Data (src))
  {
  }

  DirEntry::~DirEntry ()
  {
    delete m;
  }

  svn_node_kind_t
  DirEntry::kind () const
  {
    return m->kind;
  }

  bool DirEntry::isDir()const
  {
	  return kind()==svn_node_dir;
  }

  QLONG
  DirEntry::size () const
  {
    return m->size;
  }

  bool
  DirEntry::hasProps () const
  {
    return m->hasProps;
  }

  svn_revnum_t
  DirEntry::createdRev () const
  {
    return m->createdRev;
  }

  const DateTime&
  DirEntry::time () const
  {
    return m->time;
  }

  const QString&
  DirEntry::lastAuthor () const
  {
    return m->lastAuthor;
  }

  const QString&
  DirEntry::name () const
  {
    return m->name;
  }

  const LockEntry&
  DirEntry::lockEntry() const
  {
      return m->m_Lock;
  }

  void
  DirEntry::setLock(const svn_lock_t*_l)
  {
     m->m_Lock.init(_l);
  }

  DirEntry &
  DirEntry::operator= (const DirEntry & dirEntry)
  {
    if (this == &dirEntry)
      return *this;

    m->init (dirEntry);
    return *this;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
