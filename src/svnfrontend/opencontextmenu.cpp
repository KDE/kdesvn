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

#include <KIconLoader>
#include <KLocalizedString>
#include <KRun>
#include <QAction>
#include <QApplication>

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
    Q_FOREACH(const KService::Ptr &ptr, m_List) {
        if (_found.contains(ptr->name())) {
            continue;
        }
        _found.append(ptr->name());
        QString actionName(ptr->name().replace(QLatin1Char('&'), QLatin1String("&&")));
        QAction *act = addAction(SmallIcon(ptr->icon()), actionName);
        QVariant _data = m_mapPopup.size();
        act->setData(_data);

        m_mapPopup.push_back(ptr);
    }
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(slotRunService(QAction*)));
    if (!m_List.isEmpty()) {
        addSeparator();
    }
    QAction *act = new QAction(i18n("Other..."), this);
    QVariant _data = int(0);
    act->setData(_data);
    addAction(act);
}

void OpenContextmenu::slotRunService(QAction *act)
{
    const int idx = act->data().toInt();
    if (idx >= 0 && idx < m_mapPopup.size()) {
        KRun::runService(*m_mapPopup.at(idx), QList<QUrl>() << m_Path, QApplication::activeWindow());
    } else {
        slotOpenWith();
    }

}

void OpenContextmenu::slotOpenWith()
{
    QList<QUrl> lst;
    lst.append(m_Path);
    KRun::displayOpenWithDialog(lst, QApplication::activeWindow());
}
