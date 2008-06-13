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

#include "sshagent.h"
#include "kdesvn-config.h"

#include <qregexp.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <k3process.h>
#include <kdebug.h>

#include <stdlib.h>


// initialize static member variables
bool    SshAgent::m_isRunning  = false;
bool    SshAgent::m_isOurAgent = false;
bool    SshAgent::m_addIdentitiesDone = false;
QString SshAgent::m_authSock   = QString::null;
QString SshAgent::m_pid        = QString::null;


SshAgent::SshAgent(QObject* parent)
    : QObject(parent)
{
}


SshAgent::~SshAgent()
{
}


bool SshAgent::querySshAgent()
{
    if( m_isRunning )
        return true;

    // Did the user already start a ssh-agent process?
    char* pid;
    if( (pid = ::getenv("SSH_AGENT_PID")) != 0 )
    {
        m_pid = QString::fromLocal8Bit(pid);

        char* sock = ::getenv("SSH_AUTH_SOCK");
        if( sock )
            m_authSock = QString::fromLocal8Bit(sock);
        /* make sure that we have a askpass program.
         * on some systems something like that isn't installed.*/
#ifdef FORCE_ASKPASS
        kDebug()<<"Using test askpass"<<endl;
#ifdef HAS_SETENV
            ::setenv("SSH_ASKPASS",FORCE_ASKPASS,1);
#else
            ::putenv("SSH_ASKPASS="FORCE_ASKPASS);
#endif
#else
/*
        char*agent = ::getenv("SSH_ASKPASS");
        if (!agent) {
*/
#ifdef HAS_SETENV
            ::setenv("SSH_ASKPASS", "kdesvnaskpass",1);
#else
            ::putenv("SSH_ASKPASS=kdesvnaskpass");
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
    K3Process proc;

    proc.setEnvironment("SSH_AGENT_PID", m_pid);
    proc.setEnvironment("SSH_AUTH_SOCK", m_authSock);

#ifdef FORCE_ASKPASS
    kDebug()<<"Using test askpass"<<endl;
    proc.setEnvironment("SSH_ASKPASS",FORCE_ASKPASS);
#else
    char*agent = 0;
/*
    if (force) {
        agent = ::getenv("SSH_ASKPASS");
    }
*/
    if (!agent) {
        proc.setEnvironment("SSH_ASKPASS", "kdesvnaskpass");
    }
#endif

    proc << "ssh-add";

    connect(&proc, SIGNAL(receivedStdout(K3Process*, char*, int)),
            SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(&proc, SIGNAL(receivedStderr(K3Process*, char*, int)),
            SLOT(slotReceivedStderr(K3Process*, char*, int)));

    proc.start(K3Process::DontCare, K3Process::AllOutput);

    // wait for process to finish
    // TODO CL use timeout?
    proc.wait();

    m_addIdentitiesDone = proc.normalExit() && proc.exitStatus() == 0;
    return m_addIdentitiesDone;
}


void SshAgent::killSshAgent()
{
    if( !m_isRunning || !m_isOurAgent )
        return;

    K3Process proc;

    proc << "kill" << m_pid;

    proc.start(K3Process::DontCare, K3Process::NoCommunication);
}


void SshAgent::slotProcessExited(K3Process*)
{
    QRegExp cshPidRx("setenv SSH_AGENT_PID (\\d*);");
    QRegExp cshSockRx("setenv SSH_AUTH_SOCK (.*);");

    QRegExp bashPidRx("SSH_AGENT_PID=(\\d*).*");
    QRegExp bashSockRx("SSH_AUTH_SOCK=(.*\\.\\d*);.*");
    QStringList m_outputLines = m_Output.split("\n",QString::SkipEmptyParts);

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


void SshAgent::slotReceivedStdout(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);
    m_Output+=output;
}


void SshAgent::slotReceivedStderr(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);
    m_Output+=output;
}


bool SshAgent::startSshAgent()
{
    K3Process proc;

    proc << "ssh-agent";

    connect(&proc, SIGNAL(processExited(K3Process*)),
            SLOT(slotProcessExited(K3Process*)));
    connect(&proc, SIGNAL(receivedStdout(K3Process*, char*, int)),
            SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(&proc, SIGNAL(receivedStderr(K3Process*, char*, int)),
            SLOT(slotReceivedStderr(K3Process*, char*, int)) );

    proc.start(K3Process::NotifyOnExit, K3Process::All);

    // wait for process to finish
    // TODO CL use timeout?
    proc.wait();

    return (proc.normalExit() && proc.exitStatus() == 0);
}

#include "sshagent.moc"
