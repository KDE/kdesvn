/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
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
#ifndef CREATEREPO_IMPL_H
#define CREATEREPO_IMPL_H

#include "createrepo_dlg.h"

class Createrepo_impl: public CreateRepo_Dlg {
    Q_OBJECT
public:
    Createrepo_impl(bool enable_compat13,bool enable_compat14, QWidget *parent = 0, const char *name = 0);
    QString targetDir();
    QString fsType();
    bool disableFsync();
    bool keepLogs();
    bool createMain();
    bool compat13()const;
    bool compat14()const;

protected slots:
    virtual void fsTypeChanged(int);
    virtual void compatChanged14(bool);
    virtual void compatChanged13(bool);

protected:
    bool inChangeCompat;
};

#endif
