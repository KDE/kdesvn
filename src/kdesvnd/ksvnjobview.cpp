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

#include "ksvnjobview.h"
#include <klocalizedstring.h>

KsvnJobView::KsvnJobView(qulonglong id, const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : org::kde::JobViewV2(service, path, connection, parent), m_id(id), m_state(STOPPED), m_max(0)
{
    connect(this, &OrgKdeJobViewV2Interface::cancelRequested, this,
            &KsvnJobView::killJob);
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

unsigned long KsvnJobView::percent(qulonglong amount) const
{
    return static_cast<unsigned long>(amount / m_max * 100.0);
}

void KsvnJobView::setTotal(qlonglong amount)
{
    m_max = amount;
    setTotalAmount(amount, i18n("bytes"));
}
