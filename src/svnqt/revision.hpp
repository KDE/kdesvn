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

#ifndef _SVNCPP_REVISION_HPP_
#define _SVNCPP_REVISION_HPP_

// svncpp
#include <svnqt/datetime.hpp>
#include <svnqt/svnqt_defines.hpp>

// qt
#include <qglobal.h>
#if QT_VERSION < 0x040000
    #include <qstring.h>
    #include <qtextstream.h>
#else
    #include <QtCore>
#endif

// subversion api
#include "svn_types.h"
#include "svn_opt.h"

namespace svn
{
  /**
   * Class that encapsulates svn_opt_revnum_t.
   *
   * @see svn_opt_revnum_t
   */
  class SVNQT_EXPORT Revision
  {
  private:
    svn_opt_revision_t m_revision;

    void
    init (const svn_opt_revision_t * revision);

  public:
    static const svn_opt_revision_kind START;
    static const svn_opt_revision_kind BASE;
    static const svn_opt_revision_kind HEAD;
    static const svn_opt_revision_kind WORKING;
    static const svn_opt_revision_kind UNDEFINED;

    /**
     * Constructor
     *
     * @param revision revision information
     */
    Revision (const svn_opt_revision_t * revision);

    /**
     * Constructor
     *
     * @param revnum revision number
     */
    Revision (const svn_revnum_t revnum);

    /**
     * Constructor
     * @param revnum a revision number
     * @param revstring a revision string
     *
     * The revision string MUST uppercase, it may some of "HEAD", "BASE", "WORKING", "COMMITED", "PREV",
     * or a date in form {YYYY-MM-DD}.
     */
    Revision (const int revnum, const QString&revstring);

    /**
     * Constructor
     *
     * @param kind
     */
    Revision (const svn_opt_revision_kind kind = svn_opt_revision_unspecified);

    /**
     * Constructor
     *
     * @param dateTime DateTime wrapper for apr_time_t
     */
    Revision (const DateTime dateTime);

    /**
     * Copy constructor
     *
     * @param revision Source
     */
    Revision (const Revision & revision);

    /**
     * @return revision information
     */
    const svn_opt_revision_t *
    revision () const;

    /**
     * @see revision (). Same function
     * but with operator overloading
     */
    operator svn_opt_revision_t * ()
    {
      return &m_revision;
    }

    /**
     * @see revision (). Same function
     * but with operator overloading
     */
     operator const svn_opt_revision_t*()const
    {
      return &m_revision;
    }

    /**
     * @return revision numver
     */
    const svn_revnum_t
    revnum () const;

    /**
     * @return revision kind
     */
    const svn_opt_revision_kind
    kind () const;

    operator QString ()const;
    QString toString()const;

    /**
     * @return date
     */
    const apr_time_t
    date () const;

    bool operator==(const Revision&)const;
    bool operator!=(const svn_opt_revision_kind)const;
    bool operator==(const svn_opt_revision_kind)const;

  };
}

inline QTextStream& operator<<(QTextStream&s,svn::Revision&r)
{
    s << r.toString();
    return s;
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
