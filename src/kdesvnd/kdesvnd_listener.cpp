/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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

#include "kdesvnd_listener.h"
#include "kdesvnd.h"

#include "settings/kdesvnsettings.h"
#include "ksvnwidgets/pwstorage.h"

KdesvndListener::KdesvndListener(kdesvnd *p)
    : svn::ContextListener()
    , m_back(p)
    , m_CurrentContext(new svn::Context)
    , m_Svnclient(svn::Client::getobject(m_CurrentContext))
{
    m_CurrentContext->setListener(this);
}

KdesvndListener::~KdesvndListener()
{
}

bool KdesvndListener::contextGetSavedLogin(const QString &realm, QString &username, QString &password)
{
    PwStorage::self()->getLogin(realm, username, password);
    return true;
}

bool KdesvndListener::contextGetCachedLogin(const QString &realm, QString &username, QString &password)
{
    PwStorage::self()->getCachedLogin(realm, username, password);
    return true;
}

bool KdesvndListener::contextGetLogin(const QString &realm,
                                      QString &username,
                                      QString &password,
                                      bool &maySave)
{
    maySave = false;
    QStringList res = m_back->get_login(realm, username);
    if (res.count() != 3) {
        return false;
    }
    username = res[0];
    password = res[1];
    maySave = (res[2] == QLatin1String("true"));
    if (maySave && Kdesvnsettings::passwords_in_wallet()) {
        PwStorage::self()->setLogin(realm, username, password);
        maySave = false;
    }
    return true;
}

void KdesvndListener::contextNotify(const char * /*path*/,
                                    svn_wc_notify_action_t /*action*/,
                                    svn_node_kind_t /*kind*/,
                                    const char */*mime_type*/,
                                    svn_wc_notify_state_t /*content_state*/,
                                    svn_wc_notify_state_t /*prop_state*/,
                                    svn_revnum_t /*revision*/)
{
}

void KdesvndListener::contextNotify(const svn_wc_notify_t * /*action*/)
{
}

bool KdesvndListener::contextCancel()
{
    return false;
}

bool KdesvndListener::contextGetLogMessage(QString &msg, const svn::CommitItemList &)
{
    const QStringList res = m_back->get_logmsg();
    if (res.isEmpty()) {
        return false;
    }
    msg = res[1];
    return true;
}

svn::ContextListener::SslServerTrustAnswer KdesvndListener::contextSslServerTrustPrompt(const SslServerTrustData &data,
        apr_uint32_t & /*acceptedFailures*/)
{
    int res = m_back->get_sslaccept(data.hostname,
                                    data.fingerprint,
                                    data.validFrom,
                                    data.validUntil,
                                    data.issuerDName,
                                    data.realm);
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

bool KdesvndListener::contextSslClientCertPrompt(QString &certFile)
{
    certFile = m_back->get_sslclientcertfile();
    if (certFile.isEmpty()) {
        return false;
    }
    return true;
}

bool KdesvndListener::contextLoadSslClientCertPw(QString &password, const QString &realm)
{
    return PwStorage::self()->getCertPw(realm, password);
}

bool KdesvndListener::contextSslClientCertPwPrompt(QString &password,
        const QString &realm, bool &maySave)
{
    maySave = false;
    if (PwStorage::self()->getCertPw(realm, password)) {
        return true;
    }
    QStringList res = m_back->get_sslclientcertpw(realm);
    if (res.size() != 2) {
        return false;
    }
    password = res[0];
    maySave = res[1] == QLatin1String("true");

    if (maySave && Kdesvnsettings::passwords_in_wallet()) {
        PwStorage::self()->setCertPw(realm, password);
        maySave = false;
    }

    return true;
}

void KdesvndListener::contextProgress(long long int, long long int)
{
}
