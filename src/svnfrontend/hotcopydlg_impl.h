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
#ifndef HOTCOPYDLG_IMPL_H
#define HOTCOPYDLG_IMPL_H

#include "ui_hotcopydlg.h"

#include <QString>
#include <QWidget>

class HotcopyDlg_impl : public QWidget, public Ui::HotcopyDlg
{
    Q_OBJECT
public:
    explicit HotcopyDlg_impl(QWidget *parent = nullptr);
    ~HotcopyDlg_impl() override;

    QString srcPath() const;
    QString destPath() const;
    bool cleanLogs() const;
};

#endif
