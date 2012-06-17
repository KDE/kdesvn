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
#include "revision.h"
#include "pool.h"

// qt
#include "qdatetime.h"

namespace svn
{
  const svn_opt_revision_kind Revision::START = svn_opt_revision_number;
  const svn_opt_revision_kind Revision::BASE = svn_opt_revision_base;
  const svn_opt_revision_kind Revision::HEAD = svn_opt_revision_head;
  const svn_opt_revision_kind Revision::WORKING = svn_opt_revision_working;
  const svn_opt_revision_kind Revision::UNDEFINED = svn_opt_revision_unspecified;
  const svn_opt_revision_kind Revision::PREV = svn_opt_revision_previous;

  const svn_opt_revision_kind Revision::DATE = svn_opt_revision_date;
  const svn_opt_revision_kind Revision::NUMBER = Revision::START;


  Revision::Revision (const svn_opt_revision_t * revision)
  {
    init (revision);
  }

  Revision::Revision (const svn_revnum_t revnum)
  {
      if (revnum<0) {
        m_revision.kind = svn_opt_revision_unspecified;
        m_revision.value.number = 0;
      } else {
        m_revision.kind = svn_opt_revision_number;
        m_revision.value.number = revnum;
      }
  }

  Revision::Revision (const svn_opt_revision_kind kind)
  {
    m_revision.kind = kind;
    m_revision.value.number = 0;
  }

  Revision::Revision (const int revnum, const QString&revstring)
  {
    m_revision.kind = svn_opt_revision_unspecified;

    if (revnum > -1) {
        m_revision.kind = svn_opt_revision_number;
        m_revision.value.number = revnum;
    } else {
        assign(revstring);
    }
  }

  Revision::Revision (const QString&revstring)
  {
      assign(revstring);
  }

  void
  Revision::assign(const QString&revstring)
  {
    m_revision.kind = svn_opt_revision_unspecified;
    if (revstring.isEmpty()) {
        return;
    }
    if (revstring=="WORKING") {
        m_revision.kind = WORKING;
    } else if (revstring=="BASE") {
        m_revision.kind = BASE;
    } else if (revstring=="START"){
        m_revision.kind = Revision::START;
        m_revision.value.number = 0;
    } else if (revstring=="PREV"){
        m_revision.kind = Revision::PREV;
    } else if (!revstring.isNull()) {
        Pool pool;
        svn_opt_revision_t endrev;
        svn_opt_parse_revision(&m_revision,&endrev,revstring.TOUTF8(),pool);
    }
  }

  void
  Revision::assign(const QDateTime&dateTime)
  {
    m_revision.kind = svn_opt_revision_date;
    DateTime dt(dateTime);
    m_revision.value.date = dt.GetAPRTimeT();
  }

  Revision& Revision::operator=(const QString&what)
  {
    assign(what);
    return *this;
  }

  Revision::Revision (const DateTime dateTime)
  {
    m_revision.kind = svn_opt_revision_date;
    m_revision.value.date = dateTime.GetAPRTimeT();
  }

  Revision::Revision (const QDateTime&dateTime)
  {
    assign(dateTime);
  }

  Revision::Revision (const Revision & revision)
  {
    init (revision.revision ());
  }

  void
  Revision::init (const svn_opt_revision_t * revision)
  {
    if( !revision )
    {
      m_revision.kind = svn_opt_revision_unspecified;
    }
    else
    {
      m_revision.kind = revision->kind;

      // m_revision.value is a union so we are not
      // allowed to set number if we want to use date
      // and vice versa

      switch( revision->kind )
      {
      case svn_opt_revision_number:
        m_revision.value.number = revision->value.number;
        break;

      case svn_opt_revision_date:
        m_revision.value.date = revision->value.date;
        break;

      default:
        m_revision.value.number = 0;
      }
    }
  }

  Revision::operator QString ()const
  {
    return toString();
  }

  QString Revision::toString()const
  {
    QString value;
    QDateTime result;
    switch (m_revision.kind) {
    case svn_opt_revision_number:
        value.sprintf("%li",m_revision.value.number);
        break;
    case svn_opt_revision_date:
        value = DateTime(m_revision.value.date).toString("{yyyy-MM-dd}");
        break;
    case svn_opt_revision_base:
        value = "BASE";
        break;
    case svn_opt_revision_head:
        value = "HEAD";
        break;
    case svn_opt_revision_working:
        value = "WORKING";
        break;
    case svn_opt_revision_previous:
        value="PREVIOUS";
        break;
    case svn_opt_revision_unspecified:
    default:
        value="-1";
        break;
    }
    return value;
  }

  const svn_opt_revision_t *
  Revision::revision () const
  {
    return &m_revision;
  }

  svn_revnum_t
  Revision::revnum () const
  {
      if (m_revision.kind==svn_opt_revision_number) {
          return m_revision.value.number;
      }
      return SVN_INVALID_REVNUM;
  }

  apr_time_t
  Revision::date () const
  {
    return m_revision.value.date;
  }

  svn_opt_revision_kind
  Revision::kind () const
  {
    return m_revision.kind;
  }

  bool Revision::operator==(const Revision&r)const
  {
    if (r.kind()!=kind()) {
        return false;
    }
    if (m_revision.kind == svn_opt_revision_number) {
        return revnum()==r.revnum();
    } else if (m_revision.kind == svn_opt_revision_date) {
        return date()==r.date();
    }
    return true;
  }

  bool Revision::operator==(int value)const
  {
    if (m_revision.kind!=svn_opt_revision_number || value!=revnum()) {
        return false;
    }
    return true;
  }

  bool Revision::operator!=(const svn_opt_revision_kind t)const
  {
    return kind()!=t;
  }
  bool Revision::operator==(const svn_opt_revision_kind t)const
  {
    return kind()==t;
  }

  bool Revision::operator!()const
  {
    return kind()==UNDEFINED;
  }

  bool Revision::operator!()
  {
    return kind()==UNDEFINED;
  }

  Revision::operator bool()const
  {
    return kind()!=UNDEFINED;
  }

  Revision::operator bool()
  {
    return kind()!=UNDEFINED;
  }

  bool Revision::isRemote()const
  {
      return kind()!=UNDEFINED && kind()!=BASE && kind()!=WORKING;
  }

    bool Revision::isValid() const
    {
        return kind()!=UNDEFINED;
    }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
