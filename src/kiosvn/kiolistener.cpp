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
#include "kiolistener.h"
#include "kiosvn.h"
#include "kdesvndinterface.h"
#include "kio_macros.h"

#include <kdebug.h>
#include <klocale.h>

#include <qvariant.h>

namespace KIO
{

KioListener::KioListener(KIO::kio_svnProtocol *_par)
    : svn::ContextListener(), m_notifyCounter(0), m_External(false), m_HasChanges(false), m_FirstTxDelta(false), m_Canceld(false)
{
    par = _par;
}

KioListener::~KioListener()
{
}

/*!
    \fn KioListener::contextCancel()
 */
bool KioListener::contextCancel()
{
    return par->wasKilled() || m_Canceld;
}

/*!
    \fn KioListener::contextGetLogMessage (QString & msg)
 */
bool KioListener::contextGetLogMessage(QString &msg, const svn::CommitItemList &_items)
{
    Q_UNUSED(_items);
    CON_DBUS_VAL(false);

    QDBusReply<QStringList> res = kdesvndInterface.get_logmsg();

    if (!res.isValid()) {
        kWarning() << "Didn't get a valid reply!" << endl;
        return false;
    }
    QStringList lt = res.value();

    if (lt.count() != 1) {
        msg = "Wrong or missing log (may cancel pressed).";
        kDebug(9510) << msg << endl;
        return false;
    }
    msg = lt[0];

    return true;
}

/*! the content of that method is taken from the notify in kio::svn in KDE SDK */
/* this moment we don't use it full 'cause not all is made via KIO */
void KioListener::contextNotify(const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind , const char *mime_type , svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision)
{
    if (par->wasKilled()) {
        return;
    }
    if (par->checkKioCancel()) {
        m_Canceld = true;
    }
    QString userstring;

    switch (action) {
    case svn_wc_notify_add: {
        if (mime_type && (svn_mime_type_is_binary(mime_type))) {
            userstring = i18n("A (bin) %1", path);
        } else {
            userstring = i18n("A %1", path);
        }
        break;
    }
    break;
    case svn_wc_notify_copy: //copy
        break;
    case svn_wc_notify_delete: //delete
        m_HasChanges = true;
        userstring = i18n("D %1", path);
        break;
    case svn_wc_notify_restore : //restore
        userstring = i18n("Restored %1.", path);
        break;
    case svn_wc_notify_revert : //revert
        userstring = i18n("Reverted %1.", path);
        break;
    case svn_wc_notify_failed_revert: //failed revert
        userstring = i18n("Failed to revert %1.\nTry updating instead.", path);
        break;
    case svn_wc_notify_resolved: //resolved
        userstring = i18n("Resolved conflicted state of %1.", path);
        break;
    case svn_wc_notify_skip: //skip
        if (content_state == svn_wc_notify_state_missing) {
            userstring = i18n("Skipped missing target %1.", path);
        } else {
            userstring = i18n("Skipped %1.", path);
        }
        break;
    case svn_wc_notify_update_delete: //update_delete
        m_HasChanges = true;
        userstring = i18n("D %1", path);
        break;
    case svn_wc_notify_update_add: //update_add
        m_HasChanges = true;
        userstring = i18n("A %1", path);
        break;
    case svn_wc_notify_update_update: { //update_update
        /* If this is an inoperative dir change, do no notification.
           An inoperative dir change is when a directory gets closed
           without any props having been changed. */
        if (!((kind == svn_node_dir)
                && ((prop_state == svn_wc_notify_state_inapplicable)
                    || (prop_state == svn_wc_notify_state_unknown)
                    || (prop_state == svn_wc_notify_state_unchanged)))) {
            m_HasChanges = true;

            if (kind == svn_node_file) {
                if (content_state == svn_wc_notify_state_conflicted) {
                    userstring = QLatin1Char('C');
                } else if (content_state == svn_wc_notify_state_merged) {
                    userstring = QLatin1Char('G');
                } else if (content_state == svn_wc_notify_state_changed) {
                    userstring = QLatin1Char('U');
                }
            }

            if (prop_state == svn_wc_notify_state_conflicted) {
                userstring += QLatin1Char('C');
            } else if (prop_state == svn_wc_notify_state_merged) {
                userstring += QLatin1Char('G');
            } else if (prop_state == svn_wc_notify_state_changed) {
                userstring += QLatin1Char('U');
            } else {
                userstring += QLatin1Char(' ');
            }

            if (!((content_state == svn_wc_notify_state_unchanged
                    || content_state == svn_wc_notify_state_unknown)
                    && (prop_state == svn_wc_notify_state_unchanged
                        || prop_state == svn_wc_notify_state_unknown))) {
                userstring += QLatin1Char(' ') + path;
            }
        }
        break;
    }
    case svn_wc_notify_update_completed: { //update_completed
        if (!m_External) {
            if (SVN_IS_VALID_REVNUM(revision)) {
                userstring = i18n("Finished at revision %1.", revision);
            } else {
                userstring = i18n("Update finished.");
            }
        } else {
            if (SVN_IS_VALID_REVNUM(revision)) {
                userstring = i18n("Finished external at revision %1.", revision);
            } else {
                userstring = i18n("Finished external.");
            }
        }
    }
    if (m_External) {
        m_External = false;
    }
    break;
    case svn_wc_notify_update_external: //update_external
        m_External = true;
        userstring = i18n("Fetching external item into %1.", path);
        break;
    case svn_wc_notify_status_completed: //status_completed
        if (SVN_IS_VALID_REVNUM(revision)) {
            userstring = i18n("Status against revision: %1.", revision);
        }
        break;
    case svn_wc_notify_status_external: //status_external
        userstring = i18n("Performing status on external item at %1.", path);
        break;
    case svn_wc_notify_commit_modified: //commit_modified
        userstring = i18n("Sending %1.", path);
        break;
    case svn_wc_notify_commit_added: //commit_added
        if (mime_type && svn_mime_type_is_binary(mime_type)) {
            userstring = i18n("Adding (bin) %1.", path);
        } else {
            userstring = i18n("Adding %1.", path);
        }
        break;
    case svn_wc_notify_commit_deleted: //commit_deleted
        userstring = i18n("Deleting %1.", path);
        break;
    case svn_wc_notify_commit_replaced: //commit_replaced
        userstring = i18n("Replacing %1.", path);
        break;
    case svn_wc_notify_commit_postfix_txdelta: //commit_postfix_txdelta
        if (!m_FirstTxDelta) {
            m_FirstTxDelta = true;
            // check fullstops!
            userstring = i18n("Transmitting file data ");
        } else {
            userstring = QLatin1Char('.');
        }
        break;
    case svn_wc_notify_blame_revision: //blame_revision
        break;
    default:
        break;
    }
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "path" , QString::fromUtf8(path));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "action", QString::number(action));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "kind", QString::number(kind));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "mime_t", QString::fromUtf8(mime_type));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "content", QString::number(content_state));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "prop", QString::number(prop_state));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "rev", QString::number(revision));
    par->setMetaData(QString::number(counter()).rightJustified(10, '0') + "string", userstring);
    incCounter();
}

