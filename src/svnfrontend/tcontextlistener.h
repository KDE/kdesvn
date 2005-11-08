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
#ifndef TCONTEXTLISTENER_H
#define TCONTEXTLISTENER_H

#include "eventnumbers.h"

#include "ccontextlistener.h"

#include <qevent.h>
#include <qmutex.h>
#include <qwaitcondition.h>

/**
@author Rajko Albrecht
*/
class ThreadContextListener : public CContextListener
{
    Q_OBJECT
public:
    ThreadContextListener(QObject* parent, const char* name);

    ~ThreadContextListener();

    virtual bool contextGetLogin(const QString& realm, QString& username, QString& password, bool& maySave);
    virtual bool contextGetLogMessage(QString& msg);
    virtual bool contextSslClientCertPrompt(QString& certFile);
    virtual bool contextSslClientCertPwPrompt(QString& password, const QString& realm, bool& maySave);
    virtual svn::ContextListener::SslServerTrustAnswer contextSslServerTrustPrompt(const SslServerTrustData& data, apr_uint32_t& acceptedFailures);

    virtual void event_contextGetLogin(const QString& realm, QString& username, QString& password);
    virtual void event_contextGetLogMessage(QString& msg);
    virtual void event_contextSslClientCertPrompt(QString& certFile);
    virtual void event_contextSslClientCertPwPrompt(QString& password, const QString& realm);
    virtual void event_contextSslServerTrustPrompt(SslServerTrustData* data);

protected:
    virtual void customEvent(QCustomEvent*);
    QMutex m_trustpromptMutex;
    QWaitCondition m_trustpromptWait;
    /* safed due condition above */
    svn::ContextListener::SslServerTrustAnswer m_SslTrustAnswer;
};


#endif
