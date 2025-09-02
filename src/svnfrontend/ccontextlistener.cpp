/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include "ccontextlistener.h"
#include "fronthelpers/cursorstack.h"
#include "helpers/kdesvn_debug.h"
#include "ksvnwidgets/authdialogimpl.h"
#include "ksvnwidgets/commitmsg_impl.h"
#include "ksvnwidgets/pwstorage.h"
#include "ksvnwidgets/ssltrustprompt.h"
#include "settings/kdesvnsettings.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>

#include <QFileDialog>
#include <QMutex>
#include <QTextStream>

class CContextListenerData
{
public:
    CContextListenerData();
    virtual ~CContextListenerData();

    // data
    bool m_cancelMe;
    QMutex m_CancelMutex;

    bool noDialogs;

    QStringList m_updatedItems;
};

CContextListenerData::CContextListenerData()
    : m_cancelMe(false)
    , m_CancelMutex()
    , noDialogs(false)
    , m_updatedItems()
{
}

CContextListenerData::~CContextListenerData()
{
}

const int CContextListener::smax_actionstring = svn_wc_notify_failed_unlock + 1;

const KLocalizedString CContextListener::action_strings[] = {ki18n("Add to revision control"),
                                                             ki18n("Copy"),
                                                             ki18n("Delete"),
                                                             ki18n("Restore missing"),
                                                             ki18n("Revert"),
                                                             ki18n("Revert failed"),
                                                             ki18n("Resolved"),
                                                             ki18n("Skip"),
                                                             ki18n("Deleted"),
                                                             ki18n("Added"),
                                                             ki18n("Update"), // svn_wc_notify_update_update
                                                             ki18n("Update complete"),
                                                             ki18n("Update external module"),
                                                             KLocalizedString(), // status completed - will not send is just noisy
                                                             ki18n("Status on external"), // svn_wc_notify_status_external
                                                             ki18n("Commit Modified"),
                                                             ki18n("Commit Added"),
                                                             ki18n("Commit Deleted"),
                                                             ki18n("Commit Replaced"),
                                                             KLocalizedString(), // tx delta -> making ticks instead
                                                             KLocalizedString(), // svn_wc_notify_blame_revision - using ticks
                                                             ki18n("Locking"),
                                                             ki18n("Unlocked"),
                                                             ki18n("Lock failed"),
                                                             ki18n("Unlock failed")};

const KLocalizedString CContextListener::notify_state_strings[] = {
    KLocalizedString(), // = 0
    KLocalizedString(),
    ki18n("unchanged"),
    ki18n("item wasn't present"),
    ki18n("unversioned item obstructed work"),
    // ki18n("Pristine state was modified."), // should send a signal with path instead of message?
    KLocalizedString(),
    ki18n("Modified state had mods merged in."),
    ki18n("Modified state got conflicting mods.")};

QString CContextListener::NotifyAction(svn_wc_notify_action_t action)
{
    if (action >= smax_actionstring || action < 0) {
        return QString();
    }
    return (action_strings[action].isEmpty()) ? QString() : action_strings[action].toString();
}

QString CContextListener::NotifyState(svn_wc_notify_state_t state)
{
    if (state > svn_wc_notify_state_conflicted || state < 0) {
        return QString();
    }
    return (notify_state_strings[state].isEmpty()) ? QString() : notify_state_strings[state].toString();
}

CContextListener::CContextListener(QObject *parent)
    : QObject(parent)
    , svn::ContextListener()
    , m_Data(new CContextListenerData())
{
}

CContextListener::~CContextListener()
{
    disconnect();
    delete m_Data;
}

bool CContextListener::contextGetCachedLogin(const QString &realm, QString &username, QString &password)
{
    PwStorage::self()->getCachedLogin(realm, username, password);
    return true;
}

bool CContextListener::contextGetSavedLogin(const QString &realm, QString &username, QString &password)
{
    if (!Kdesvnsettings::passwords_in_wallet()) {
        return true;
    }
    Q_EMIT waitShow(true);
    PwStorage::self()->getLogin(realm, username, password);
    PwStorage::self()->setCachedLogin(realm, username, password);
    Q_EMIT waitShow(false);
    /* the return value isn't interesting to us... */
    return true;
}