void KioListener::contextNotify(const svn_wc_notify_t *action)
{
    if (!action) {
        return;
    }
//    if (action->action<svn_wc_notify_locked) {
    contextNotify(action->path, action->action, action->kind, action->mime_type,
                  action->content_state, action->prop_state, action->revision);
//        return;
//    }
//    QString aString = NotifyAction(action->action);
}

svn::ContextListener::SslServerTrustAnswer
KioListener::contextSslServerTrustPrompt(const SslServerTrustData &data, apr_uint32_t &acceptedFailures)
{
    Q_UNUSED(acceptedFailures);
    QDBusReply<int> res;

    CON_DBUS_VAL(DONT_ACCEPT);
    res = kdesvndInterface.get_sslaccept(data.hostname,
                                         data.fingerprint, data.validFrom, data.validUntil, data.issuerDName, data.realm);

    if (!res.isValid()) {
        kWarning() << "Unexpected reply type";
        return DONT_ACCEPT;
    }

    switch (res.value()) {
    case -1:
        return DONT_ACCEPT;
        break;
    case 1:
        return ACCEPT_PERMANENTLY;
        break;
    default:
    case 0:
        return ACCEPT_TEMPORARILY;
        break;
    }
    /* avoid compiler warnings */
    return ACCEPT_TEMPORARILY;
}

