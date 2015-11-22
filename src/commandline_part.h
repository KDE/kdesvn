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
#ifndef COMMANDLINE_PART_H
#define COMMANDLINE_PART_H

#include <qobject.h>
#include <QVariantList>

class CommandExec;
class KCmdLineArgs;

/**
@author Rajko Albrecht
*/
class commandline_part : public QObject
{
    Q_OBJECT
public:
    explicit commandline_part(QObject *parent, const QVariantList &args = QVariantList());
    virtual ~commandline_part();
    virtual int exec(KCmdLineArgs *);
private:
    CommandExec *m_pCPart;

};

#endif
