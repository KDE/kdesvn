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
#include "kiolistener.h"
#include "kiosvn.h"

#include <kdebug.h>

#include <dcopclient.h>

KioListener::KioListener(kio_svnProtocol*_par)
 : svn::ContextListener()
{
    par = _par;
}


KioListener::~KioListener()
{
}




/*!
    \fn KioListener::contextCancel()
 */
bool KioListener::contextCancel()
{
    return par->wasKilled();
}


/*!
    \fn KioListener::contextGetLogMessage (QString & msg)
 */
bool KioListener::contextGetLogMessage (QString & msg)
{
    msg = "";
    return true;
}

void KioListener::contextNotify (const char * /* path */,
                    svn_wc_notify_action_t /* action */,
                    svn_node_kind_t /* kind */,
                    const char * /* mime_type */,
                    svn_wc_notify_state_t /* content_state */,
                    svn_wc_notify_state_t /* prop_state */,
                    svn_revnum_t /* revision */)
{
}

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
void KioListener::contextNotify (const svn_wc_notify_t *action)
{
    if (!action) return;
//    if (action->action<svn_wc_notify_locked) {
        contextNotify(action->path,action->action,action->kind,action->mime_type,
            action->content_state,action->prop_state,action->revision);
//        return;
//    }
//    QString aString = NotifyAction(action->action);
}
#endif

svn::ContextListener::SslServerTrustAnswer
KioListener::contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;
    QDataStream stream(params,IO_WriteOnly);
    stream << data.hostname
        << data.fingerprint
        << data.validFrom
        << data.validUntil
        << data.issuerDName
        << data.realm;

    if (!par->dcopClient()->call("kdesvnd","kdesvndInterface",
        "get_sslaccept(QString,QString,QString,QString,QString,QString)",
        params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return DONT_ACCEPT;
    }
    if (replyType!="int") {
        kdWarning()<<"Wrong reply type"<<endl;
        return DONT_ACCEPT;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    int res;
    stream2>>res;
    switch (res) {
        case -1:
            return DONT_ACCEPT;
            break;
        case 1:
            return ACCEPT_PERMANENTLY;
            break;
        default:
        case 0:
            return ACCEPT_TEMPORARILY;
            break;
    }
    /* avoid compiler warnings */
    return ACCEPT_TEMPORARILY;
}

bool KioListener::contextSslClientCertPrompt (QString & certFile)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;
    if (!par->dcopClient()->call("kdesvnd","kdesvndInterface",
        "get_sslclientcertfile()",
        params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return false;
    }
    if (replyType!="QString") {
        kdWarning()<<"Wrong reply type"<<endl;
        return false;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    stream2>>certFile;
    if (certFile.isEmpty()) {
        return false;
    }
    return true;
}

bool KioListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave)
{
    return false;
}

bool KioListener::contextGetLogin (const QString & realm,
                     QString & username,
                     QString & password,
                     bool & maySave)
{
    QByteArray reply;
    QByteArray params;
    QCString replyType;

    QDataStream stream(params,IO_WriteOnly);
    stream << realm;

    if (!par->dcopClient()->call("kdesvnd","kdesvndInterface","get_login(QString)",params,replyType,reply)) {
        kdWarning()<<"Communication with dcop failed"<<endl;
        return false;
    }
    if (replyType!="QStringList") {
        kdWarning()<<"Wrong reply type"<<endl;
        return false;
    }
    QDataStream stream2(reply,IO_ReadOnly);
    QStringList lt;
    stream2>>lt;
    if (lt.count()!=3) {
        kdDebug()<<"Wrong or missing auth list (may cancel pressed)." << endl;
        return false;
    }
    username = lt[0];
    password = lt[1];
    maySave = lt[2]=="true";
    return true;
}
