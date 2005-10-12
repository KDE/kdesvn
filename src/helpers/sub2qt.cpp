/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "sub2qt.h"

#include "svncpp/revision.hpp"
#include <qmap.h>

namespace helpers {

sub2qt::sub2qt()
{
}


sub2qt::~sub2qt()
{
}

QDateTime sub2qt::apr_time2qt(apr_time_t _time)
{
    QDateTime result;
    if (_time<=0) result.setTime_t(0,Qt::LocalTime);
    else result.setTime_t(_time/(1000*1000),Qt::LocalTime);
    return result;
}

/*!
    \fn helpers::sub2qt::qt_time2apr(const QDateTime&)
 */
apr_time_t sub2qt::qt_time2apr(const QDateTime&_time)
{
    apr_time_t r;
    apr_time_ansi_put(&r,_time.toTime_t());
    return r;
}

svn::Revision sub2qt::urlToRev(const KURL&url)
{
    svn::Revision r = svn::Revision::UNDEFINED;
    QMap<QString,QString> q = url.queryItems();
    if (q.find("rev")!=q.end()) {
        QString v = q["rev"];
        if (!v.isEmpty()) {
            r = stringToRev(v);
        }
    }
    return r;
}

svn::Revision sub2qt::stringToRev(const QString&v)
{
    svn::Revision r = svn::Revision::UNDEFINED;
    if (!QString::compare(v,"HEAD")) {
        r = svn::Revision::HEAD;
    } else if (!QString::compare(v,"BASE")) {
        r = svn::Revision::BASE;
    } else if (!QString::compare(v,"START")) {
        r = svn::Revision::START;
    } else if (!QString::compare(v,"WORKING")) {
        r = svn::Revision::WORKING;
    } else {
        r = v.toInt();
    }
    return r;
}

};

