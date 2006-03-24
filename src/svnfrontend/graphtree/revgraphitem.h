/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#ifndef REVGRAPHITEM_H
#define REVGRAPHITEM_H

#include <qstring.h>
#include <qdatetime.h>

namespace svn {
    class LogEntry;
}
/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class RevGraphItem{
public:
    RevGraphItem();
    RevGraphItem(const QString&,const QString&,const svn::LogEntry&);
    virtual ~RevGraphItem();

protected:
    QString m_Name,m_Author,m_Message,m_Action;
    QDateTime m_ChangeDate;
    long m_Revision;
};

#endif
