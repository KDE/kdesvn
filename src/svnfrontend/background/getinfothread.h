/***************************************************************************
 *   Copyright (C) 2009 by Rajko Albrecht  ral@alwins-world.de             *
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

#ifndef GETINFOTHREAD_H
#define GETINFOTHREAD_H

#include "svnthread.h"

#include <QThread>
#include <QEvent>
#include <QQueue>
#include <QMutex>
#include <QReadWriteLock>

class SvnItemModelNode;

class GetInfoThread: public SvnThread
{
    Q_OBJECT
public:
    explicit GetInfoThread(QObject *parent);
    ~GetInfoThread();
    void run() override;
    void cancelMe() override;

    void appendNode(SvnItemModelNode *);
    void clearNodes();

protected:
    QQueue<SvnItemModelNode *> m_NodeQueue;
    bool m_Cancel;
    QMutex m_QueueLock;
    QReadWriteLock m_CancelLock;
};

#endif
