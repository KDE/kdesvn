/***************************************************************************
 *   Copyright (C) 2006-2010 by Rajko Albrecht                             *
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

#ifndef EDITIGNOREPATTERN_H_
#define EDITIGNOREPATTERN_H_

#include "ui_editignorepattern.h"

#include "svnqt/svnqttypes.h"

#include <QWidget>
#include <QStringList>

class EditIgnorePattern: public QWidget, public Ui::EditIgnorePattern
{
    Q_OBJECT

public:
    explicit EditIgnorePattern(QWidget *parent = nullptr);
    ~EditIgnorePattern();

    QStringList items()const;
    svn::Depth depth()const;
    bool unignore()const;
};

#endif /* EDITIGNOREPATTERN_H_ */
