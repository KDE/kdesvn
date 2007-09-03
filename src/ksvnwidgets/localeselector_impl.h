/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef LOCALESELECTOR_IMPL_H
#define LOCALESELECTOR_IMPL_H

#include "src/ksvnwidgets/localeselector.h"

class QTextCodec;

class LocaleSelector_impl: public LocaleSelector {
Q_OBJECT
public:
    LocaleSelector_impl(const QString&cur, QWidget *parent = 0, const char *name = 0);
    virtual ~LocaleSelector_impl(){}

protected slots:
    virtual void itemActivated(int);

signals:
    void TextCodecChanged(const QString&);
};

#endif
