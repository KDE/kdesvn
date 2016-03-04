/***************************************************************************
 *   Copyright (C) 2016 Christian Ehrlicher <ch.ehrlicher@gmx.de>          *
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

#include "windowgeometryhelper.h"
#include "settings/kdesvnsettings.h"

#include <KConfig>
#include <KConfigGroup>
#include <KWindowConfig>
#include <QWidget>
#include <QWindow>

WindowGeometryHelper::WindowGeometryHelper(QWidget *w, const QString &groupName)
  : m_widget(w)
  , m_config(Kdesvnsettings::self()->config())
  , m_groupName(groupName)
{
   restore();
}

void WindowGeometryHelper::restore(QWidget *w, const QString &groupName)
{
    KConfigGroup kcg(Kdesvnsettings::self()->config(), groupName);
    KWindowConfig::restoreWindowSize(w->windowHandle(), kcg);
    w->resize(w->windowHandle()->size()); // needed, see https://git.reviewboard.kde.org/r/119594/
}

void WindowGeometryHelper::save(QWidget *w, const QString &groupName)
{
    KConfigGroup kcg(Kdesvnsettings::self()->config(), groupName);
    KWindowConfig::saveWindowSize(w->windowHandle(), kcg);
}

void WindowGeometryHelper::restore()
{
    if (!m_widget) {
        return;
    }
    KConfigGroup kcg(m_config, m_groupName);
    KWindowConfig::restoreWindowSize(m_widget->windowHandle(), kcg);
    m_widget->resize(m_widget->windowHandle()->size()); // needed, see https://git.reviewboard.kde.org/r/119594/
}

void WindowGeometryHelper::save()
{
    if (!m_widget) {
        return;
    }
    KConfigGroup kcg(m_config, m_groupName);
    KWindowConfig::saveWindowSize(m_widget->windowHandle(), kcg);
}
