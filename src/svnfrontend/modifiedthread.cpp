/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

#include "modifiedthread.h"
#include "tcontextlistener.h"
#include "src/kdesvn_events.h"

#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/client_parameter.hpp"

#include <qobject.h>
#include <kdebug.h>
#include <kapplication.h>

CheckModifiedThread::CheckModifiedThread(QObject*_parent,const QString&what,bool _updates)
    : QThread(),mutex(),m_ContextListener(0)
{
    m_Parent = _parent;
    m_CurrentContext = new svn::Context();
    m_ContextListener = new ThreadContextListener(m_Parent);
    QObject::connect(m_ContextListener,SIGNAL(sendNotify(const QString&)),m_Parent,SLOT(slotNotifyMessage(const QString&)));

    m_CurrentContext->setListener(m_ContextListener);
    m_what = what;
    m_Svnclient = svn::Client::getobject(m_CurrentContext,0);
    m_updates = _updates;
}

CheckModifiedThread::~CheckModifiedThread()
{
    m_CurrentContext->setListener(0);
    delete m_Svnclient;
    m_ContextListener=0;
}

void CheckModifiedThread::cancelMe()
{
    // method is threadsafe!
    m_ContextListener->setCanceled(true);
}

const svn::StatusEntries&CheckModifiedThread::getList()const
{
    return m_Cache;
}

void CheckModifiedThread::run()
{
    // what must be cleaned!
    QString ex;
    svn::StatusParameter params(m_what);
    try {
        m_Cache = m_Svnclient->status(params.depth(svn::DepthInfinity).all(false).update(m_updates).noIgnore(false).revision(svn::Revision::HEAD));
    } catch (const svn::Exception&e) {
        m_ContextListener->contextNotify(e.msg());
    }
    KApplication*k = KApplication::kApplication();
    if (k) {
        DataEvent*ev = new DataEvent(m_updates?EVENT_UPDATE_CACHE_FINISHED:EVENT_CACHE_THREAD_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
