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
#include "logmsg_impl.h"
#include "ssltrustprompt_impl.h"
#include "helpers/stl2qt.h"

#include <klocale.h>
#include <kapp.h>
#include <kinputdialog.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include <qtextstream.h>

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
const int CContextListener::smax_actionstring=svn_wc_notify_failed_unlock+1;
#else
const int CContextListener::smax_actionstring=svn_wc_notify_blame_revision+1;
#endif

const QString CContextListener::action_strings[]={
    I18N_NOOP("Add to revision control"),
    I18N_NOOP("Copy"),
    I18N_NOOP("Delete"),
    I18N_NOOP("Restore missing"),
    I18N_NOOP("Revert"),
    I18N_NOOP("Revert failed"),
    I18N_NOOP("Resolved"),
    I18N_NOOP("Skip"),
    I18N_NOOP("Deleted"),
    I18N_NOOP("Added"),
    I18N_NOOP("Update"), //svn_wc_notify_update_update
    I18N_NOOP("Update complete"),
    I18N_NOOP("Update external module"),
    QString::null, // status completed - will not send is just noisy
    I18N_NOOP("Status on external"), //svn_wc_notify_status_external
    I18N_NOOP("Commit Modified"),
    I18N_NOOP("Commit Added"),
    I18N_NOOP("Commit Deleted"),
    I18N_NOOP("Commit Replaced"),
    QString::null, //tx delta -> making ticks instead
    I18N_NOOP("Blame") //svn_wc_notify_blame_revision
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
    ,
    I18N_NOOP("Locking"),
    I18N_NOOP("Unlocked"),
    I18N_NOOP("Lock failed"),
    I18N_NOOP("Unlock failed")
#endif
};

const QString CContextListener::notify_state_strings[]={
    QString::null, // = 0
    QString::null,
    I18N_NOOP("unchanged"),
    I18N_NOOP("item wasn't present"),
    I18N_NOOP("unversioned item obstructed work"),
    I18N_NOOP("Pristine state was modified."), // should send a signal with path instead of message?
    I18N_NOOP("Modified state had mods merged in."),
    I18N_NOOP("Modified state got conflicting mods.")
};

QString CContextListener::NotifyAction(svn_wc_notify_action_t action)
{
    if (action>=smax_actionstring||action<0) {
        return QString::null;
    }
    return action_strings[action].isEmpty()?QString::null:i18n(action_strings[action]);
}

QString CContextListener::NotifyState(svn_wc_notify_state_t state)
{
    if (state > svn_wc_notify_state_conflicted || state<0) return QString::null;
    return notify_state_strings[state].isEmpty()?QString::null:i18n(notify_state_strings[state]);
}

CContextListener::CContextListener(QObject *parent, const char *name)
 : QObject(parent, name), svn::ContextListener(),ref_count(),m_cancelMe(false)
{
}


CContextListener::~CContextListener()
{
    disconnect();
}

bool CContextListener::contextGetLogin (
                    const QString & realm,
                    QString & username,
                    QString & password,
                    bool & maySave)
{
    emit waitShow(true);
    emit sendNotify(realm);
    AuthDialogImpl auth(realm);
    if (auth.exec()==QDialog::Accepted) {
        username=auth.Username();
        password=auth.Password();
        maySave = auth.maySave();
        emit waitShow(false);
        return true;
    }
    emit waitShow(false);
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
    QString aString = NotifyAction(action);
    if (aString.isEmpty()) {
        emit tickProgress();
        return;
    }
    QString msg;
    QTextStream ts(&msg,IO_WriteOnly);

    ts << NotifyAction(action) << " " << QString::fromUtf8(path);
    if (revision>-1) {
        ts << " (Rev "<<revision<<")";
    }
    aString = NotifyState(content_state);
    if (!aString.isEmpty()) {
        ts << "\n" << aString;
    }
    emit sendNotify(msg);
}

#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
void CContextListener::contextNotify (const svn_wc_notify_t *action)
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

bool CContextListener::contextCancel()
{
    if (m_cancelMe) {
        m_cancelMe=false;
        return true;
    }
    return false;
}

bool CContextListener::contextGetLogMessage (QString & msg)
{
    bool isOk = false;
    emit waitShow(true);
    QString logMessage = Logmsg_impl::getLogmessage(&isOk,0,0,0);
    if (isOk) {
        msg = logMessage;
    }
    emit waitShow(false);
    return isOk;
}

svn::ContextListener::SslServerTrustAnswer CContextListener::contextSslServerTrustPrompt (
    const svn::ContextListener::SslServerTrustData & data , apr_uint32_t & /* acceptedFailures */)
{
    bool ok,saveit;
    emit waitShow(true);
    if (!SslTrustPrompt_impl::sslTrust(
        data.hostname,
        data.fingerprint,
        data.validFrom,
        data.validUntil,
        data.issuerDName,
        data.realm,
        &ok,&saveit)) {
        return DONT_ACCEPT;
    }
    emit waitShow(false);
    if (!saveit) {
        return ACCEPT_TEMPORARILY;
    }
    return ACCEPT_PERMANENTLY;
}

bool CContextListener::contextSslClientCertPrompt (QString & certFile)
{
    kdDebug()<<"CContextListener::contextSslClientCertPrompt "
        << certFile << endl;
    emit waitShow(true);
    QString afile = KFileDialog::getOpenFileName(QString::null,
        QString::null,
        0,
        i18n("Open a file with a #PKCS12 certificate"));
    emit waitShow(false);
    if (afile.isEmpty()) {
        return false;
    }
    certFile = afile;
    return true;
}

bool CContextListener::contextSslClientCertPwPrompt (QString & password,
                                   const QString & realm, bool & maysave)
{
    emit waitShow(true);
    QCString npass;
    int keep = 1;
    int res = KPasswordDialog::getPassword(npass,
        i18n("Enter password for realm %1").arg(realm),
        &keep);
    emit waitShow(false);
    if (res!=KPasswordDialog::Accepted) {
        return false;
    }
    maysave = keep!=0;
    password = npass;
    return true;
}

#include "ccontextlistener.moc"
