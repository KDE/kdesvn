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

#ifndef SVNQT_REVISION_H
#define SVNQT_REVISION_H

// svncpp
#include <svnqt/datetime.h>
#include <svnqt/svnqt_defines.h>

#include <QString>
#include <QDateTime>
#include <QTextStream>

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

    void
    assign(const QString&);

    void
    assign(const QDateTime&);

  public:
    /*!
     * \defgroup Predefinedrevisions Predefined revision
     *
     * defines some well-known revision and revision-types for easier use.
     */
    /*@{*/
    //! Describes the start revision
    static const svn_opt_revision_kind START;
    //! Describes the base revision (eg, last update of working copy)
    static const svn_opt_revision_kind BASE;
    //! Describes HEAD revision of repository, eg. latest commit into repository
    static const svn_opt_revision_kind HEAD;
    //! Describes current working state of working copy
    static const svn_opt_revision_kind WORKING;
    //! Describes not know revision
    static const svn_opt_revision_kind UNDEFINED;
    //! Defines the revision before current head.
    static const svn_opt_revision_kind PREV;
    //! the revision contains a date.
    /*!
     * When Revision is of this type the date() methode returns a valid value.
     * \sa date()
     */
    static const svn_opt_revision_kind DATE;
    //! Revision contains a revision number
    /*!
     * When revision is of this type revnum() returns a valid value.
     * @sa revnum()
     */
    static const svn_opt_revision_kind NUMBER;
    /*@}*/

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
     * @param revstring a revision string
     *
     * The revision string MUST uppercase, it may some of "HEAD", "BASE", "WORKING", "COMMITED", "PREV",
     * or a date in form {YYYY-MM-DD}.
     */
    Revision (const QString&revstring);

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
     * @todo change it to referenced parameter (requires interface upgrade of lib)
     */
    Revision (const DateTime dateTime);
    /**
     * Constructor
     *
     * @param dateTime QDateTime type
     */
    Revision (const QDateTime&dateTime);

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
    svn_revnum_t
    revnum () const;

    /**
     * @return revision kind
     */
    svn_opt_revision_kind
    kind () const;

    operator QString ()const;
    QString toString()const;

    bool isRemote()const;
    bool isValid()const;

    /**
     * @return valid date if kind is Revision::DATE
     */
    apr_time_t
    date () const;

    bool operator==(const Revision&)const;
    bool operator!=(const svn_opt_revision_kind)const;
    bool operator==(const svn_opt_revision_kind)const;
    bool operator==(int)const;

    bool operator!()const;
    bool operator!();
    operator bool()const;
    operator bool();

    /**
     * assignment operator
     * @param what a simple revision string (not s:e but s)
     * @return object itself
     */
    Revision& operator=(const QString&what);

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
