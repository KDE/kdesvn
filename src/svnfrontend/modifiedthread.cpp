/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#include "tcontextlistener.h"

#include "src/svnqt/svnqttypes.hpp"

#include <qobject.h>
#include <kdebug.h>
#include <kapplication.h>

CheckModifiedThread::CheckModifiedThread(QObject*_parent,const QString&what,bool _updates)
    : QThread(),mutex()
{
    m_Parent = _parent;
    m_CurrentContext = new svn::Context();
    m_SvnContext = new ThreadContextListener(m_Parent,0);
    if (m_Parent) {
        QObject::connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),m_Parent,SLOT(slotNotifyMessage(const QString&)));
    }

    m_CurrentContext->setListener(m_SvnContext);
    m_what = what;
    m_Svnclient = svn::Client::getobject(m_CurrentContext,0);
    m_updates = _updates;
}

CheckModifiedThread::~CheckModifiedThread()
{
    delete m_Svnclient;
}

void CheckModifiedThread::cancelMe()
{
    // method is threadsafe!
    m_SvnContext->setCanceled(true);
}

const svn::StatusEntries&CheckModifiedThread::getList()const
{
    return m_Cache;
}

void CheckModifiedThread::run()
{
    // what must be cleaned!
    svn::Revision where = svn::Revision::HEAD;
    QString ex;
    try {
        //                                  rec  all    up        noign
        m_Cache = m_Svnclient->status(m_what,svn::DepthInfinity,false,m_updates,false,where);
    } catch (const svn::Exception&e) {
        m_SvnContext->contextNotify(e.msg());
    }
    KApplication*k = KApplication::kApplication();
    if (k) {
        QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
