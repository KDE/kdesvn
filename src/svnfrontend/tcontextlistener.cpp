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
#include "tcontextlistener.h"

#include "ccontextlistener.h"
#include "src/ksvnwidgets/authdialogimpl.h"
#include "src/ksvnwidgets/logmsg_impl.h"
#include "src/ksvnwidgets/ssltrustprompt_impl.h"
#include "src/helpers/stringhelper.h"
#include "threadcontextlistenerdata.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

ThreadContextListener::ThreadContextListener(QObject* parent, const char* name)
    : CContextListener(parent, name)
{
    m_Data = new ThreadContextListenerData;
}

ThreadContextListener::~ThreadContextListener()
{
    delete m_Data;
}

bool ThreadContextListener::contextGetLogin(const QString& realm, QString& username, QString& password, bool& maySave)
{
    kdDebug()<<"Getting threaded login"<<endl;
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    ThreadContextListenerData::slogin_data _data;
    _data.realm=realm;
    _data.user=username;
    _data.password=password;
    _data.maysave=maySave;
    _data.ok=false;

    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_LOGIN_PROMPT);
    void*t = (void*)&_data;
    ev->setData(t);
    kdDebug()<<"Post event "<<EVENT_THREAD_LOGIN_PROMPT<<" (login) from thread " << endl;
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    username = _data.user;
    password = _data.password;
    maySave = _data.maysave;
    return _data.ok;
}

bool ThreadContextListener::contextGetSavedLogin(const QString & realm,QString & username,QString & password)
{

    kdDebug()<<"Getting threaded saved login"<<endl;
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    ThreadContextListenerData::slogin_data _data;
    _data.realm=realm;
    _data.user=username;
    _data.password=password;
    _data.maysave=false;
    _data.ok=false;

    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_LOGIN_SAVED);
    void*t = (void*)&_data;
    ev->setData(t);
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    username = _data.user;
    password = _data.password;
    return _data.ok;
}

bool ThreadContextListener::contextGetLogMessage(QString& msg,const svn::CommitItemList&_items)
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    ThreadContextListenerData::slog_message log;
    log.ok = false;
    log.msg = "";
    log._items = &_items;
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_LOGMSG_PROMPT);
    void*t = (void*)&log;
    ev->setData(t);
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    msg = log.msg;
    return log.ok;
}

bool ThreadContextListener::contextSslClientCertPrompt(QString& certFile)
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    ThreadContextListenerData::scert_file scertf;
    scertf.ok = false;
    scertf.certfile="";
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_CERT_SELECT_PROMPT);
    ev->setData((void*)&scertf);
    kdDebug()<<"Post event "<<EVENT_THREAD_CERT_SELECT_PROMPT<<" from thread " << endl;
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    certFile = scertf.certfile;
    return scertf.ok;
}

bool ThreadContextListener::contextSslClientCertPwPrompt(QString& password, const QString& realm, bool& maySave)
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    ThreadContextListenerData::scert_pw scert_data;
    scert_data.ok=false;
    scert_data.maysave=false;
    scert_data.password="";
    scert_data.realm=realm;
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_CERT_PW_PROMPT);
    ev->setData((void*)&scert_data);
    kdDebug()<<"Post event "<<EVENT_THREAD_CERT_PW_PROMPT<<" from thread " << endl;
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    password = scert_data.password;
    maySave = scert_data.maysave;
    return scert_data.ok;
}

svn::ContextListener::SslServerTrustAnswer ThreadContextListener::contextSslServerTrustPrompt(const SslServerTrustData& data, apr_uint32_t&/* acceptedFailures*/)
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_SSL_TRUST_PROMPT);
    ThreadContextListenerData::strust_answer trust_answer;
    trust_answer.m_SslTrustAnswer=DONT_ACCEPT;
    trust_answer.m_Trustdata = &data;
    ev->setData((void*)&trust_answer);
    kdDebug()<<"Post event "<<EVENT_THREAD_SSL_TRUST_PROMPT<<" from thread " << endl;
    kapp->postEvent(this,ev);
    m_Data->m_trustpromptWait.wait();
    return trust_answer.m_SslTrustAnswer;
}

void ThreadContextListener::contextNotify(const QString&aMsg)
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_NOTIFY);
    // receiver must delete data!
    ThreadContextListenerData::snotify* _notify = new ThreadContextListenerData::snotify();
    _notify->msg = aMsg;
    ev->setData((void*)_notify);
    kapp->postEvent(this,ev);
}

/*!
    \fn ThreadContextListener::contextProgress(long long int current, long long int max)
 */
