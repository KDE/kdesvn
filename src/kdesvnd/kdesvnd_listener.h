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
#ifndef KDESVND_LISTENER_H
#define KDESVND_LISTENER_H

#include "svnqt/context_listener.h"

class kdesvnd;

class KdesvndListener: public svn::ContextListener
{
    friend class kdesvnd;

    kdesvnd *m_back;
public:
    explicit KdesvndListener(kdesvnd *p);
    virtual ~KdesvndListener();
    /* context-listener methods */
    bool contextGetLogin(const QString &realm,
                                 QString &username,
                                 QString &password,
                                 bool &maySave) override;
    bool contextGetSavedLogin(const QString &realm, QString &username, QString &password) override;
    bool contextGetCachedLogin(const QString &realm, QString &username, QString &password) override;

    void contextNotify(const char *path,
                               svn_wc_notify_action_t action,
                               svn_node_kind_t kind,
                               const char *mime_type,
                               svn_wc_notify_state_t content_state,
                               svn_wc_notify_state_t prop_state,
                               svn_revnum_t revision) override;
    void contextNotify(const svn_wc_notify_t *action) override;

    bool contextCancel() override;
    bool contextGetLogMessage(QString &msg, const svn::CommitItemList &) override;
    virtual svn::ContextListener::SslServerTrustAnswer
    contextSslServerTrustPrompt(const SslServerTrustData &data,
                                apr_uint32_t &acceptedFailures) override;
    bool contextSslClientCertPrompt(QString &certFile) override;
    bool contextLoadSslClientCertPw(QString &password, const QString &realm) override;
    bool contextSslClientCertPwPrompt(QString &password,
            const QString &realm, bool &maySave) override;
    void contextProgress(long long int current, long long int max) override;

    /* context listener virtuals end */

protected:
    svn::ContextP m_CurrentContext;
    svn::ClientP m_Svnclient;
};

#endif
