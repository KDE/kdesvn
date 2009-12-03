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
#ifndef _SVNCPP_ANNOTATE_LINE_HPP_
#define _SVNCPP_ANNOTATE_LINE_HPP_

#include "svnqt/svnqt_defines.h"

#include <qstring.h>
#include <qdatetime.h>

namespace svn
{
  /**
   * This class holds the data for one line in an annotation
   */
  class AnnotateLine
  {
  public:
      AnnotateLine (QLONG line_no,
                    QLONG revision,
                    const char *author,
                    const char *date,
                    const char *line)
    : m_line_no (line_no), m_revision (revision),
      m_date( (date&&strlen(date))?QDateTime::fromString(QString::FROMUTF8(date),Qt::ISODate):QDateTime()),
      m_line(line?line:""),m_author(author?author:""),
      m_merge_revision(-1),
      m_merge_date(QDateTime()),
      m_merge_author(""),m_merge_path("")

    {
    }

    AnnotateLine (QLONG line_no,
                  QLONG revision,
                  const char *author,
                  const char *date,
                  const char *line,
                  QLONG merge_revision,
                  const char *merge_author,
                  const char *merge_date,
                  const char *merge_path
                 )
          : m_line_no (line_no), m_revision (revision),
            m_date( (date&&strlen(date))?QDateTime::fromString(QString::FROMUTF8(date),Qt::ISODate):QDateTime()),
            m_line(line?line:""),m_author(author?author:""),
            m_merge_revision(merge_revision),
            m_merge_date( (merge_date&&strlen(merge_date))?QDateTime::fromString(QString::FROMUTF8(merge_date),Qt::ISODate):QDateTime()),
            m_merge_author(merge_author?merge_author:""),m_merge_path(merge_path?merge_path:"")
        {
        }

    AnnotateLine ( const AnnotateLine &other)
          : m_line_no (other.m_line_no), m_revision (other.m_revision), m_date (other.m_date),
            m_line (other.m_line), m_author (other.m_author)
    {
    }
    AnnotateLine()
          : m_line_no(0),m_revision(-1),m_date(),
            m_line(), m_author()
    {
    }

    /**
     * destructor
     */
    virtual ~AnnotateLine ()
    {
    }

    QLONG
    lineNumber () const
    {
        return m_line_no;
    }
    QLONG
    revision () const
    {
        return m_revision;
    }


    const QByteArray &
    author () const
    {
        return m_author;
    }


    const QDateTime &
    date () const
    {
        return m_date;
    }


    const QByteArray &
    line () const
    {
        return m_line;
    }

  protected:
    QLONG m_line_no;
    QLONG m_revision;
    QDateTime m_date;
    QByteArray m_line;
    QByteArray m_author;

    QLONG m_merge_revision;
    QDateTime m_merge_date;
    QByteArray m_merge_author;
    QByteArray m_merge_path;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