void ThreadContextListener::contextProgress(long long int current, long long int max)
{
    if (m_Data->noProgress||current==0) {
        return;
    }
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_NOTIFY);
    // receiver must delete data!
    ThreadContextListenerData::snotify* _notify = new ThreadContextListenerData::snotify();
    QString msg;
    QString s1 = helpers::ByteToString()(current);
    if (max>-1) {
        QString s2 = helpers::ByteToString()(max);
        msg = i18n("%1 of %2 transferred.").arg(s1).arg(s2);
    } else {
        msg = i18n("%1 transferred.").arg(s1);
    }
    _notify->msg = msg;
    ev->setData((void*)_notify);
    kapp->postEvent(this,ev);
}

void ThreadContextListener::sendTick()
{
    QMutexLocker lock(&(m_Data->m_CallbackMutex));
    QCustomEvent*ev = new QCustomEvent(EVENT_THREAD_NOTIFY);
    // receiver must delete data!
    ThreadContextListenerData::snotify* _notify = new ThreadContextListenerData::snotify();
    _notify->msg = "";
    ev->setData((void*)_notify);
    kapp->postEvent(this,ev);
}

/* methods below may only called from mainthread! (via event) */
void ThreadContextListener::event_contextGetLogin(void*data)
{
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::slogin_data*_data = (ThreadContextListenerData::slogin_data*)data;

    _data->ok = CContextListener::contextGetLogin(_data->realm, _data->user, _data->password, _data->maysave);
    //_data->ok = CContextListener::contextGetSavedLogin(_data->realm, _data->user, _data->password);
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextGetSavedLogin(void*data)
{
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::slogin_data*_data = (ThreadContextListenerData::slogin_data*)data;
    _data->ok = CContextListener::contextGetSavedLogin(_data->realm, _data->user, _data->password);
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextGetLogMessage(void * data)
{
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::slog_message * _log = (ThreadContextListenerData::slog_message*)data;

    _log->ok = CContextListener::contextGetLogMessage(_log->msg,(_log->_items?*(_log->_items):svn::CommitItemList()));
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextSslClientCertPrompt(void*data)
{
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::scert_file*scertf = (ThreadContextListenerData::scert_file*)data;
    scertf->ok = CContextListener::contextSslClientCertPrompt(scertf->certfile);
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextSslClientCertPwPrompt(void*data)
{
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::scert_pw*scert_data = (ThreadContextListenerData::scert_pw*)data;
    scert_data->ok = CContextListener::contextSslClientCertPwPrompt(scert_data->password, scert_data->realm, scert_data->maysave);
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextSslServerTrustPrompt(void*data)
{
    /*
     * m_SslTrustAnswer is made threadsafe due the m_trustpromptWait - the calling thread waits until wakeAll is called!
     */
    if (!data) {
        m_Data->m_trustpromptWait.wakeAll();
        return;
    }
    ThreadContextListenerData::strust_answer*_data = (ThreadContextListenerData::strust_answer*)data;
    apr_uint32_t _t = _data->m_Trustdata->failures;
    _data->m_SslTrustAnswer =  CContextListener::contextSslServerTrustPrompt(*(_data->m_Trustdata),_t);
    m_Data->m_trustpromptWait.wakeAll();
}

void ThreadContextListener::event_contextNotify(void*data)
{
    if (!data) {
        return;
    }
    ThreadContextListenerData::snotify* _notify = (ThreadContextListenerData::snotify*)data;
    CContextListener::contextNotify(_notify->msg);
    delete _notify;
}

void ThreadContextListener::customEvent(QCustomEvent*ev)
{
    if (ev->type()==EVENT_THREAD_SSL_TRUST_PROMPT) {
        event_contextSslServerTrustPrompt(ev->data());
    }else if (ev->type()==EVENT_THREAD_LOGIN_PROMPT) {
        event_contextGetLogin(ev->data());
    }else if (ev->type()==EVENT_THREAD_LOGMSG_PROMPT) {
        event_contextGetLogMessage(ev->data());
    }else if (ev->type()==EVENT_THREAD_CERT_PW_PROMPT) {
        event_contextSslClientCertPwPrompt(ev->data());
    }else if (ev->type()==EVENT_THREAD_CERT_SELECT_PROMPT) {
        event_contextSslClientCertPrompt(ev->data());
    }else if (ev->type()==EVENT_THREAD_NOTIFY) {
        event_contextNotify(ev->data());
    } else if (ev->type() == EVENT_THREAD_LOGIN_SAVED) {
        event_contextGetSavedLogin(ev->data());
    }
}

#include "tcontextlistener.moc"
