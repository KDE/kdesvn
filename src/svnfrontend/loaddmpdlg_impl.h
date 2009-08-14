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
#ifndef LOADDMPDLG_IMPL_H
#define LOADDMPDLG_IMPL_H

#include "ui_loaddmpdlg.h"

class LoadDmpDlg_impl: public QWidget, public Ui::LoadDmpDlg {
    Q_OBJECT
public:
    LoadDmpDlg_impl(QWidget *parent = 0, const char *name = 0);
    virtual ~LoadDmpDlg_impl();
    bool usePost()const;
    bool usePre()const;
    int uuidAction()const;
    KUrl dumpFile()const;
    QString repository()const;
    QString parentPath()const;

public slots:
};

#endif
