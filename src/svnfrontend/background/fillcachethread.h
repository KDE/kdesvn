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
#ifndef FILLCACHE_THREAD_H
#define FILLCACHE_THREAD_H

#include "svnqt/client.h"
#include "svnqt/revision.h"
#include "svnqt/status.h"
#include "svnfrontend/frontendtypes.h"
#include "svnthread.h"

class QObject;

class FillCacheThread: public SvnThread
{
    Q_OBJECT
public:
    FillCacheThread(QObject *, const QString &aPath, bool startup);
    ~FillCacheThread();

    const QString &reposRoot()const;
    const QString &Path()const;

Q_SIGNALS:
    void fillCacheStatus(qlonglong current, qlonglong max);
    void fillCacheFinished();

protected:
    void run() override;
    void fillInfo();

    QString m_what;
    QString m_path;
    bool m_startup;
};

#endif
