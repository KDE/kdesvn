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
#include "revgraphitem.h"
#include "svnqt/log_entry.hpp"
#include "helpers/sub2qt.h"

RevGraphItem::RevGraphItem()
    : m_Name(QString::null),m_Author(QString::null),m_Message(QString::null),
        m_Action(QString::null),m_ChangeDate(),m_Revision(-1)
{
}

RevGraphItem::RevGraphItem(const QString&_name,const QString&_action,const svn::LogEntry&source)
    : m_Name(_name),m_Author(source.author),m_Message(source.message),
    m_Action(_action),m_ChangeDate(helpers::sub2qt::apr_time2qt(source.date)),m_Revision(source.revision)
{
}

RevGraphItem::~RevGraphItem()
{
}
