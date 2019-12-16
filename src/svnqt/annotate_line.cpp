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

#include "annotate_line.h"

namespace svn
{
AnnotateLine::AnnotateLine()
    : m_line_no(0)
    , m_revision(0)
    , m_merge_revision(0)
{}

AnnotateLine::AnnotateLine(qlonglong line_no,
                           qlonglong revision,
                           const char *author,
                           const char *date,
                           const char *line,
                           qlonglong merge_revision,
                           const char *merge_author,
                           const char *merge_date,
                           const char *merge_path
                          )
    : m_line_no(line_no)
    , m_revision(revision)
    , m_date((date &&strlen(date)) ? QDateTime::fromString(QString::fromUtf8(date), Qt::ISODate) : QDateTime())
    , m_line(line ? line : "")
    , m_author(author ? author : "")
    , m_merge_revision(merge_revision)
    , m_merge_date((merge_date &&strlen(merge_date)) ? QDateTime::fromString(QString::fromUtf8(merge_date), Qt::ISODate) : QDateTime())
    , m_merge_author(merge_author ? merge_author : "")
    , m_merge_path(merge_path ? merge_path : "")
{}

AnnotateLine::AnnotateLine(qlonglong line_no,
                           qlonglong revision,
                           const PropertiesMap &revisionproperties,
                           const char *line,
                           qlonglong merge_revision,
                           const PropertiesMap &mergeproperties,
                           const char *merge_path,
                           qlonglong revstart,
                           qlonglong revend,
                           bool local
                           )
    : m_line_no(line_no)
    , m_revision(revision)
    , m_date(QDateTime())
    , m_line(line ? QByteArray(line) : QByteArray())
    , m_merge_revision(merge_revision)
    , m_merge_path(merge_path ? QByteArray(merge_path) : QByteArray())
{
    QString _s = revisionproperties[QStringLiteral("svn:author")];
    m_author = _s.toUtf8();
    _s = revisionproperties[QStringLiteral("svn:date")];
    if (!_s.isEmpty()) {
        m_date = QDateTime::fromString(_s, Qt::ISODate);
    }
    _s = mergeproperties[QStringLiteral("svn:author")];
    m_merge_author = _s.toUtf8();
    _s = mergeproperties[QStringLiteral("svn:date")];
    if (!_s.isEmpty()) {
        m_merge_date = QDateTime::fromString(_s, Qt::ISODate);
    }
}
}
