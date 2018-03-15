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
#ifndef DUMPREPO_IMPL_H
#define DUMPREPO_IMPL_H

#include "ui_dumprepo_dlg.h"

class DumpRepo_impl: public QWidget, public Ui::DumpRepoDlg
{
    Q_OBJECT
public:
    explicit DumpRepo_impl(QWidget *parent = nullptr);
    QString reposPath() const;
    QString targetFile() const;
    bool incremental() const;
    bool use_deltas() const;
    bool useNumbers() const;
    int startNumber() const;
    int endNumber() const;
protected slots:
    void slotDumpRange(bool);
};

#endif
