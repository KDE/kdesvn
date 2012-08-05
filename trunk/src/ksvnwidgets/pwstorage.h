/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
#ifndef PWSTORAGE_H
#define PWSTORAGE_H

#include <qstring.h>
#include <qobject.h>

class PwStorageData;

/**
    Access to wallet isn't threadsafe 'cause wallet has not to be called from within threads!
 */
class PwStorage:public QObject
{
    Q_OBJECT
protected:
    PwStorageData* mData;
public:
    bool getCertPw(const QString&realm,QString&pw);
    bool getLogin(const QString&realm,QString&user,QString&pw);
    bool getCachedLogin(const QString&realm,QString&user,QString&pw);
    bool setCertPw(const QString&realm, const QString&pw);
    bool setLogin(const QString&realm,const QString&user,const QString&pw);
    bool setCachedLogin(const QString&realm,const QString&user,const QString&pw);
    bool connectWallet();

    static PwStorage*self();

protected:
    PwStorage();
    virtual ~PwStorage();
};

#endif

