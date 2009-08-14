/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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
