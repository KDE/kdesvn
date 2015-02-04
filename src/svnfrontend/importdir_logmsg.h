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
#ifndef IMPORTDIR_LOGMSG_H
#define IMPORTDIR_LOGMSG_H

#include "src/ksvnwidgets/commitmsg_impl.h"

class QCheckBox;
/**
@author Rajko Albrecht
*/
class Importdir_logmsg : public Commitmsg_impl
{
    Q_OBJECT
public:
    Importdir_logmsg(QWidget *parent = 0);

    virtual ~Importdir_logmsg();

    bool createDir();
    bool ignoreUnknownNodes();
    bool noIgnore();
    void createDirboxDir(const QString &which = QString());

protected:
    QCheckBox *m_createDirBox;
    QCheckBox *m_ignoreUnknownNodes;
    QCheckBox *m_noIgnore;
};

#endif
