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

// apr
#include "apr_date.h"

// svncpp
#include "datetime.hpp"


namespace svn
{
  DateTime::DateTime ()
  : m_time(APR_DATE_BAD)
  {
  }

  DateTime::DateTime (const apr_time_t time)
  : m_time(time)
  {
  }

  DateTime::DateTime(const QDateTime&dt)
    : m_time(0)
  {
    apr_time_ansi_put(&m_time,dt.toTime_t());
  }

  DateTime::DateTime (const DateTime & dateTime)
  : m_time(dateTime.m_time)
  {
  }

  const DateTime &
  DateTime::operator =(const DateTime & dateTime)
  {
    m_time = dateTime.m_time;
    return *this;
  }

  const bool DateTime::operator<(const DateTime&dateTime)const
  {
    return m_time<dateTime.m_time;
  }

  const bool DateTime::operator>(const DateTime&dateTime)const
  {
    return dateTime<*this;
  }

  const bool DateTime::operator!=(const DateTime&dateTime)const
  {
    return *this<dateTime||dateTime<*this;
  }
  const bool DateTime::operator==(const DateTime&dateTime)const
  {
    return !(*this!=dateTime);
  }

  const bool
  DateTime::operator ==(const DateTime & dateTime)
  {
    return m_time == dateTime.m_time;
  }

  const bool
  DateTime::operator !=(const DateTime & dateTime)
  {
    return m_time != dateTime.m_time;
  }

  const bool
  DateTime::IsValid () const
  {
    return m_time != APR_DATE_BAD;
  }

  const apr_time_t
  DateTime::GetAPRTimeT () const
  {
    return m_time;
  }

  const bool
  DateTime::SetRFC822Date (const char* date)
  {
    m_time = apr_date_parse_rfc(date);
    return IsValid();
  }

  DateTime::operator QDateTime()const
  {
    QDateTime result;
#if QT_VERSION < 0x040000
    if (m_time<0)result.setTime_t(0,Qt::LocalTime);
    else result.setTime_t(m_time/(1000*1000),Qt::LocalTime);
#else
    result.setTimeSpec(Qt::LocalTime);
    if (m_time<0)result.setTime_t(0);
    else result.setTime_t(m_time/(1000*1000));
#endif
    return result;
  }
  QString DateTime::toString(const QString&format)const
  {
    QDateTime dummy = *this;
    return dummy.toString(format);
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
