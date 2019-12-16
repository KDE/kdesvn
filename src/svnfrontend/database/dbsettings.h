/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU General Public        *
 * License along with this program (in the file GPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#pragma once

#include "ksvndialog.h"

namespace Ui
{
class DbSettings;
}
class KEditListWidget;

class DbSettings: public KSvnDialog
{
    Q_OBJECT
public:
    static void showSettings(const QString &repository, QWidget *parent = nullptr);

protected Q_SLOTS:
    void accept() override final;
private:
    void init();
    explicit DbSettings(const QString &repository, QWidget *parent = nullptr);
    ~DbSettings();

    void store_list(KEditListWidget *, const QString &);
private:
    QString m_repository;
    Ui::DbSettings *m_ui;
};
