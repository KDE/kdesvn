/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
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
/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 */

#ifndef SSHAGENT_H
#define SSHAGENT_H

#include <QObject>
#include <QProcess>
#include <QString>

class KProcess;

class SshAgent : public QObject
{
    Q_OBJECT

public:
    explicit SshAgent(QObject *parent = nullptr);
    ~SshAgent();

    bool querySshAgent();
    bool addSshIdentities(bool force = false);
    void killSshAgent();

    bool isRunning() const
    {
        return m_isRunning;
    }
    QString pid() const
    {
        return m_pid;
    }
    QString authSock() const
    {
        return m_authSock;
    }

private slots:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout();

private:
    bool startSshAgent();
    void askPassEnv();

    QString        m_Output;

    static bool    m_isRunning;
    static bool    m_isOurAgent;
    static bool    m_addIdentitiesDone;
    static QString m_authSock;
    static QString m_pid;

    KProcess *sshAgent;
};

#endif
