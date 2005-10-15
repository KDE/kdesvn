/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
 *   rajko.albrecht@tecways.com                                            *
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


#ifndef _kdesvnd_dcop_H
#define _kdesvnd_dcop_H

#include <qstringlist.h>
#include <qstring.h>
#include <dcopobject.h>

class kdesvnd_dcop :  public DCOPObject
{
    K_DCOP

private:
    QStringList m_List;

public:
    kdesvnd_dcop();
    virtual ~kdesvnd_dcop();

k_dcop:
    bool exit();
    //! get a subversion login
    /*!
    * \param realm the realm
    * \return a stringlist containing username-password-saveit as "true" or "false" or empty list if cancel hit.
    */
    QStringList get_login(QString);

    // return: -1 dont accept 0 accept temporary 1 accept always
    //               hostname, fingerprint, validFrom, validUntil, issuerDName, realm,
    int get_sslaccept(QString, QString,     QString,   QString,    QString,     QString);

    // returns cert file or empty string
    QString get_sslclientcertfile();
    // return a logmessage at pos 0, null-size list if cancel hit
    QStringList get_logmsg();
};
#endif
