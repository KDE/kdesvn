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
#include "tcontextlistener.h"

#include "svnfrontend/ccontextlistener.h"
#include "ksvnwidgets/commitmsg_impl.h"
#include "helpers/stringhelper.h"
#include "threadcontextlistenerdata.h"

#include <KLocalizedString>

// ThreadContextListenerData
QMutex *ThreadContextListenerData::callbackMutex()
{
    static QMutex s_CallbackMutex;
    return &s_CallbackMutex;
}

// ThreadContextListener
ThreadContextListener::ThreadContextListener(QObject *parent)
    : CContextListener(parent)
    , m_Data(new ThreadContextListenerData)
{
    connect(this, &ThreadContextListener::signal_contextGetLogin,
            this, &ThreadContextListener::event_contextGetLogin,
            Qt::BlockingQueuedConnection);
    connect(this, &ThreadContextListener::signal_contextGetSavedLogin,
            this, &ThreadContextListener::event_contextGetSavedLogin,
            Qt::BlockingQueuedConnection);
    connect(this, &ThreadContextListener::signal_contextGetLogMessage,
            this, &ThreadContextListener::event_contextGetLogMessage,
            Qt::BlockingQueuedConnection);
    connect(this, &ThreadContextListener::signal_contextSslClientCertPrompt,
            this, &ThreadContextListener::event_contextSslClientCertPrompt,
            Qt::BlockingQueuedConnection);
    connect(this, &ThreadContextListener::signal_contextSslClientCertPwPrompt,
            this, &ThreadContextListener::event_contextSslClientCertPwPrompt,
            Qt::BlockingQueuedConnection);
    connect(this, &ThreadContextListener::signal_contextSslServerTrustPrompt,
            this, &ThreadContextListener::event_contextSslServerTrustPrompt,
            Qt::BlockingQueuedConnection);
    // no user input, BlockingQueuedConnection not needed here
    connect(this, &ThreadContextListener::signal_contextNotify,
            this, &ThreadContextListener::event_contextNotify);
}

ThreadContextListener::~ThreadContextListener()
{
    delete m_Data;
}

bool ThreadContextListener::contextGetLogin(const QString &realm, QString &username, QString &password, bool &maySave)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_slogin_data.realm = realm;
    m_Data->m_slogin_data.user = username;
    m_Data->m_slogin_data.password = password;
    m_Data->m_slogin_data.maysave = maySave;
    m_Data->bReturnValue = false;

    // call event_contextGetLogin() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextGetLogin();

    username = m_Data->m_slogin_data.user;
    password = m_Data->m_slogin_data.password;
    maySave = m_Data->m_slogin_data.maysave;
    return m_Data->bReturnValue;
}

bool ThreadContextListener::contextGetSavedLogin(const QString &realm, QString &username, QString &password)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_slogin_data.realm = realm;
    m_Data->m_slogin_data.user = username;
    m_Data->m_slogin_data.password = password;
    m_Data->m_slogin_data.maysave = false;
    m_Data->bReturnValue = false;

    // call event_contextGetSavedLogin() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextGetSavedLogin();

    username = m_Data->m_slogin_data.user;
    password = m_Data->m_slogin_data.password;
    return m_Data->bReturnValue;
}

bool ThreadContextListener::contextGetLogMessage(QString &msg, const svn::CommitItemList &_items)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_slog_message.items = _items;
    m_Data->bReturnValue = false;

    // call event_contextGetLogMessage() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextGetLogMessage();

    msg = m_Data->m_slog_message.msg;
    return m_Data->bReturnValue;
}

bool ThreadContextListener::contextSslClientCertPrompt(QString &certFile)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_scert_file.certfile.clear();
    m_Data->bReturnValue = false;

    // call event_contextSslClientCertPrompt() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextSslClientCertPrompt();

    certFile = m_Data->m_scert_file.certfile;
    return m_Data->bReturnValue;
}

bool ThreadContextListener::contextSslClientCertPwPrompt(QString &password, const QString &realm, bool &maySave)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_scert_pw.maysave = false;
    m_Data->m_scert_pw.realm = realm;
    m_Data->bReturnValue = false;

    // call event_contextSslClientCertPrompt() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextSslClientCertPwPrompt();

    password = m_Data->m_scert_pw.password;
    maySave = m_Data->m_scert_pw.maysave;
    return m_Data->bReturnValue;
}

svn::ContextListener::SslServerTrustAnswer ThreadContextListener::contextSslServerTrustPrompt(const SslServerTrustData &data, apr_uint32_t &/* acceptedFailures*/)
{
    QMutexLocker lock(ThreadContextListenerData::callbackMutex());

    m_Data->m_strust_answer.sslTrustAnswer = DONT_ACCEPT;
    m_Data->m_strust_answer.trustdata = data;
    m_Data->bReturnValue = false;

    // call event_contextSslClientCertPrompt() in main thread, wait until finished due to BlockingQueuedConnection
    emit signal_contextSslServerTrustPrompt();

    return m_Data->m_strust_answer.sslTrustAnswer;
}

void ThreadContextListener::contextNotify(const QString &aMsg)
{
    // call event_contextNotify() in main thread
    emit signal_contextNotify(aMsg);
}

/*!
    \fn ThreadContextListener::contextProgress(long long int current, long long int max)
 */
void ThreadContextListener::contextProgress(long long int current, long long int max)
{
    if (m_Data->noProgress || current == 0) {
        return;
    }
    QString msg;
    QString s1 = helpers::ByteToString(current);
    if (max > -1) {
        QString s2 = helpers::ByteToString(max);
        msg = i18n("%1 of %2 transferred.", s1, s2);
    } else {
        msg = i18n("%1 transferred.", s1);
    }
    emit signal_contextNotify(msg);
}

void ThreadContextListener::sendTick()
{
    emit signal_contextNotify(QString());
}

/* methods below may only called from mainthread! (via signal/slot) */
void ThreadContextListener::event_contextGetLogin()
{
    m_Data->bReturnValue = CContextListener::contextGetLogin(m_Data->m_slogin_data.realm,
                                                             m_Data->m_slogin_data.user,
                                                             m_Data->m_slogin_data.password,
                                                             m_Data->m_slogin_data.maysave);
}

void ThreadContextListener::event_contextGetSavedLogin()
{
    m_Data->bReturnValue = CContextListener::contextGetSavedLogin(m_Data->m_slogin_data.realm,
                                                                  m_Data->m_slogin_data.user,
                                                                  m_Data->m_slogin_data.password);
}

void ThreadContextListener::event_contextGetLogMessage()
{
    m_Data->bReturnValue = CContextListener::contextGetLogMessage(m_Data->m_slog_message.msg,
                                                                  m_Data->m_slog_message.items);
}

void ThreadContextListener::event_contextSslClientCertPrompt()
{
    m_Data->bReturnValue = CContextListener::contextSslClientCertPrompt(m_Data->m_scert_file.certfile);
}

void ThreadContextListener::event_contextSslClientCertPwPrompt()
{
    m_Data->bReturnValue = CContextListener::contextSslClientCertPwPrompt(m_Data->m_scert_pw.password,
                                                                          m_Data->m_scert_pw.realm,
                                                                          m_Data->m_scert_pw.maysave);
}

void ThreadContextListener::event_contextSslServerTrustPrompt()
{
    m_Data->m_strust_answer.sslTrustAnswer = CContextListener::contextSslServerTrustPrompt(m_Data->m_strust_answer.trustdata,
                                                                                           m_Data->m_strust_answer.trustdata.failures);
}

void ThreadContextListener::event_contextNotify(const QString &msg)
{
    CContextListener::contextNotify(msg);
}
