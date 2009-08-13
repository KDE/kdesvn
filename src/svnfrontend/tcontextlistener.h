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
#ifndef TCONTEXTLISTENER_H
#define TCONTEXTLISTENER_H

#include "eventnumbers.h"

#include "ccontextlistener.h"

#include <qevent.h>
#include <qmutex.h>
#include <qwaitcondition.h>

class ThreadContextListenerData;

/**
@author Rajko Albrecht
*/
class ThreadContextListener : public CContextListener
{
    Q_OBJECT
public:
    ThreadContextListener(QObject* parent, const char* name=0);

    virtual ~ThreadContextListener();

    virtual bool contextGetLogin(const QString& realm, QString& username, QString& password, bool& maySave);
    virtual bool contextGetSavedLogin(const QString & realm,QString & username,QString & password);

    virtual bool contextGetLogMessage(QString& msg,const svn::CommitItemList&);
    virtual bool contextSslClientCertPrompt(QString& certFile);
    virtual bool contextSslClientCertPwPrompt(QString& password, const QString& realm, bool& maySave);
    virtual svn::ContextListener::SslServerTrustAnswer contextSslServerTrustPrompt(const SslServerTrustData& data, apr_uint32_t& acceptedFailures);
    virtual void sendTick();
    virtual void contextProgress(long long int current, long long int max);

    virtual void contextNotify (const char *path,
                svn_wc_notify_action_t action,
                svn_node_kind_t kind,
                const char *mime_type,
                svn_wc_notify_state_t content_state,
                svn_wc_notify_state_t prop_state,
                svn_revnum_t revision);
    virtual void contextNotify (const svn_wc_notify_t *action){CContextListener::contextNotify(action);}
    virtual void contextNotify(const QString&);
    static QMutex*callbackMutex();

protected:
    virtual void event_contextGetLogin(void*_data);
    virtual void event_contextGetSavedLogin(void*_data);
    virtual void event_contextGetLogMessage(void*data);
    virtual void event_contextSslClientCertPrompt(void*data);
    virtual void event_contextSslClientCertPwPrompt(void*data);
    virtual void event_contextSslServerTrustPrompt(void* data);
    virtual void event_contextNotify(void*data);
    virtual void customEvent(QEvent*);

    /* stores all internals */
    QMutex m_WaitMutex;
    ThreadContextListenerData*m_Data;
};

#endif
