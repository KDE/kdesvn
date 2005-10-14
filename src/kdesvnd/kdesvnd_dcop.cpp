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


#include "kdesvnd_dcop.h"
#include "authdialogimpl.h"
#include "ssltrustprompt_impl.h"

#include <kdebug.h>
#include <kapplication.h>

kdesvnd_dcop::kdesvnd_dcop() : DCOPObject("kdesvndInterface")
{
    kdDebug() << "Starting new service... " << endl;
    m_List = QStringList();
}

kdesvnd_dcop::~kdesvnd_dcop()
{
    kdDebug() << "Going away... " << endl;
}

QString kdesvnd_dcop::string(int idx)
{
    return *m_List.at(idx);
}

QStringList kdesvnd_dcop::list()
{
    return m_List;
}

void kdesvnd_dcop::add(QString arg)
{
    kdDebug() << "Adding " << arg << " to the list" << endl;
    m_List << arg;
}

bool kdesvnd_dcop::remove(QString arg)
{
    QStringList::Iterator it = m_List.find(arg);
    if (it != m_List.end())
    {
        m_List.remove(it);
    }
    else
        return false;
    return true;
}

bool kdesvnd_dcop::exit()
{
    kapp->quit();
    return true;
}

QStringList kdesvnd_dcop::get_login(QString realm)
{
    AuthDialogImpl auth(realm);
    QStringList res;
    if (auth.exec()==QDialog::Accepted) {
        res.append(auth.Username());
        res.append(auth.Password());
        if (auth.maySave()) {
            res.append("true");
        } else {
            res.append("false");
        }
    }
    return res;
}

int kdesvnd_dcop::get_sslaccept(QString hostname,QString fingerprint,QString validFrom,QString validUntil,QString issuerDName,QString realm)
{
    bool ok,saveit;
    if (!SslTrustPrompt_impl::sslTrust(
        hostname,
        fingerprint,
        validFrom,
        validUntil,
        issuerDName,
        realm,
        &ok,&saveit)) {
        return -1;
    }
    if (!saveit) {
        return 0;
    }
    return 1;
}
