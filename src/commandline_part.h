/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include <qstring.h>

class pCPart;
class KCmdLineArgs;

/**
@author Rajko Albrecht
*/
class commandline_part : public QObject
{
    Q_OBJECT
public:
    commandline_part(QObject *parent, const char *name, KCmdLineArgs *args);
    virtual ~commandline_part();
    virtual int exec();
private:
    pCPart*m_pCPart;

protected slots:
    virtual void clientException(const QString&);
    virtual void slotNotifyMessage(const QString&);
    virtual void slotCmd_log();
    virtual void slotCmd_update();
    virtual void slotCmd_diff();
    virtual void slotCmd_blame();

signals:
    void executeMe();
protected:
    virtual bool scanRevision();
};

#endif
