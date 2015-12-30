/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#ifndef HELPERSSUB2QT_H
#define HELPERSSUB2QT_H

#include "src/svnqt/datetime.h"
#include "src/svnqt/targets.h"
#include <svn_time.h>

#include <QDateTime>
#include <QString>
#include <KUrl>

namespace helpers
{

/**
@author Rajko Albrecht
*/
namespace sub2qt
{
QString apr_time2qtString(apr_time_t _time);
QString DateTime2qtString(const svn::DateTime &_time);
/**
  * Convert a QStringList into a QVector<Path>
  */
svn::Targets fromStringList(const QStringList &paths);
/**
  * Convert a KUrl::List into a QVector<Path>
  */
svn::Targets fromUrlList(const KUrl::List &urls);
}

}
#endif
