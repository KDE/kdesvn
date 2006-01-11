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
#ifndef _SVNCPP_ANNOTATE_LINE_HPP_
#define _SVNCPP_ANNOTATE_LINE_HPP_

#include <qstring.h>

namespace svn
{
  /**
   * This class holds the data for one line in an annotation
   */
  class AnnotateLine
  {
  public:
    AnnotateLine (apr_int64_t line_no,
                  svn_revnum_t revision,
                  const char *author,
                  const char *date,
                  const char *line)
    : m_line_no (line_no), m_revision (revision),
      m_author(QString::fromUtf8(author)),
      m_date(QString::fromUtf8(date)),
      m_line(QString::fromUtf8(line))
    {
    }

    AnnotateLine ( const AnnotateLine &other)
    : m_line_no (other.m_line_no), m_revision (other.m_revision),
      m_author (other.m_author), m_date (other.m_date),
      m_line (other.m_line)
    {
    }
    AnnotateLine()
    : m_line_no(0),m_revision(-1),
      m_author(""),m_date(""),m_line(0)
    {
    }

    /**
     * destructor
     */
    virtual ~AnnotateLine ()
    {
    }

    apr_int64_t
    lineNumber () const
    {
        return m_line_no;
    }
    svn_revnum_t
    revision () const
    {
        return m_revision;
    }


    const QString &
    author () const
    {
        return m_author;
    }


    const QString &
    date () const
    {
        return m_date;
    }


    const QString &
    line () const
    {
        return m_line;
    }

  private:
    apr_int64_t m_line_no;
    svn_revnum_t m_revision;
    QString m_author;
    QString m_date;
    QString m_line;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
