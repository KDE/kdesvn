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

// apr
#include "apr_date.h"

// svncpp
#include "datetime.h"


namespace svn
{
  DateTime::DateTime ()
  : m_time()
  {
  }

  DateTime::DateTime (const apr_time_t time)
  : m_time()
  {
      setAprTime(time);
  }

  DateTime::DateTime(const QDateTime&dt)
    : m_time(dt)
  {
  }

  DateTime::DateTime (const DateTime & dateTime)
  : m_time(dateTime.m_time)
  {
  }

  DateTime::DateTime(const QString&dt)
  {
      SetRFC822Date(dt.toUtf8().constData());
  }
  
  const DateTime &
  DateTime::operator =(const DateTime & dateTime)
  {
    m_time = dateTime.m_time;
    return *this;
  }

  bool DateTime::operator<(const DateTime&dateTime)const
  {
    return m_time<dateTime.m_time;
  }

  bool DateTime::operator>(const DateTime&dateTime)const
  {
    return dateTime<*this;
  }

  bool DateTime::operator!=(const DateTime&dateTime)const
  {
    return *this<dateTime||dateTime<*this;
  }
  
  bool DateTime::operator==(const DateTime&dateTime)const
  {
    return !(*this!=dateTime);
  }

  bool
  DateTime::operator<=(const DateTime&dateTime)const
  {
      return *this==dateTime||*this<dateTime;
  }
  bool
  DateTime::operator>=(const DateTime&dateTime)const
  {
      return *this==dateTime||*this>dateTime;
  }

  bool
  DateTime::IsValid () const
  {
    return m_time.isValid();
  }

  apr_time_t
  DateTime::GetAPRTimeT () const
  {
    apr_time_t aTime;
    apr_time_ansi_put(&aTime,m_time.toTime_t());
    return aTime;
  }

  bool
  DateTime::SetRFC822Date (const char* date)
  {
    apr_time_t aTime = apr_date_parse_rfc(date);
    setAprTime(aTime);
    return IsValid();
  }

  DateTime::operator const QDateTime&()const
  {
    return m_time;
  }

  const QDateTime&DateTime::toQDateTime()const
  {
      return *this;
  }

  void DateTime::setAprTime(apr_time_t aTime)
  {
    m_time.setTimeSpec(Qt::LocalTime);
    if (aTime<0)m_time.setTime_t(0);
    else m_time.setTime_t(aTime/(1000*1000));
  }

  QString DateTime::toString(const QString&format)const
  {
    return m_time.toString(format);
  }
}

/*!
    \fn svn::DateTime::toTime_t()
 */
unsigned int svn::DateTime::toTime_t()const
{
    return m_time.toTime_t();
}

void svn::DateTime::setTime_t(unsigned int sec)
{
    m_time.setTime_t(sec);
}
