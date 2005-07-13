/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ccontextlistener.h"
#include <klocale.h>

const int CContextListener::smax_actionstring=svn_wc_notify_blame_revision+1;

const QString CContextListener::action_strings[]={
    I18N_NOOP("Add"),
    I18N_NOOP("Copy"),
    I18N_NOOP("Delete"),
    I18N_NOOP("Restore"),
    I18N_NOOP("Revert"),
    I18N_NOOP("Revert failed"),
    I18N_NOOP("Resolved"),
    I18N_NOOP("Skip"),
    I18N_NOOP("Deleted"),
    I18N_NOOP("Added"),
    I18N_NOOP("Updated"),
    I18N_NOOP("Update complete"),
    I18N_NOOP("Update external"),
    I18N_NOOP("Status complete"),
    I18N_NOOP("Status external"),
    I18N_NOOP("Commit Modified"),
    I18N_NOOP("Commit Added"),
    I18N_NOOP("Commit Deleted"),
    I18N_NOOP("Commit Replaced"),
    I18N_NOOP("Postfix txdelta"),
    I18N_NOOP("Blame")
};

QString CContextListener::NotifyAction(svn_wc_notify_action_t action)
{
    if (action>=smax_actionstring||action<0) {
        return QString::null;
    }
    return action_strings[action];
}

CContextListener::CContextListener(QObject *parent, const char *name)
 : QObject(parent, name), svn::ContextListener()
{
}


CContextListener::~CContextListener()
{
}

bool CContextListener::contextGetLogin (
                    const std::string & /*realm*/,
                    std::string & /*username*/,
                    std::string & /* password */,
                    bool & /* maySave*/)
{
    return false;
}

void CContextListener::contextNotify (const char *path,
                    svn_wc_notify_action_t action,
                    svn_node_kind_t /* kind */,
                    const char *mime_type,
                    svn_wc_notify_state_t content_state,
                    svn_wc_notify_state_t prop_state,
                    svn_revnum_t revision)
{
    //qDebug("Notify: %s - %i - %s",path,action,mime_type);
    QString aString = NotifyAction(action);
    if (aString.isEmpty()) {
        return;
    }
    QString msg = QString("%1 %2 (Rev %3)").arg(NotifyAction(action)).
        arg(QString::fromLocal8Bit(path)).
        arg(revision);
    emit sendNotify(msg);
}

bool CContextListener::contextCancel()
{
    return false;
}

bool CContextListener::contextGetLogMessage (std::string & msg)
{
    return false;
}

svn::ContextListener::SslServerTrustAnswer CContextListener::contextSslServerTrustPrompt (
    const svn::ContextListener::SslServerTrustData & /* data */, apr_uint32_t & /* acceptedFailures */)
{
}

bool CContextListener::contextSslClientCertPrompt (std::string & /* certFile*/)
{
    return false;
}

bool CContextListener::contextSslClientCertPwPrompt (std::string & /* password */,
                                   const std::string & /* realm */, bool & /* maySave*/)
{
    return false;
}


#include "ccontextlistener.moc"
