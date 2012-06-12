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

#ifndef SVNQT_DATETIME_H
#define SVNQT_DATETIME_H

#include "svnqt/svnqt_defines.h"

#include <qdatetime.h>

// subversion api
#include "svn_types.h"


namespace svn
{
  /**
   * Class that encapsulates apr_time_t.
   *
   * @see apr_time_t
   */
  class SVNQT_EXPORT DateTime
  {
  private:
    QDateTime m_time;

  public:

    /**
     * Default Constructor
     */
    DateTime ();

    /**
     * Constructor
     *
     * @param time number of microseconds since 00:00:00 january 1, 1970 UTC
     */
    DateTime (const apr_time_t time);

    /**
     * Constructor
     *
     * @param dt QDateTime class
     */
    DateTime(const QDateTime&dt);
    
    /**
     * Constructor
     * @param dt RFC822 compatible string
     */
    DateTime(const QString&dt);

    /**
     * Copy constructor
     *
     * @param dateTime Source
     */
    DateTime (const DateTime & dateTime);

    /**
     * @param dateTime Source
     */
    const DateTime &
    operator =(const DateTime & dateTime);

    bool
    operator<(const DateTime&dateTime)const;
    bool
    operator>(const DateTime&dateTime)const;
    bool
    operator!=(const DateTime&dateTime)const;
    bool
    operator==(const DateTime&dateTime)const;
    bool
    operator<=(const DateTime&dateTime)const;
    bool
    operator>=(const DateTime&dateTime)const;


    /**
     * @return Is a valid (non-zero) date
     */
    bool
    IsValid () const;

    /**
     * @return APR apr_time_t
     */
    apr_time_t
    GetAPRTimeT () const;

    /**
     * @return QDateTime object
     */
    operator const QDateTime&()const;

    /**
     * @return QDateTime object
     */
    const QDateTime&toQDateTime()const;

    /**
     * @param format format string
     * @return formatted string
     * @see QDateTime::toString
     */
    QString toString(const QString&format)const;

    /**
     * Set from date string of the form below, using apr_date_parse_rfc
     *
     * <PRE>
     *     Sun, 06 Nov 1994 08:49:37 GMT
     * </PRE>
     *
     * @see apr_date_parse_rfc
     * @return Successfully parsed
     */
    bool
    SetRFC822Date (const char* date);

    void setAprTime(apr_time_t aTime);
    unsigned int toTime_t()const;
    void setTime_t(unsigned int sec);
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