bool CContextListener::contextGetLogin(const QString &realm, QString &username, QString &password, bool &maySave)
{
    bool ret = false;
    maySave = false;
    Q_EMIT waitShow(true);
    Q_EMIT sendNotify(realm);
    QPointer<AuthDialogImpl> auth(new AuthDialogImpl(realm, username));
    if (auth->exec() == QDialog::Accepted) {
        username = auth->Username();
        password = auth->Password();
        maySave = (Kdesvnsettings::passwords_in_wallet() ? false : auth->maySave());
        if (Kdesvnsettings::passwords_in_wallet() && auth->maySave()) {
            PwStorage::self()->setLogin(realm, username, password);
        }
        if (Kdesvnsettings::use_password_cache()) {
            PwStorage::self()->setCachedLogin(realm, username, password);
        }
        ret = true;
    }
    delete auth;
    Q_EMIT waitShow(false);
    return ret;
}

void CContextListener::contextNotify(const QString &aMsg)
{
    if (aMsg.isEmpty()) {
        Q_EMIT tickProgress();
    } else {
        Q_EMIT sendNotify(aMsg);
    }
}

void CContextListener::contextNotify(const char *path,
                                     svn_wc_notify_action_t action,
                                     svn_node_kind_t /* kind */,
                                     const char *mime_type,
                                     svn_wc_notify_state_t content_state,
                                     svn_wc_notify_state_t prop_state,
                                     svn_revnum_t revision)
{
    Q_UNUSED(mime_type);
    Q_UNUSED(prop_state);

    QString msg;
    QString aString = NotifyAction(action);
    extraNotify(QString::fromUtf8(path), action, revision);
    if (!aString.isEmpty()) {
        QTextStream ts(&msg, QIODevice::WriteOnly);
        ts << NotifyAction(action) << " " << QString::fromUtf8(path);
        if (revision > -1) {
            ts << " (Rev " << revision << ")";
        }
        aString = NotifyState(content_state);
        if (!aString.isEmpty()) {
            ts << "\n" << aString;
        }
    }
    contextNotify(msg);
}

void CContextListener::contextNotify(const svn_wc_notify_t *action)
{
    if (!action) {
        return;
    }
    //    if (action->action<svn_wc_notify_locked) {
    contextNotify(action->path, action->action, action->kind, action->mime_type, action->content_state, action->prop_state, action->revision);
    //        return;
    //    }
    //    QString aString = NotifyAction(action->action);
}

void CContextListener::sendTick()
{
    Q_EMIT tickProgress();
}

bool CContextListener::contextCancel()
{
    {
        QMutexLocker lock(&(m_Data->m_CancelMutex));
        if (m_Data->m_cancelMe) {
            m_Data->m_cancelMe = false;
            return true;
        }
    }
    // otherwise deadlock!
    sendTick();
    return false;
}

bool CContextListener::contextGetLogMessage(QString &msg, const svn::CommitItemList &items)
{
    bool isOk = false;
    Q_EMIT waitShow(true);
    QString logMessage = Commitmsg_impl::getLogmessage(items, &isOk, nullptr, nullptr, nullptr);
    if (isOk) {
        msg = logMessage;
    }
    Q_EMIT waitShow(false);
    return isOk;
}

svn::ContextListener::SslServerTrustAnswer CContextListener::contextSslServerTrustPrompt(const svn::ContextListener::SslServerTrustData &data,
                                                                                         apr_uint32_t &acceptedFailures)
{
    CursorStack cs(Qt::ArrowCursor);

    bool ok, saveit;
    Q_EMIT waitShow(true);
    if (!SslTrustPrompt::sslTrust(data.hostname,
                                  data.fingerprint,
                                  data.validFrom,
                                  data.validUntil,
                                  data.issuerDName,
                                  data.realm,
                                  failure2Strings(acceptedFailures),
                                  &ok,
                                  &saveit)) {
        return DONT_ACCEPT;
    }
    Q_EMIT waitShow(false);
    if (!saveit) {
        return ACCEPT_TEMPORARILY;
    }
    return ACCEPT_PERMANENTLY;
}

