/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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

#include "sshagent.h"
#include "kdesvn-config.h"

#include <KProcess>
#include <QCoreApplication>
#include <QRegExp>
#include <QStandardPaths>
#include "kdesvn_debug.h"

// initialize static member variables
bool    SshAgent::m_isRunning  = false;
bool    SshAgent::m_isOurAgent = false;
bool    SshAgent::m_addIdentitiesDone = false;
QString SshAgent::m_authSock;
QString SshAgent::m_pid;

class SshClean
{
public:
    SshClean() {}

    ~SshClean()
    {
        SshAgent ssh; ssh.killSshAgent();
    }
};

SshAgent::SshAgent(QObject *parent)
    : QObject(parent), sshAgent(nullptr)
{
    static SshClean st;
}

SshAgent::~SshAgent()
{
}

bool SshAgent::querySshAgent()
{
    if (m_isRunning) {
        return true;
    }

    // Did the user already start a ssh-agent process?
    const QByteArray pid = qgetenv("SSH_AGENT_PID");
    if (!pid.isEmpty()) {
        m_pid = QString::fromLocal8Bit(pid);

        const QByteArray sock = qgetenv("SSH_AUTH_SOCK");
        if (!sock.isEmpty()) {
            m_authSock = QString::fromLocal8Bit(sock);
        }
        /* make sure that we have a askpass program.
         * on some systems something like that isn't installed.*/
        m_isOurAgent = false;
        m_isRunning  = true;
    }
    // We have to start a new ssh-agent process
    else {
        m_isOurAgent = true;
        m_isRunning  = startSshAgent();
    }
    askPassEnv();
    return m_isRunning;
}

void SshAgent::askPassEnv()
{
#ifdef FORCE_ASKPASS
    qCDebug(KDESVN_LOG) << "Using test askpass" << endl;
    qputenv("SSH_ASKPASS", FORCE_ASKPASS);
#else
    const QString kdesvnAskPass(QStringLiteral("kdesvnaskpass"));
    // first search nearby us
    QString askPassPath = QStandardPaths::findExecutable(kdesvnAskPass, {QCoreApplication::applicationDirPath()});
    if (askPassPath.isEmpty()) {
        // now search in PATH
        askPassPath = QStandardPaths::findExecutable(kdesvnAskPass);
    }
    if (askPassPath.isEmpty()) {
        // ok, not found, but maybe ssh-agent does ...
        askPassPath = kdesvnAskPass;
    }
    qputenv("SSH_ASKPASS", askPassPath.toLocal8Bit());
#endif
}

bool SshAgent::addSshIdentities(bool force)
{
    if (m_addIdentitiesDone && !force) {
        return true;
    }

    if (!m_isRunning) {
        qWarning() << "No ssh-agent is running, can not execute ssh-add";
        return false;
    }

    // add identities to ssh-agent
    KProcess proc;

    proc.setEnv(QStringLiteral("SSH_AGENT_PID"), m_pid);
    proc.setEnv(QStringLiteral("SSH_AUTH_SOCK"), m_authSock);

#ifdef FORCE_ASKPASS
    qCDebug(KDESVN_LOG) << "Using test askpass" << endl;
    proc.setEnv("SSH_ASKPASS", FORCE_ASKPASS);
#else
    qCDebug(KDESVN_LOG) << "Using kdesvnaskpass" << endl;
    proc.setEnv(QStringLiteral("SSH_ASKPASS"), QStringLiteral("kdesvnaskpass"));
#endif

    proc << QStringLiteral("ssh-add");
    proc.start();
    // endless
    proc.waitForFinished(-1);

    m_addIdentitiesDone = proc.exitStatus() == QProcess::NormalExit && proc.exitStatus() == 0;
    askPassEnv();
    return m_addIdentitiesDone;
}

void SshAgent::killSshAgent()
{
    if (!m_isRunning || !m_isOurAgent) {
        return;
    }

    QProcess proc;
    proc.start(QStringLiteral("kill"), {m_pid});
    proc.waitForFinished();
}

void SshAgent::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        return;
    }
    const QRegExp cshPidRx(QStringLiteral("setenv SSH_AGENT_PID (\\d*);"));
    const QRegExp cshSockRx(QStringLiteral("setenv SSH_AUTH_SOCK (.*);"));

    const QRegExp bashPidRx(QStringLiteral("SSH_AGENT_PID=(\\d*).*"));
    const QRegExp bashSockRx(QStringLiteral("SSH_AUTH_SOCK=(.*\\.\\d*);.*"));
    const QStringList m_outputLines = m_Output.split(QLatin1Char('\n'), QString::SkipEmptyParts);

    for (const auto &outputLine : m_outputLines) {
        if (m_pid.isEmpty()) {
            int pos = cshPidRx.indexIn(outputLine);
            if (pos > -1) {
                m_pid = cshPidRx.cap(1);
                continue;
            }

            pos = bashPidRx.indexIn(outputLine);
            if (pos > -1) {
                m_pid = bashPidRx.cap(1);
                continue;
            }
        }

        if (m_authSock.isEmpty()) {
            int pos = cshSockRx.indexIn(outputLine);
            if (pos > -1) {
                m_authSock = cshSockRx.cap(1);
                continue;
            }

            pos = bashSockRx.indexIn(outputLine);
            if (pos > -1) {
                m_authSock = bashSockRx.cap(1);
                continue;
            }
        }
    }

}

void SshAgent::slotReceivedStdout()
{
    if (!sshAgent) {
        return;
    }
    m_Output += QString::fromLocal8Bit(sshAgent->readAllStandardOutput());
}

bool SshAgent::startSshAgent()
{
    if (sshAgent) {
        return false;
    }
    sshAgent = new KProcess();
    *sshAgent << QStringLiteral("ssh-agent");

    sshAgent->setOutputChannelMode(KProcess::MergedChannels);

    connect(sshAgent, QOverload<int,QProcess::ExitStatus>::of(&KProcess::finished),
            this, &SshAgent::slotProcessExited);
    connect(sshAgent, &KProcess::readyReadStandardOutput,
            this, &SshAgent::slotReceivedStdout);
    sshAgent->start();
    // wait for process to finish eg. backgrounding
    sshAgent->waitForFinished(-1);
    bool ok = (sshAgent->exitStatus() == QProcess::NormalExit && sshAgent->exitStatus() == 0);
    delete sshAgent;
    sshAgent = nullptr;

    return ok;
}
