/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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

#ifndef COPYMOVEVIEW_IMPL_H
#define COPYMOVEVIEW_IMPL_H

#include "ui_copymoveview.h"

class CopyMoveView_impl : public QWidget, public Ui::CopyMoveView
{
    Q_OBJECT

public:
    CopyMoveView_impl(const QString &baseName, const QString &sourceName, bool move,
                      QWidget *parent);
    ~CopyMoveView_impl();
    QString newName() const;
    bool force() const;
    static QString getMoveCopyTo(bool *ok, bool move, const QString &old,
                                 const QString &base, QWidget *parent = nullptr);
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/

protected:
    QString m_OldName;
    QString m_BaseName;
};

#endif

