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
#ifndef CREATEREPO_IMPL_H
#define CREATEREPO_IMPL_H

#include "ui_createrepo_dlg.h"

#include "src/svnqt/svnqttypes.h"

#include <QScopedPointer>

struct CreateRepoData;

class Createrepo_impl: public QWidget, public Ui::CreateRepo_Dlg
{
    Q_OBJECT
public:
    explicit Createrepo_impl(QWidget *parent = 0);
    ~Createrepo_impl();
    const svn::repository::CreateRepoParameter &parameter()const;
    bool createMain()const;
    QString targetDir()const;

protected Q_SLOTS:
    void fsTypeChanged(int);
    void compatChanged15(bool);
    void compatChanged14(bool);
    void compatChanged13(bool);

private:
    QScopedPointer<CreateRepoData> _data;

protected:
    void checkCompatList();
};

#endif
