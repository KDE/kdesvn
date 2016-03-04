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

#pragma once

#include <QString>
#include <QPointer>

class KConfig;
class QWidget;

/**
 * @brief A small helper class to restore/save the dialog dimensions
 */
class WindowGeometryHelper
{
public:
    WindowGeometryHelper(QWidget *w, const QString &groupName);
    ~WindowGeometryHelper() = default;

    static void restore(QWidget *w, const QString &groupName);
    static void save(QWidget *w, const QString &groupName);

    // no need to call restore() - already called in ctor when bAutoRestore = true
    void restore();
    void save();
private:
    QPointer<QWidget> m_widget;
    const KConfig *m_config;
    QString m_groupName;
};
