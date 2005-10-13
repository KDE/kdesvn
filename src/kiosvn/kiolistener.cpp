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
#include "kiolistener.h"

KioListener::KioListener()
 : svn::ContextListener()
{
}


KioListener::~KioListener()
{
}




/*!
    \fn KioListener::contextCancel()
 */
bool KioListener::contextCancel()
{
    return false;
}


/*!
    \fn KioListener::contextGetLogMessage (QString & msg)
 */
bool KioListener::contextGetLogMessage (QString & msg)
{
    msg = "";
    return true;
}

void KioListener::contextNotify (const char * /* path */,
                    svn_wc_notify_action_t /* action */,
                    svn_node_kind_t /* kind */,
                    const char * /* mime_type */,
                    svn_wc_notify_state_t /* content_state */,
                    svn_wc_notify_state_t /* prop_state */,
                    svn_revnum_t /* revision */)
{
}

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
void KioListener::contextNotify (const svn_wc_notify_t *action)
{
    if (!action) return;
//    if (action->action<svn_wc_notify_locked) {
        contextNotify(action->path,action->action,action->kind,action->mime_type,
            action->content_state,action->prop_state,action->revision);
//        return;
//    }
//    QString aString = NotifyAction(action->action);
}
#endif

svn::ContextListener::SslServerTrustAnswer
KioListener::contextSslServerTrustPrompt (const SslServerTrustData & data,
                                 apr_uint32_t & acceptedFailures)
{
    return ACCEPT_TEMPORARILY;
}

bool KioListener::contextSslClientCertPrompt (QString & certFile)
{
    return false;
}

bool KioListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maySave)
{
    return false;
}

bool KioListener::contextGetLogin (const QString & /*realm*/,
                     QString & /*username*/,
                     QString & /*password*/,
                     bool & /*maySave*/)
{
    return false;
}

