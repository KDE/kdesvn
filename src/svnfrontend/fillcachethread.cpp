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
#include "src/kdesvn_events.h"

#include <qobject.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

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

const QString&FillCacheThread::reposRoot()const
{
    return m_what;
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
    bool breakit=false;
    KApplication*k = KApplication::kApplication();

    try {
        svn::Revision latestCache = rl.latestCachedRev();
        svn::Revision Head = rl.latestHeadRev();
        Q_LLONG i = latestCache.revnum();
        Q_LLONG j = Head.revnum();

        Q_LLONG _max=j-i;
        Q_LLONG _cur=0;

        FillCacheStatusEvent*fev;
        if (k) {
            fev = new FillCacheStatusEvent(_cur,_max);
            k->postEvent(m_Parent,fev);
        }

        for (;i<j;i+=200) {
            _cur+=200;
            rl.fillCache(i);
            if (m_SvnContext->contextCancel()) {
                m_SvnContext->contextNotify(i18n("Filling cache canceld."));
                kdDebug()<<"Cancel thread"<<endl;
                breakit=true;
                break;
            }
            if (latestCache==rl.latestCachedRev()) {
                break;
            }
            if (k) {
                fev = new FillCacheStatusEvent(_cur>_max?_max:_cur,_max);
                k->postEvent(m_Parent,fev);
            }
            latestCache=rl.latestCachedRev();
        }
        rl.fillCache(Head);
        i=Head.revnum();
        m_SvnContext->contextNotify(i18n("Cache filled up to revision %1").arg(i));
    } catch (const svn::Exception&e) {
        m_SvnContext->contextNotify(e.msg());
    }
    if (k && !breakit) {
        QCustomEvent*ev = new QCustomEvent(EVENT_LOGCACHE_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
