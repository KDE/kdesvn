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

#include "ksvnjobview.h"

KsvnJobView::KsvnJobView(qulonglong id, const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
: org::kde::JobView(service,path,connection,parent),m_id(id),m_state(STOPPED),m_max(0)
{
    connect(this, SIGNAL(cancelRequested()),this,
                     SLOT(killJob()));
#if 0
        QObject::connect(jobView, SIGNAL(suspendRequested()), this,
                         SLOT(suspend()));
        QObject::connect(jobView, SIGNAL(resumeRequested()), this,
                         SLOT(resume()));
#endif
}

void KsvnJobView::killJob()
{
    m_state = CANCELD;
}

unsigned long KsvnJobView::percent(qulonglong amount)
{
    return (unsigned long)((float)(amount)/(float)(m_max)*100.0);
}

void KsvnJobView::setTotal(qlonglong amount)
{
    static const QString unit("bytes");
    m_max = amount;
    setTotalAmount(amount,unit);
}

#include "ksvnjobview.moc"
