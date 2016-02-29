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

#include <KConfig>
#include <KConfigGroup>
#include <KWindowConfig>
#include <QWidget>
#include <QWindow>

WindowGeometryHelper::WindowGeometryHelper(QWidget *w, const KConfig *config, const QString &groupName, bool bAutoRestore)
  : m_widget(w)
  , m_config(config)
  , m_groupName(groupName)
{
   if (bAutoRestore)
   {
       restore();
   }
}

void WindowGeometryHelper::restore()
{
    KConfigGroup kcg(m_config, m_groupName);
    KWindowConfig::restoreWindowSize(m_widget->windowHandle(), kcg);
    m_widget->resize(m_widget->windowHandle()->size()); // needed, see https://git.reviewboard.kde.org/r/119594/
}

void WindowGeometryHelper::save()
{
    KConfigGroup kcg(m_config, m_groupName);
    KWindowConfig::saveWindowSize(m_widget->windowHandle(), kcg);
}
