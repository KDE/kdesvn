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
#include "fillcachethread.h"
#include "tcontextlistener.h"

#include "src/svnqt/cache/LogCache.h"
#include "src/svnqt/cache/ReposLog.h"
#include "src/svnqt/cache/ReposConfig.h"
#include "src/svnqt/cache/DatabaseException.h"
#include "src/kdesvn_events.h"
#include "src/svnqt/url.h"

#include <QObject>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>

FillCacheThread::FillCacheThread(QObject*_parent,const QString&aPath,bool startup)
    : SvnThread(_parent),mutex()
{
    setObjectName("fillcachethread");
    m_path = aPath;
    m_startup = startup;
}

const QString&FillCacheThread::Path()const
{
    return m_path;
}

FillCacheThread::~FillCacheThread()
{
}

const QString&FillCacheThread::reposRoot()const
{
    return m_what;
}

void FillCacheThread::fillInfo()
{
    svn::InfoEntry e;
    itemInfo(Path(),e);
    if (!e.reposRoot().isEmpty()) {
        m_what = e.reposRoot();
    }
}

void FillCacheThread::run()
{
    svn::Revision where = svn::Revision::HEAD;
    QString ex;
    bool breakit=false;
    KApplication*k = KApplication::kApplication();
    try {
        fillInfo();

        if (m_what.isEmpty() || svn::Url::isLocal(m_what) ) {
            return;
        }
        if (m_startup && svn::cache::ReposConfig::self()->readEntry(m_what,"no_update_cache",false)) {
            m_SvnContextListener->contextNotify(i18n("Not filling logcache because it is disabled due setting for this repository."));
        } else {
            m_SvnContextListener->contextNotify(i18n("Filling log cache in background"));

            svn::cache::ReposLog rl(m_Svnclient,m_what);
            svn::Revision latestCache = rl.latestCachedRev();
            svn::Revision Head = rl.latestHeadRev();
            qlonglong i = latestCache.revnum();
            if (i<0) {
                i=0;
            }
            qlonglong j = Head.revnum();

            qlonglong _max=j-i;
            qlonglong _cur=0;

            FillCacheStatusEvent*fev;
            if (k) {
                fev = new FillCacheStatusEvent(_cur,_max);
                k->postEvent(m_Parent,fev);
            }

            if (i<j) {
                for (;i<j;i+=200) {
                    _cur+=200;
                    rl.fillCache(i);

                    if (m_SvnContextListener->contextCancel()) {
                        m_SvnContextListener->contextNotify(i18n("Filling cache canceled."));
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
                if (latestCache.revnum()<Head.revnum()) {
                    rl.fillCache(Head.revnum());
                }
                i=Head.revnum();
                m_SvnContextListener->contextNotify(i18n("Cache filled up to revision %1",i));
            }
        }
    } catch (const svn::Exception&e) {
        m_SvnContextListener->contextNotify(e.msg());
    }
    if (k && !breakit) {
        DataEvent*ev = new DataEvent(EVENT_LOGCACHE_FINISHED);
        ev->setData((void*)this);
        k->postEvent(m_Parent,ev);
    }
}
