/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * https://kde.org/applications/development/org.kde.kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
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

#include "datetime.h"

// apr
#include <apr_date.h>

namespace svn
{
DateTime::DateTime()
    : m_time()
{
}

DateTime::DateTime(const apr_time_t time)
    : m_time()
{
    setAprTime(time);
}

DateTime::DateTime(const QDateTime &dt)
    : m_time(dt)
{
}

DateTime::DateTime(const QString &dt)
{
    SetRFC822Date(dt.toUtf8().constData());
}

apr_time_t
DateTime::GetAPRTimeT() const
{
    apr_time_t aTime;
    apr_time_ansi_put(&aTime, m_time.toSecsSinceEpoch());
    return aTime;
}

bool
DateTime::SetRFC822Date(const char *date)
{
    apr_time_t aTime = apr_date_parse_rfc(date);
    setAprTime(aTime);
    return IsValid();
}

void DateTime::setAprTime(apr_time_t aTime)
{
    if (aTime < 0) {
        m_time = QDateTime();
    } else {
        m_time = QDateTime::fromMSecsSinceEpoch(aTime / 1000);  // microsec -> millisec
    }
    m_time.setTimeSpec(Qt::LocalTime);
}

QString DateTime::toString(const QString &format)const
{
    return m_time.toString(format);
}

QString DateTime::toString(Qt::DateFormat f) const
{
    return m_time.toString(f);
}
} // namespace svn
