/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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

#include "svnthread.h"
#include "tcontextlistener.h"

#include "src/svnqt/url.h"

#include <kurl.h>
#include <kdebug.h>

SvnThread::SvnThread(QObject *_parent)
    : QThread(), m_Parent(_parent)
{
    m_CurrentContext = new svn::Context();

    m_SvnContextListener = new ThreadContextListener(m_Parent);
    if (m_Parent) {
        QObject::connect(m_SvnContextListener, SIGNAL(sendNotify(QString)), m_Parent, SLOT(slotNotifyMessage(QString)));
    }

    m_CurrentContext->setListener(m_SvnContextListener);
    m_Svnclient = svn::Client::getobject(m_CurrentContext, 0);
}

SvnThread::~SvnThread()
{
    m_CurrentContext->setListener(0);
    delete m_Svnclient;
    m_SvnContextListener = 0;
}

void SvnThread::cancelMe()
{
    m_SvnContextListener->setCanceled(true);
}

void SvnThread::itemInfo(const QString &what, svn::InfoEntry &target, const svn::Revision &_rev, const svn::Revision &_peg)
{
    QString url, cacheKey;
    svn::Revision rev = _rev;
    svn::Revision peg = _peg;

    if (!svn::Url::isValid(what)) {
        // working copy
        // url = svn::Wc::getUrl(what);
        url = what;
        if (url.indexOf("@") != -1) {
            url += "@BASE";
        }
        peg = svn::Revision::UNDEFINED;
        cacheKey = url;
    } else {
        KUrl _uri = what;
        QString prot = svn::Url::transformProtokoll(_uri.protocol());
        _uri.setProtocol(prot);
        url = _uri.prettyUrl();
        if (peg == svn::Revision::UNDEFINED) {
            peg = rev;
        }
        if (peg == svn::Revision::UNDEFINED) {
            peg = svn::Revision::HEAD;
        }
    }
    svn::InfoEntries _e;
    _e = (m_Svnclient->info(url, svn::DepthEmpty, rev, peg));
    if (_e.count() > 0) {
        target = _e[0];
    }
}
