/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
//Added by qt3to4:
#include <Q3CString>
#include <kurl.h>
#include <dcopobject.h>
#include <kdedmodule.h>

class IListener;

class kdesvnd_dcop :  public KDEDModule
{
    Q_OBJECT
    K_DCOP

private:

public:
    kdesvnd_dcop(const Q3CString&);
    virtual ~kdesvnd_dcop();

protected:
    bool isWorkingCopy(const KURL&url,QString&base);
    bool isRepository(const KURL&url);
    static QString cleanUrl(const KURL&url);
    IListener*m_Listener;

k_dcop:
    //! get a subversion login
    /*!
    * \param realm the realm
    * \param user default username
    * \return a stringlist containing username-password-saveit as "true" or "false" or empty list if cancel hit.
    */
    QStringList get_login(QString,QString);

    // return: -1 dont accept 0 accept temporary 1 accept always
    //               hostname, fingerprint, validFrom, validUntil, issuerDName, realm,
    int get_sslaccept(QString, QString,     QString,   QString,    QString,     QString);

    // returns cert file or empty string
    QString get_sslclientcertfile();
    // return a logmessage at pos 0, null-size list if cancel hit
    QStringList get_logmsg();
    // return a logmessage at pos 0, null-size list if cancel hit, parameter is path -> action for display
    QStringList get_logmsg(QMap<QString,QString>);
    // return pw at pos 0, maysafe at pos 1, null-size if cancel hit.
    QStringList get_sslclientcertpw(QString);
    QStringList getActionMenu(KURL::List);
    QStringList getSingleActionMenu(Q3CString);
};
#endif
