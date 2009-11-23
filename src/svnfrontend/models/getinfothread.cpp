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

#include "getinfothread.h"
#include "svnitemnode.h"

#include <QMutexLocker>
#include <QReadLocker>
#include <QWriteLocker>

#include <kdebug.h>

GetInfoThread::GetInfoThread()
: QThread(),m_NodeQueue(),m_Cancel(false),m_QueueLock(),m_CancelLock()
{
}

GetInfoThread::~GetInfoThread()
{
}

void GetInfoThread::run()
{
    while (1) {
        kDebug()<<"Loop..."<<endl;
        {
            QReadLocker cl(&m_CancelLock);
            if (m_Cancel) {
                break;
            }
        }
        SvnItemModelNode*current = 0;
        {
            QMutexLocker ml(&m_QueueLock);
            if (m_NodeQueue.count()>0) {
                kDebug()<<m_NodeQueue.count();
                current = m_NodeQueue.dequeue ();
                kDebug()<<m_NodeQueue.count();
            }
        }
        if (current) {
            kDebug()<<"Got an item" << endl;
            if (!current->hasToolTipText()) {
                kDebug()<<"Get tooltip for "<<current->shortName();
                current->getToolTipText();
            } else {
                kDebug()<<"Found it for "<<current->shortName();
            }
        } else {
            break;
        }
    }
}

void GetInfoThread::cancelMe()
{
    QWriteLocker cl(&m_CancelLock);
    m_Cancel = true;
}

void GetInfoThread::appendNode(SvnItemModelNode*node)
{
    QMutexLocker ml(&m_QueueLock);
    m_NodeQueue.enqueue(node);
    if (!isRunning()) {
        kDebug()<<"Restart thread"<<endl;
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
