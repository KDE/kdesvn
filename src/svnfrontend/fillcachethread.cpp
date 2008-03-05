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
#include "fillcachethread.h"
#include "tcontextlistener.h"

#include "src/svnqt/cache/LogCache.hpp"
#include "src/svnqt/cache/ReposLog.hpp"
#include "src/svnqt/cache/DatabaseException.hpp"

#include <qobject.h>
#include <kdebug.h>
#include <kapplication.h>

FillCacheThread::FillCacheThread(QObject*_parent,const QString&reposRoot)
    : QThread(),mutex()
{
    m_Parent = _parent;
    m_CurrentContext = new svn::Context();
    m_SvnContext = new ThreadContextListener(m_Parent,0);
    if (m_Parent) {
        QObject::connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),m_Parent,SLOT(slotNotifyMessage(const QString&)));
    }

    m_CurrentContext->setListener(m_SvnContext);
    m_what = reposRoot;
    m_Svnclient = svn::Client::getobject(m_CurrentContext,0);
}

FillCacheThread::~FillCacheThread()
{
    delete m_Svnclient;
}

void FillCacheThread::cancelMe()
{
    // method is threadsafe!
    m_SvnContext->setCanceled(true);
}

void FillCacheThread::run()
{
    svn::Revision where = svn::Revision::HEAD;
    QString ex;
    svn::cache::ReposLog rl(m_Svnclient,m_what);

    try {
        svn::Revision latestCache = rl.latestCachedRev();
        svn::Revision Head = rl.latestHeadRev();
        Q_LLONG i = latestCache.revnum();
        Q_LLONG j = Head.revnum();

        for (;i<j;i+=200) {
            rl.fillCache(i);
            if (m_SvnContext->contextCancel()||latestCache==rl.latestCachedRev()) {
                break;
            }
            latestCache=rl.latestCachedRev();
        }
        rl.fillCache(Head);
    } catch (const svn::ClientException&e) {
        m_SvnContext->contextNotify(e.msg());
    }
    KApplication*k = KApplication::kApplication();
    if (k) {
        QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
