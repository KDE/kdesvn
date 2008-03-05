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
#ifndef _FILLCACHE_THREAD_H
#define _FILLCACHE_THREAD_H

#include "src/svnqt/client.hpp"
#include "src/svnqt/revision.hpp"
#include "src/svnqt/status.hpp"
#include "ccontextlistener.h"
#include "eventnumbers.h"

#include <qthread.h>
#include <qevent.h>

class QObject;
class ThreadContextListener;

class FillCacheThread:public QThread
{
public:
    FillCacheThread(QObject*,const QString&reposRoot);
    virtual ~FillCacheThread();
    virtual void run();
    virtual void cancelMe();

protected:
    QMutex mutex;
    svn::Client* m_Svnclient;
    svn::ContextP m_CurrentContext;
    svn::smart_pointer<ThreadContextListener> m_SvnContext;
    QObject*m_Parent;
    QString m_what;
};

#endif