bool KioListener::contextLoadSslClientCertPw(QString &password, const QString &realm)
{
    QDBusReply<QString> res;

    CON_DBUS_VAL(false);
    res = kdesvndInterface.load_sslclientcertpw(realm);
    if (!res.isValid()) {
        kWarning() << "Unexpected reply type";
        return false;
    }
    password = res.value();
    return true;
}

bool KioListener::contextSslClientCertPrompt(QString &certFile)
{
    QDBusReply<QString> res;

    CON_DBUS_VAL(false);
    res = kdesvndInterface.get_sslclientcertfile();
    if (!res.isValid()) {
        kWarning() << "Unexpected reply type";
        return false;
    }
    certFile = res.value();
    if (certFile.isEmpty()) {
        return false;
    }
    return true;
}

bool KioListener::contextSslClientCertPwPrompt(QString &password,
        const QString &realm, bool &maySave)
{
    Q_UNUSED(password);
    Q_UNUSED(realm);
    Q_UNUSED(maySave);
    return false;
}

bool KioListener::contextGetSavedLogin(const QString &realm, QString &username, QString &password)
{
    QDBusReply<QStringList> res;

    CON_DBUS_VAL(false);
    res = kdesvndInterface.get_saved_login(realm, username);
    if (!res.isValid()) {
        kWarning() << "Unexpected reply type";
        return false;
    }
    QStringList lt = res.value();
    if (lt.count() != 2) {
        kDebug(9510) << "Wrong or missing auth list." << endl;
        return false;
    }
    username = lt[0];
    password = lt[1];
    return true;
}

bool KioListener::contextGetCachedLogin(const QString &realm, QString &username, QString &password)
{
    Q_UNUSED(realm);
    Q_UNUSED(username);
    Q_UNUSED(password);
    return true;
}

bool KioListener::contextGetLogin(const QString &realm, QString &username, QString &password, bool &maySave)
{
    QDBusReply<QStringList> res;

    CON_DBUS_VAL(false);
    res = kdesvndInterface.get_login(realm, username);
    if (!res.isValid()) {
        kWarning() << "Unexpected reply type";
        return false;
    }
    QStringList lt = res.value();
    if (lt.count() != 3) {
        kDebug(9510) << "Wrong or missing auth list (may cancel pressed)." << endl;
        return false;
    }
    username = lt[0];
    password = lt[1];
    maySave = lt[2] == "true";
    return true;
}

bool KioListener::contextAddListItem(svn::DirEntries *entries, const svn_dirent_t *dirent, const svn_lock_t *lock, const QString &path)
{
    Q_UNUSED(entries);
    if (!dirent || path.isEmpty()) {
        // the path itself? is a problem within kio
        return false;
    }
    if (par) {
        if (par->checkKioCancel()) {
            m_Canceld = true;
        }
        par->listSendDirEntry(svn::DirEntry(path, dirent, lock));
        return true;
    }
    return false;
}

/*!
    \fn KioListener::contextProgress(long long int current, long long int max)
 */
void KioListener::contextProgress(long long int cur, long long int max)
{
    if (par) {
        if (par->checkKioCancel()) {
            m_Canceld = true;
        }
        par->contextProgress(cur, max);
    }
}

} // namespace KIO
