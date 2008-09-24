/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef SSHAGENT_H
#define SSHAGENT_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qprocess.h>

class KProcess;

class SshAgent : public QObject
{
    Q_OBJECT

public:
    SshAgent(QObject* parent = 0);
    ~SshAgent();

    bool querySshAgent();
    bool addSshIdentities(bool force=false);
    void killSshAgent();

    bool isRunning() const { return m_isRunning; }
    QString pid() const { return m_pid; }
    QString authSock() const { return m_authSock; }

private slots:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout();

private:
    bool startSshAgent();

    QString        m_Output;

    static bool    m_isRunning;
    static bool    m_isOurAgent;
    static bool    m_addIdentitiesDone;
    static QString m_authSock;
    static QString m_pid;

    KProcess*sshAgent;
};


#endif
