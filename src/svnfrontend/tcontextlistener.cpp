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
#include "tcontextlistener.h"

#include "ccontextlistener.h"
#include "authdialogimpl.h"
#include "logmsg_impl.h"
#include "ssltrustprompt_impl.h"

#include <kapplication.h>
#include <kdebug.h>

ThreadContextListener::ThreadContextListener(QObject* parent, const char* name)
    : CContextListener(parent, name)
{
}

ThreadContextListener::~ThreadContextListener()
{
}


bool ThreadContextListener::contextGetLogin(const QString& realm, QString& username, QString& password, bool& maySave)
{
    return false;
}

bool ThreadContextListener::contextGetLogMessage(QString& msg)
{
    return false;
}

bool ThreadContextListener::contextSslClientCertPrompt(QString& certFile)
{
    return false;
}

bool ThreadContextListener::contextSslClientCertPwPrompt(QString& password, const QString& realm, bool& maySave)
{
    return false;
}

svn::ContextListener::SslServerTrustAnswer ThreadContextListener::contextSslServerTrustPrompt(const SslServerTrustData& data, apr_uint32_t& acceptedFailures)
{
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_SSL_TRUST_PROMPT);
    void * t = (void*)&data;
    ev->setData(t);
    kdDebug()<<"Post event "<<EVENT_THREAD_SSL_TRUST_PROMPT<<" from thread " << endl;
    kapp->postEvent(this,ev);
    m_trustpromptWait.wait();
    return m_SslTrustAnswer;
}

void ThreadContextListener::event_contextGetLogin(const QString& realm, QString& username, QString& password)
{
    //return CContextListener::contextGetLogin(realm, username, password, maySave);
}

void ThreadContextListener::event_contextGetLogMessage(QString& msg)
{
    //return CContextListener::contextGetLogMessage(msg);
}

void ThreadContextListener::event_contextSslClientCertPrompt(QString& certFile)
{
    //return CContextListener::contextSslClientCertPrompt(certFile);
}

void ThreadContextListener::event_contextSslClientCertPwPrompt(QString& password, const QString& realm)
{
    //return CContextListener::contextSslClientCertPwPrompt(password, realm, maySave);
}

void ThreadContextListener::event_contextSslServerTrustPrompt(SslServerTrustData* data)
{
    //return CContextListener::contextSslServerTrustPrompt(data, acceptedFailures);
    /*
     * m_SslTrustAnswer is made threadsafe due the m_trustpromptWait - the calling thread waits until wakeAll is called! */
    bool ok,saveit;
    kdDebug()<<"Host: " << data->hostname << endl;
    if (!SslTrustPrompt_impl::sslTrust(
        data->hostname,
        data->fingerprint,
        data->validFrom,
        data->validUntil,
        data->issuerDName,
        data->realm,
        failure2Strings(data->failures),
        &ok,&saveit)) {
        m_SslTrustAnswer = DONT_ACCEPT;
    }
    if (!saveit) {
        m_SslTrustAnswer = ACCEPT_TEMPORARILY;
    }
    m_SslTrustAnswer = ACCEPT_PERMANENTLY;
    /* MUST reached otherwise deadlock */
    m_trustpromptWait.wakeAll();
}

void ThreadContextListener::customEvent(QCustomEvent*ev)
{
    kdDebug()<<"Got event " << ev->type()<<endl;
    if (ev->type()==EVENT_THREAD_SSL_TRUST_PROMPT) {
        event_contextSslServerTrustPrompt( (SslServerTrustData*)ev->data() );
    }
}

#include "tcontextlistener.moc"
