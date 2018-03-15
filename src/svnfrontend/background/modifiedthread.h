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

#include "svnqt/client.h"
#include "svnqt/revision.h"
#include "svnqt/status.h"
#include "svnthread.h"

class CheckModifiedThread: public SvnThread
{
    Q_OBJECT
public:
    CheckModifiedThread(QObject *parent, const QString &what, bool updates);
    ~CheckModifiedThread();
    const svn::StatusEntries &getList()const;
Q_SIGNALS:
    void checkModifiedFinished();
protected:
    void run() override;

    QString m_what;
    bool m_updates;
    svn::StatusEntries m_Cache;
};

#endif

