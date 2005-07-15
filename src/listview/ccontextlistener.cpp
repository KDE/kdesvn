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
#include "authdialogimpl.h"
#include <klocale.h>
#include <kinputdialog.h>
#include <klocale.h>

const int CContextListener::smax_actionstring=svn_wc_notify_blame_revision+1;

const QString CContextListener::action_strings[]={
    i18n("Add"),
    i18n("Copy"),
    i18n("Delete"),
    i18n("Restore"),
    i18n("Revert"),
    i18n("Revert failed"),
    i18n("Resolved"),
    i18n("Skip"),
    i18n("Deleted"),
    i18n("Added"),
    i18n("Updated"),
    i18n("Update complete"),
    i18n("Update external"),
    i18n("Status complete"),
    i18n("Status external"),
    i18n("Commit Modified"),
    i18n("Commit Added"),
    i18n("Commit Deleted"),
    i18n("Commit Replaced"),
    i18n("Postfix txdelta"),
    i18n("Blame")
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
                    const std::string & realm,
                    std::string & username,
                    std::string & password,
                    bool & maySave)
{
    emit sendNotify(QString::fromLocal8Bit(realm.c_str()));
    AuthDialogImpl auth(QString::fromLocal8Bit(realm.c_str()));
    if (auth.exec()==QDialog::Accepted) {
        username.assign(auth.Username().local8Bit());
        password.assign(auth.Password().local8Bit());
        maySave = auth.maySave();
        return true;
    }
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
    bool isOk = false;
    QString logMessage = KInputDialog::getMultiLineText(i18n("Logmessage"),i18n("Enter a logmessage"),QString::null,&isOk);
    if (isOk) {
        msg.assign(logMessage.local8Bit());
    }
    return isOk;
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
