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

#include "sshagent.h"
#include "kdesvn-config.h"

#include <qregexp.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kprocess.h>
#include <kdebug.h>

#include <stdlib.h>


// initialize static member variables
bool    SshAgent::m_isRunning  = false;
bool    SshAgent::m_isOurAgent = false;
bool    SshAgent::m_addIdentitiesDone = false;
QString SshAgent::m_authSock;
QString SshAgent::m_pid;

class SshClean
{
public:
    SshClean() {};

    ~SshClean()
    {
        SshAgent ssh; ssh.killSshAgent();
    }
};

SshAgent::SshAgent(QObject* parent)
    : QObject(parent),sshAgent(0)
{
    static SshClean st;
}


SshAgent::~SshAgent()
{
}


bool SshAgent::querySshAgent()
{
    if( m_isRunning )
        return true;

    // Did the user already start a ssh-agent process?
    QByteArray pid = qgetenv("SSH_AGENT_PID");
    if( pid.length() != 0 )
    {
        m_pid = QString::fromLocal8Bit(pid);

        QByteArray sock = qgetenv("SSH_AUTH_SOCK");
        if( sock.length()>0 )
            m_authSock = QString::fromLocal8Bit(sock);
        /* make sure that we have a askpass program.
         * on some systems something like that isn't installed.*/
#ifdef FORCE_ASKPASS
        kDebug(9510)<<"Using test askpass"<<endl;
#ifdef HAS_SETENV
            ::setenv("SSH_ASKPASS",FORCE_ASKPASS,1);
#else
            ::putenv("SSH_ASKPASS="FORCE_ASKPASS);
#endif
#else
            QString pro = BIN_INSTALL_DIR;
            if (pro.size()>0) {
                pro.append("/");
            }
            pro.append("kdesvnaskpass");
#ifdef HAS_SETENV
            ::setenv("SSH_ASKPASS", pro.toAscii(),1);
#else
            pro = "SSH_ASKPASS="+pro;
            ::putenv(pro.toAscii());
#endif
/*
        }
*/
#endif
        m_isOurAgent = false;
        m_isRunning  = true;
    }
    // We have to start a new ssh-agent process
    else
    {
        m_isOurAgent = true;
        m_isRunning  = startSshAgent();
    }

    return m_isRunning;
}


bool SshAgent::addSshIdentities(bool force)
{
    if (m_addIdentitiesDone && !force) {
        return true;
    }


    if( !m_isRunning || (!m_isOurAgent&&!force)) {
        return false;
    }

    // add identities to ssh-agent
    KProcess proc;

    proc.setEnv("SSH_AGENT_PID", m_pid);
    proc.setEnv("SSH_AUTH_SOCK", m_authSock);

#ifdef FORCE_ASKPASS
    kDebug(9510)<<"Using test askpass"<<endl;
    proc.setEnv("SSH_ASKPASS",FORCE_ASKPASS);
#else
    proc.setEnv("SSH_ASKPASS", "kdesvnaskpass");
#endif

    proc << "ssh-add";
    proc.start();
    // endless
    proc.waitForFinished(-1);

    m_addIdentitiesDone = proc.exitStatus()==QProcess::NormalExit && proc.exitStatus() == 0;
    return m_addIdentitiesDone;
}

void SshAgent::killSshAgent()
{
    if( !m_isRunning || !m_isOurAgent )
        return;

    KProcess proc;

    proc << "kill" << m_pid;

    proc.start();
    proc.waitForFinished();
}


void SshAgent::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus!=QProcess::NormalExit || exitCode!=0) {
        return;
    }
    QRegExp cshPidRx("setenv SSH_AGENT_PID (\\d*);");
    QRegExp cshSockRx("setenv SSH_AUTH_SOCK (.*);");

    QRegExp bashPidRx("SSH_AGENT_PID=(\\d*).*");
    QRegExp bashSockRx("SSH_AUTH_SOCK=(.*\\.\\d*);.*");
    QStringList m_outputLines = m_Output.split('\n',QString::SkipEmptyParts);

    QStringList::Iterator it  = m_outputLines.begin();
    QStringList::Iterator end = m_outputLines.end();
    for( ; it != end; ++it )
    {
        if( m_pid.isEmpty() )
        {
            int pos = cshPidRx.indexIn(*it);
            if( pos > -1 )
            {
                m_pid = cshPidRx.cap(1);
                continue;
            }

            pos = bashPidRx.indexIn(*it);
            if( pos > -1 )
            {
                m_pid = bashPidRx.cap(1);
                continue;
            }
        }

        if( m_authSock.isEmpty() )
        {
            int pos = cshSockRx.indexIn(*it);
            if( pos > -1 )
            {
                m_authSock = cshSockRx.cap(1);
                continue;
            }

            pos = bashSockRx.indexIn(*it);
            if( pos > -1 )
            {
                m_authSock = bashSockRx.cap(1);
                continue;
            }
        }
    }

}


void SshAgent::slotReceivedStdout()
{
    if (!sshAgent) return;
    m_Output+=QString::fromLocal8Bit(sshAgent->readAllStandardOutput());
}


bool SshAgent::startSshAgent()
{
    if (sshAgent)  return false;
    sshAgent = new KProcess();
    *sshAgent << "ssh-agent";

    sshAgent->setOutputChannelMode(KProcess::MergedChannels);

    connect(sshAgent, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(slotProcessExited(int, QProcess::ExitStatus)));
    connect(sshAgent, SIGNAL(readyReadStandardOutput()),
            SLOT(slotReceivedStdout()));
    sshAgent->start();
    // wait for process to finish eg. backgrounding
    sshAgent->waitForFinished(-1);
    bool ok = (sshAgent->exitStatus()==QProcess::NormalExit && sshAgent->exitStatus() == 0);
    delete sshAgent;
    sshAgent=0;

    return ok;
}

#include "sshagent.moc"
