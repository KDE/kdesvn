/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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

#include "getinfothread.h"
#include "svnfrontend/models/svnitemnode.h"
#include "svnqt/status.h"
#include "svnqt/url.h"
#include "tcontextlistener.h"

#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>

GetInfoThread::GetInfoThread(QObject *_parent)
    : SvnThread(_parent)
    , m_NodeQueue()
    , m_Cancel(false)
    , m_QueueLock()
    , m_CancelLock()
{
}

GetInfoThread::~GetInfoThread()
{
}

void GetInfoThread::run()
{
    svn::InfoEntry info;
    svn::Revision rev = svn::Revision::UNDEFINED;
    try {
        while (true) {
            {
                QReadLocker cl(&m_CancelLock);
                if (m_Cancel) {
                    break;
                }
            }
            SvnItemModelNode *current = nullptr;
            {
                QMutexLocker ml(&m_QueueLock);
                if (!m_NodeQueue.isEmpty()) {
                    current = m_NodeQueue.dequeue();
                }
            }
            if (current) {
                if (!current->hasToolTipText()) {
                    if (current->isRealVersioned() && !current->stat()->entry().url().isEmpty()) {
                        if (svn::Url::isValid(current->fullName())) {
                            rev = current->revision();
                        } else {
                            rev = svn::Revision::UNDEFINED;
                        }
                        itemInfo(current->fullName(), info, rev, current->correctPeg());
                    }
                    current->generateToolTip(info);
                }
            } else {
                break;
            }
        }
    } catch (const svn::Exception &e) {
        m_SvnContextListener->contextNotify(e.msg());
    }
}

void GetInfoThread::cancelMe()
{
    SvnThread::cancelMe();
    {
        QWriteLocker cl(&m_CancelLock);
        m_Cancel = true;
    }
}

void GetInfoThread::appendNode(SvnItemModelNode *node)
{
    if (!node) {
        return;
    }
    QMutexLocker ml(&m_QueueLock);
    bool found = false;
    for (const SvnItemModelNode *qNode : std::as_const(m_NodeQueue)) {
        if (qNode->fullName() == node->fullName()) {
            found = true;
            break;
        }
    }
    if (!found) {
        m_NodeQueue.enqueue(node);
    }
    m_SvnContextListener->setCanceled(false);
    if (!isRunning()) {
        {
            QWriteLocker cl(&m_CancelLock);
            m_Cancel = false;
        }
        start();
    }
}

void GetInfoThread::clearNodes()
{
    QMutexLocker ml(&m_QueueLock);
    m_NodeQueue.clear();
}

#include "moc_getinfothread.cpp"
