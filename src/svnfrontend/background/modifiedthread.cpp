/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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

#include "modifiedthread.h"

#include "svnqt/svnqttypes.h"
#include "svnqt/client_parameter.h"

// CheckModifiedThread
CheckModifiedThread::CheckModifiedThread(QObject *parent, const QString &what, bool updates)
    : SvnThread(parent)
    , m_what(what)
    , m_updates(updates)
{}

CheckModifiedThread::~CheckModifiedThread()
{}

const svn::StatusEntries &CheckModifiedThread::getList()const
{
    return m_Cache;
}

void CheckModifiedThread::run()
{
    // what must be cleaned!
    svn::StatusParameter params(m_what);
    try {
        m_Cache = m_Svnclient->status(params.depth(svn::DepthInfinity).all(false).update(m_updates).noIgnore(false).revision(svn::Revision::HEAD));
    } catch (const svn::Exception &e) {
        m_SvnContextListener->contextNotify(e.msg());
    }
    emit checkModifiedFinished();
}
