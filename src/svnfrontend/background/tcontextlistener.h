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

#include "svnfrontend/ccontextlistener.h"

struct ThreadContextListenerData;

/**
  @author Rajko Albrecht
  Same as CContextListener but make sure the user actions are executed in the main thread
  Therefore all events have to be passed through the Qt signal/slot system to make sure
  a context switch to the main thread occurs.
  This is done by a signal 'signalFoo()' which is executed and caught inside this class.
  The slot is then passed to CContextListener::foo()

  This Listener *must* be instanciated in the main thread!
*/
class ThreadContextListener : public CContextListener
{
    Q_OBJECT
public:
    explicit ThreadContextListener(QObject *parent);

    ~ThreadContextListener();

    // called from a thread != main thread
    bool contextGetLogin(const QString &realm, QString &username, QString &password, bool &maySave) override;
    bool contextGetSavedLogin(const QString &realm, QString &username, QString &password) override;
    bool contextGetLogMessage(QString &msg, const svn::CommitItemList &) override;
    bool contextSslClientCertPrompt(QString &certFile) override;
    bool contextSslClientCertPwPrompt(QString &password, const QString &realm, bool &maySave) override;
    svn::ContextListener::SslServerTrustAnswer contextSslServerTrustPrompt(const SslServerTrustData &data, apr_uint32_t &acceptedFailures) override;
    void sendTick() override;
    void contextProgress(long long int current, long long int max) override;

    using CContextListener::contextNotify;
    void contextNotify(const QString &) override;

Q_SIGNALS:
    void signal_contextGetLogin();
    void signal_contextGetSavedLogin();
    void signal_contextGetLogMessage();
    void signal_contextSslClientCertPrompt();
    void signal_contextSslClientCertPwPrompt();
    void signal_contextSslServerTrustPrompt();
    void signal_contextNotify(const QString &msg);
protected Q_SLOTS:  // executed in main thread
    virtual void event_contextGetLogin();
    virtual void event_contextGetSavedLogin();
    virtual void event_contextGetLogMessage();
    virtual void event_contextSslClientCertPrompt();
    virtual void event_contextSslClientCertPwPrompt();
    virtual void event_contextSslServerTrustPrompt();
    virtual void event_contextNotify(const QString &msg);

private:
    /* stores all internals */
    ThreadContextListenerData *m_Data;
};

#endif
