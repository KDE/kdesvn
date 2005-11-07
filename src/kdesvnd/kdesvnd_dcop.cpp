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
#include "logmsg_impl.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>

extern "C" {
/* we will get trouble on old fedora 2 systems with the origin compiler - don't know whats up
 * with there gcc
 */
 #define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
#if GCC_VERSION == 30303
#undef KDE_EXPORT
#define KDE_EXPORT
#endif
    KDE_EXPORT KDEDModule *create_kdesvnd(const QCString &name) {
       return new kdesvnd_dcop(name);
    }
}

kdesvnd_dcop::kdesvnd_dcop(const QCString&name) : KDEDModule(name)
{
    kdDebug() << "Starting new service... " << endl;
}

kdesvnd_dcop::~kdesvnd_dcop()
{
    kdDebug() << "Going away... " << endl;
}

QStringList kdesvnd_dcop::getTopLevelActionMenu (const KURL::List list)
{
    QStringList result;
    return result;
}

QStringList kdesvnd_dcop::getActionMenu ( const KURL::List list )
{
    QStringList result;
    result << "Add";
    result << "Log";
    result << "Info";
    result << "Diff";
    return result;
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
        QStringList(),
        &ok,&saveit)) {
        return -1;
    }
    if (!saveit) {
        return 0;
    }
    return 1;
}

QString kdesvnd_dcop::get_sslclientcertfile()
{
    QString afile = KFileDialog::getOpenFileName(QString::null,
        QString::null,
        0,
        i18n("Open a file with a #PKCS12 certificate"));
    return afile;
}

QStringList kdesvnd_dcop::get_logmsg()
{
    QStringList res;
    bool ok;
    QString logMessage = Logmsg_impl::getLogmessage(&ok,0,0,"logmsg_impl");
    if (!ok) {
        return res;
    }
    res.append(logMessage);
    return res;
}