bool CContextListener::contextSslClientCertPrompt(QString &certFile)
{
    qCDebug(KDESVN_LOG) << certFile << Qt::endl;
    Q_EMIT waitShow(true);
    QString afile = QFileDialog::getOpenFileName(nullptr, i18n("Open a file with a #PKCS12 certificate"));
    Q_EMIT waitShow(false);
    if (afile.isEmpty()) {
        return false;
    }
    certFile = afile;
    return true;
}

bool CContextListener::contextLoadSslClientCertPw(QString &password, const QString &realm)
{
    PwStorage::self()->getCertPw(realm, password);
    return true;
}

bool CContextListener::contextSslClientCertPwPrompt(QString &password, const QString &realm, bool &maysave)
{
    maysave = false;
    Q_EMIT waitShow(true);
    QString npass;
    QPointer<KPasswordDialog> dlg(new KPasswordDialog(nullptr));
    dlg->setPrompt(i18n("Enter password for realm %1", realm));
    dlg->setWindowTitle(realm);
    int res = dlg->exec();
    if (res == QDialog::Accepted) {
        npass = dlg->password();
    }
    bool keepPw = (dlg ? dlg->keepPassword() : false);
    delete dlg;

    Q_EMIT waitShow(false);
    if (res != QDialog::Accepted) {
        return false;
    }
    maysave = (Kdesvnsettings::passwords_in_wallet() ? false : keepPw);
    if (Kdesvnsettings::store_passwords() && keepPw) {
        PwStorage::self()->setCertPw(realm, password);
    }
    password = npass;
    return true;
}

void CContextListener::setCanceled(bool how)
{
    QMutexLocker lock(&(m_Data->m_CancelMutex));
    m_Data->m_cancelMe = how;
}

QStringList CContextListener::failure2Strings(apr_uint32_t acceptedFailures)
{
    QStringList res;
    if (acceptedFailures & SVN_AUTH_SSL_UNKNOWNCA) {
        res << i18n("The certificate is not issued by a trusted authority. Use the fingerprint to validate the certificate manually.");
    }
    if (acceptedFailures & SVN_AUTH_SSL_CNMISMATCH) {
        res << i18n("The certificate hostname does not match.");
    }
    if (acceptedFailures & SVN_AUTH_SSL_NOTYETVALID) {
        res << i18n("The certificate is not yet valid.");
    }
    if (acceptedFailures & SVN_AUTH_SSL_EXPIRED) {
        res << i18n("The certificate has expired.");
    }
    if (acceptedFailures & SVN_AUTH_SSL_OTHER) {
        res << i18n("The certificate has an unknown error.");
    }
    return res;
}

QString CContextListener::translate(const QString &what)
{
    return i18n(what.toLocal8Bit().constData());
}

/*!
    \fn CContextListener::contextProgress(long long int current, long long int max)
 */
void CContextListener::contextProgress(long long int current, long long int max)
{
    Q_EMIT netProgress(current, max);
}

void CContextListener::maySavePlaintext(svn_boolean_t *may_save_plaintext, const QString &realmstring)
{
    Q_EMIT waitShow(true);
    if (may_save_plaintext) {
        QString question = i18n("%1\nReally store password as plain text?", realmstring);
        QString head = i18n("Save password");
        KGuiItem contButton(i18nc("@action:button", "Store in Plain Text"));
        if (KMessageBox::warningContinueCancel(nullptr, question, head, contButton) == KMessageBox::Continue) {
            *may_save_plaintext = true;
        } else {
            *may_save_plaintext = false;
        }
    }
    Q_EMIT waitShow(false);
}

const QStringList &CContextListener::updatedItems() const
{
    return m_Data->m_updatedItems;
}

void CContextListener::cleanUpdatedItems()
{
    m_Data->m_updatedItems.clear();
}

void CContextListener::extraNotify(const QString &path, svn_wc_notify_action_t action, svn_revnum_t revision)
{
    Q_UNUSED(revision);
    switch (action) {
    case svn_wc_notify_update_update:
    case svn_wc_notify_update_add:
    case svn_wc_notify_update_delete:
        m_Data->m_updatedItems.append(path);
        break;
    default:
        break;
    }
}

#include "moc_ccontextlistener.cpp"
