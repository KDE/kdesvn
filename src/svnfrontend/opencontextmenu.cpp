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
#include "opencontextmenu.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KLocalizedString>

#include <QAction>

OpenContextmenu::OpenContextmenu(const QUrl &aPath, const KService::List &aList, QWidget *parent)
    : QMenu(parent)
    , m_Path(aPath)
    , m_List(aList)
{
    setup();
}

OpenContextmenu::~OpenContextmenu()
{
}

void OpenContextmenu::setup()
{
    m_mapPopup.clear();
    QStringList _found;
    for (const KService::Ptr &ptr : std::as_const(m_List)) {
        if (_found.contains(ptr->name())) {
            continue;
        }
        _found.append(ptr->name());
        QString actionName(ptr->name().replace(QLatin1Char('&'), QLatin1String("&&")));
        QAction *act = addAction(QIcon::fromTheme(ptr->icon()), actionName);
        act->setData(m_mapPopup.size());

        m_mapPopup.push_back(ptr);
    }
    connect(this, &QMenu::triggered, this, &OpenContextmenu::slotRunService);
    if (!m_List.isEmpty()) {
        addSeparator();
    }
    QAction *act = new QAction(i18n("Other..."), this);
    act->setData(-1);
    addAction(act);
}

void OpenContextmenu::slotRunService(QAction *act)
{
    const int idx = act->data().toInt();
    if (idx >= 0 && idx < m_mapPopup.size()) {
        auto *job = new KIO::ApplicationLauncherJob(m_mapPopup.at(idx));
        job->setUrls({m_Path});
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled, parentWidget()));
        job->start();
    } else {
        slotOpenWith();
    }
}

void OpenContextmenu::slotOpenWith()
{
    auto *job = new KIO::ApplicationLauncherJob;
    job->setUrls({m_Path});
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled, parentWidget()));
    job->start();
}

#include "moc_opencontextmenu.cpp"
