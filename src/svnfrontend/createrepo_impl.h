/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef CREATEREPO_IMPL_H
#define CREATEREPO_IMPL_H

#include "ui_createrepo_dlg.h"

#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/shared_pointer.hpp"

class CreateRepoData;

class Createrepo_impl: public QWidget, public Ui::CreateRepo_Dlg {
    Q_OBJECT
public:
    Createrepo_impl(QWidget *parent = 0, const char *name = 0);
    const svn::repository::CreateRepoParameter&parameter()const;
    bool createMain()const;
    QString targetDir()const;

protected Q_SLOTS:
    virtual void fsTypeChanged(int);
    virtual void compatChanged15(bool);
    virtual void compatChanged14(bool);
    virtual void compatChanged13(bool);

private:
    svn::SharedPointer<CreateRepoData> _data;

protected:
    void checkCompatList();
};

#endif
