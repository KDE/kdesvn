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

#ifndef SVNQT_DATETIME_H
#define SVNQT_DATETIME_H

#include <svnqt/svnqt_defines.h>

#include <QDateTime>

// subversion api
#include <svn_types.h>

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
    DateTime();

    /**
     * Constructor
     *
     * @param time number of microseconds since 00:00:00 january 1, 1970 UTC
     */
    explicit DateTime(const apr_time_t time);

    /**
     * Constructor
     *
     * @param dt QDateTime class
     */
    explicit DateTime(const QDateTime &dt);

    /**
     * Constructor
     * @param dt RFC822 compatible string
     */
    explicit DateTime(const QString &dt);

    /**
     * @return Is a valid (non-zero) date
     */
    bool IsValid() const { return m_time.isValid(); }

    /**
     * @return APR apr_time_t
     */
    apr_time_t GetAPRTimeT() const;

    /**
     * @return QDateTime object
     */
    const QDateTime &toQDateTime()const { return m_time; }

    /**
     * @param format format string
     * @return formatted string
     * @see QDateTime::toString
     */
    QString toString(const QString &format)const;
    QString toString(Qt::DateFormat f = Qt::DefaultLocaleShortDate) const;

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
    bool SetRFC822Date(const char *date);

    void setAprTime(apr_time_t aTime);
    //unsigned int toTime_t()const;
    //void setTime_t(unsigned int sec);
};
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
