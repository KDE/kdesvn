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
#ifndef MODIFIED_THREAD_H
#define MODIFIED_THREAD_H

#include "src/svnqt/client.h"
#include "src/svnqt/revision.h"
#include "src/svnqt/status.h"
#include "svnthread.h"

#include <qthread.h>
#include <qevent.h>

class QObject;

class CheckModifiedThread:public SvnThread
{
public:
    CheckModifiedThread(QObject*,const QString&what,bool _updates=false);
    virtual ~CheckModifiedThread();
    virtual void run();
    virtual const svn::StatusEntries&getList()const;

protected:
    QMutex mutex;
    QString m_what;
    bool m_updates;
    svn::StatusEntries m_Cache;
};

#endif

