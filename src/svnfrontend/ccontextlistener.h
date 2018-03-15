/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#ifndef CCONTEXTLISTENER_H
#define CCONTEXTLISTENER_H

#include "svnqt/context_listener.h"

#include <QObject>
#include <QString>

class CContextListenerData;

/**
@author Rajko Albrecht
*/
class CContextListener : public QObject, public svn::ContextListener
{
    Q_OBJECT
public:
    explicit CContextListener(QObject *parent = nullptr);
    ~CContextListener();

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
    virtual void contextNotify(const QString &);

    virtual void sendTick();

    bool contextCancel() override;
    /*!
     * Get logmessage for checkin and so on...
     */
    bool contextGetLogMessage(QString &msg, const svn::CommitItemList &) override;
    SslServerTrustAnswer contextSslServerTrustPrompt(const SslServerTrustData &data,
            apr_uint32_t &acceptedFailures) override;
    bool contextSslClientCertPrompt(QString &certFile) override;
    bool contextSslClientCertPwPrompt(QString &password,
            const QString &realm, bool &maySave) override;
    bool contextLoadSslClientCertPw(QString &password, const QString &realm) override;
    QString translate(const QString &what) override;

    static QString NotifyAction(svn_wc_notify_action_t action);
    static QString NotifyState(svn_wc_notify_state_t);

    static QStringList failure2Strings(apr_uint32_t acceptedFailures);
    void contextProgress(long long int current, long long int max) override;

    void maySavePlaintext(svn_boolean_t *may_save_plaintext, const QString &realmstring) override;

    // used by SvnActions
    //! get list of updated items when svn update is called
    const QStringList &updatedItems()const;
    //! cleans list of updatedItems, should called before calling svn::Client::update
    void cleanUpdatedItems();

public Q_SLOTS:
    virtual void setCanceled(bool);

Q_SIGNALS:
    void sendNotify(const QString &);
    void tickProgress();
    void waitShow(bool);
    void netProgress(long long int, long long int);

protected:
    static const int smax_actionstring;
    static const char *action_strings[];
    static const char *notify_state_strings[];
    CContextListenerData *m_Data;

private:
    void extraNotify(const QString &path, svn_wc_notify_action_t action, svn_revnum_t revision);
};

#endif
