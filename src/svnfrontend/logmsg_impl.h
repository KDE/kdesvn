/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef LOGMSG_IMPL_H
#define LOGMSG_IMPL_H

#include "logmessage.h"
#include <qvaluelist.h>

class Logmsg_impl: public LogmessageData {
Q_OBJECT
public:
    Logmsg_impl(QWidget *parent = 0, const char *name = 0);
    QString getMessage()const;
    bool isRecursive()const;
    void initHistory();
    void saveHistory();

    static QString getLogmessage(bool*ok=0,bool*rec=0,QWidget*parent=0,const char*name=0);

protected slots:
    virtual void slotHistoryActivated(const QString&);

protected:
    static QValueList<QString> sLogHistory;
};

#endif
