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
#ifndef LOGMSG_IMPL_H
#define LOGMSG_IMPL_H

#include "src/svnfrontend/logmessage.h"
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
    void setRecCheckboxtext(const QString&what);

protected slots:
    virtual void slotHistoryActivated(const QString&);

protected:
    static QValueList<QString> sLogHistory;
    static const char* groupName;
    static int smax_message_history;
};

#endif
