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

#ifndef _SVNCPP_DATETIME_HPP_
#define _SVNCPP_DATETIME_HPP_ 

// subversion api
#include "svn_types.h"


namespace svn
{
  /**
   * Class that encapsulates apr_time_t.
   *
   * @see apr_time_t
   */
  class DateTime
  {
  private:
    apr_time_t m_time;

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

    /**
     * @param dateTime Comparator
     */
    const bool
    operator ==(const DateTime & dateTime);

    /**
     * @param dateTime Comparator
     */
    const bool
    operator !=(const DateTime & dateTime);

    /**
     * @return Is a valid (non-zero) date
     */
    const bool
    IsValid () const;

    /**
     * @return APR apr_time_t
     */
    const apr_time_t
    GetAPRTimeT () const;

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
    const bool
    SetRFC822Date (const char* date);
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
