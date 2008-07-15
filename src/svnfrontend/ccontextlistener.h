/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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

#include "src/svnqt/context_listener.hpp"
#include "src/svnqt/smart_pointer.hpp"

#include <qobject.h>
#include <qstring.h>

class CContextListenerData;

/**
@author Rajko Albrecht
*/
class CContextListener : public QObject, public svn::ContextListener,public svn::ref_count
{
    Q_OBJECT
public:
    CContextListener(QObject *parent = 0, const char *name = 0);
    virtual ~CContextListener();

    /* context-listener methods */
    virtual bool contextGetLogin (const QString & realm,
                     QString & username,
                     QString & password,
                     bool & maySave);
    virtual bool contextGetSavedLogin (const QString & realm,QString & username,QString & password);

    virtual void contextNotify (const char *path,
                   svn_wc_notify_action_t action,
                   svn_node_kind_t kind,
                   const char *mime_type,
                   svn_wc_notify_state_t content_state,
                   svn_wc_notify_state_t prop_state,
                   svn_revnum_t revision);
    virtual void contextNotify(const QString&aMsg);
    virtual void sendTick();
    virtual void contextNotify (const svn_wc_notify_t *action);

    virtual bool contextCancel();
    /*!
     * Get logmessage for checkin and so on...
     */
    virtual bool contextGetLogMessage (QString & msg,const svn::CommitItemList&);
    virtual SslServerTrustAnswer contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures);
    virtual bool contextSslClientCertPrompt (QString & certFile);
    virtual bool contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave);
    virtual bool contextLoadSslClientCertPw(QString&password,const QString&realm);
    virtual QString translate(const QString&what);

    static QString NotifyAction(svn_wc_notify_action_t action);
    static QString NotifyState(svn_wc_notify_state_t);

    static QStringList failure2Strings(apr_uint32_t acceptedFailures);
    virtual void contextProgress(long long int current, long long int max);

public slots:
    virtual void setCanceled(bool);

signals:
    void sendNotify(const QString&);
    void tickProgress();
    void waitShow(bool);
    void netProgress(long long int, long long int);

protected:
    static const int smax_actionstring;
    static const QString action_strings[];
    static const QString notify_state_strings[];
    CContextListenerData*m_Data;
};

#endif
