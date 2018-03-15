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

#include "kdesvnd.h"
#include "kdesvn-config.h"
#include "kdesvnd_listener.h"
#include "ksvnwidgets/authdialogimpl.h"
#include "ksvnwidgets/ssltrustprompt.h"
#include "ksvnwidgets/commitmsg_impl.h"
#include "ksvnwidgets/pwstorage.h"
#include "helpers/kdesvn_debug.h"

#include "settings/kdesvnsettings.h"
#include "svnqt/client.h"
#include "svnqt/revision.h"
#include "svnqt/status.h"
#include "svnqt/url.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/client_parameter.h"
#include "helpers/ktranslateurl.h"
#include "helpers/stringhelper.h"
#include "kdesvndadaptor.h"
#include "ksvnjobview.h"

#include <KIO/Global>
#include <KLocalizedString>
#include <KNotification>
#include <KPasswordDialog>
#include <KPluginFactory>

#include <QFileDialog>
#include <QVariant>
#include <QDBusConnection>
#include <QApplication>

K_PLUGIN_FACTORY_WITH_JSON(KdeSvndFactory,
                           "kdesvnd.json",
                           registerPlugin<kdesvnd>();
                          )

#define CHECK_KIO     if (!progressJobView.contains(kioid)) { \
        return;\
    }

kdesvnd::kdesvnd(QObject *parent, const QList<QVariant> &) : KDEDModule(parent),
    m_uiserver(QStringLiteral("org.kde.JobViewServer"), QStringLiteral("/JobViewServer"), QDBusConnection::sessionBus())
{
    m_Listener = new KdesvndListener(this);
    new KdesvndAdaptor(this);
}

kdesvnd::~kdesvnd()
{
    delete m_Listener;
}

QStringList kdesvnd::getTopLevelActionMenu(const QStringList &urlList) const
{
    // we get correct urls here
    QList<QUrl> urls;
    urls.reserve(urlList.size());
    Q_FOREACH(const QString &str, urlList) {
        if (str.contains(QLatin1Char('@')))
            urls += QUrl(str + QLatin1Char('@'));
        else
            urls += QUrl(str);
    }

    return getActionMenu(urls, true);
}

QStringList kdesvnd::getActionMenu(const QStringList &urlList) const
{
    // we get correct urls here
    QList<QUrl> urls;
    urls.reserve(urlList.size());
    Q_FOREACH(const QString &str, urlList) {
        if (str.contains(QLatin1Char('@')))
            urls += QUrl(str + QLatin1Char('@'));
        else
            urls += QUrl(str);
    }
    return getActionMenu(urls, false);
}

QStringList kdesvnd::getActionMenu(const QList<QUrl> &list, bool toplevel) const
{
    QStringList result;
    Kdesvnsettings::self()->load();
    if (Kdesvnsettings::no_konqueror_contextmenu() || list.isEmpty() ||
            !list.at(0).isLocalFile() ||
            (toplevel && Kdesvnsettings::no_konqueror_toplevelmenu())) {
        return result;
    }

    const bool itemIsWc = isWorkingCopy(list[0]);

    const QUrl _dir(list.at(0).adjusted(QUrl::RemoveFilename).adjusted(QUrl::StripTrailingSlash));
    const bool parentIsWc = isWorkingCopy(_dir);

    bool itemIsRepository = false;
    if (!parentIsWc && !itemIsWc) {
        itemIsRepository = isRepository(list[0]);
    }

    if (!itemIsWc) {
        if (itemIsRepository) {
            result << QStringLiteral("Export")
                   << QStringLiteral("Checkout");
        } else {
            result << QStringLiteral("Exportto")
                   << QStringLiteral("Checkoutto");
        }
    } else {
        result << QStringLiteral("Update")
               << QStringLiteral("Commit");
    }

    if (!parentIsWc && !itemIsWc) {
        if (itemIsRepository) {
            result << QStringLiteral("Log");
            if (!toplevel) {
                result << QStringLiteral("Info");
                const QUrl upUrl = KIO::upUrl(list.at(0));
                if (isRepository(upUrl)) {
                    result << QStringLiteral("Blame")
                           << QStringLiteral("Rename");
                }
                result << QStringLiteral("Tree");
            }
        }
    } else if (!toplevel) {
        if (!itemIsWc) {
            result << QStringLiteral("Add");
            return result;
        }

        result << QStringLiteral("Log")
               << QStringLiteral("Tree")
               << QStringLiteral("Info")
               << QStringLiteral("Diff")
               << QStringLiteral("Rename")
               << QStringLiteral("Revert");

        const QUrl url = list.at(0);
        QFileInfo f(url.path());
        if (f.isFile()) {
            result << QStringLiteral("Blame");
        }

        if (f.isDir()) {
            result << QStringLiteral("Addnew");
            result << QStringLiteral("Switch");
        }
    }
    return result;
}

QStringList kdesvnd::getSingleActionMenu(const QString &what) const
{
    QList<QUrl> l;
    l.append(QUrl(what.contains(QLatin1Char('@')) ? what + QLatin1Char('@') : what));
    return getActionMenu(l, false);
}

QStringList kdesvnd::get_saved_login(const QString &realm, const QString &user)
{
    Q_UNUSED(user);
    QString username;
    QString password;
    PwStorage::self()->getLogin(realm, username, password);
    QStringList res;
    res.append(username);
    res.append(password);
    return res;

}

QStringList kdesvnd::get_login(const QString &realm, const QString &user)
{
    QPointer<AuthDialogImpl> auth(new AuthDialogImpl(realm, user));
    QStringList res;
    if (auth->exec() == QDialog::Accepted) {
        res.append(auth->Username());
        res.append(auth->Password());
        if (auth->maySave()) {
            res.append(QStringLiteral("true"));
        } else {
            res.append(QStringLiteral("false"));
        }
    }
    delete auth;
    return res;
}

int kdesvnd::get_sslaccept(const QString &hostname, const QString &fingerprint, const QString &validFrom, const QString &validUntil, const QString &issuerDName, const QString &realm)
{
    bool ok, saveit;
    if (!SslTrustPrompt::sslTrust(
                hostname,
                fingerprint,
                validFrom,
                validUntil,
                issuerDName,
                realm,
                QStringList(),
                &ok, &saveit)) {
        return -1;
    }
    if (!saveit) {
        return 0;
    }
    return 1;
}

QString kdesvnd::load_sslclientcertpw(const QString &realm)
{
    QString password;
    if (!PwStorage::self()->getCertPw(realm, password)) {
        return QString();
    }
    return password;
}

QStringList kdesvnd::get_sslclientcertpw(const QString &realm)
{
    QStringList resList;
    QPointer<KPasswordDialog> dlg(new KPasswordDialog(nullptr, KPasswordDialog::DomainReadOnly | KPasswordDialog::ShowKeepPassword));
    dlg->setDomain(realm);
    dlg->setWindowTitle(i18nc("@title:window", "Enter Password for Realm %1", realm));
    dlg->setKeepPassword(true);
    if (dlg->exec() == KPasswordDialog::Accepted) {
        resList.append(dlg->password());
        if (dlg->keepPassword()) {
            resList.append(QStringLiteral("true"));
        } else {
            resList.append(QStringLiteral("false"));
        }
    }
    delete dlg;
    return resList;
}

QString kdesvnd::get_sslclientcertfile() const
{
    return QFileDialog::getOpenFileName(nullptr, i18n("Open a file with a #PKCS12 certificate"));
}

QStringList kdesvnd::get_logmsg() const
{
    QStringList res;
    bool ok;
    QString logMessage = Commitmsg_impl::getLogmessage(&ok, nullptr, nullptr, nullptr);
    if (ok) {
        res.append(logMessage);
    }
    return res;
}

QString kdesvnd::cleanUrl(const QUrl &url)
{
    return url.adjusted(QUrl::StripTrailingSlash|QUrl::NormalizePathSegments).path();
}

/* just simple name check of course - no network access! */
bool kdesvnd::isRepository(const QUrl &url) const
{
    QString proto = svn::Url::transformProtokoll(url.scheme());
    if (proto == QLatin1String("file")) {
        // local access - may a repository
        svn::StatusParameter params(svn::Path(QLatin1String("file://") + cleanUrl(url)));
        try {
            m_Listener->m_Svnclient->status(params.depth(svn::DepthEmpty).all(false).update(false).noIgnore(false).revision(svn::Revision::HEAD));
        } catch (const svn::ClientException &e) {
            qCDebug(KDESVN_LOG) << e.msg() << endl;
            return false;
        }
        return true;
    }
    return svn::Url::isValid(proto);
}

bool kdesvnd::isWorkingCopy(const QUrl &url) const
{
    if (url.isEmpty() || !url.isLocalFile() || url.scheme() != QLatin1String("file") || url.path() == QLatin1String("/")) {
        return false;
    }
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Listener->m_Svnclient->info(cleanUrl(url), svn::DepthEmpty, rev, peg);
    } catch (const svn::ClientException &e) {
        return false;
    }
    return true;
}

bool kdesvnd::canceldKioOperation(qulonglong kioid)
{
    if (!progressJobView.contains(kioid)) {
        return false;
    }
    return progressJobView[kioid]->state() == KsvnJobView::CANCELD;
}

void kdesvnd::maxTransferKioOperation(qulonglong kioid, qulonglong maxtransfer)
{
    CHECK_KIO;
    progressJobView[kioid]->setState(KsvnJobView::RUNNING);
    progressJobView[kioid]->setTotal(maxtransfer);
}

void kdesvnd::registerKioFeedback(qulonglong kioid)
{
    if (progressJobView.contains(kioid)) {
        return;
    }
    QDBusReply<QDBusObjectPath> reply = m_uiserver.requestView(qApp->applicationName(),
                                                               qApp->applicationName(),
                                                               0x0003);
    if (reply.isValid()) {
        KsvnJobView *jobView = new KsvnJobView(kioid, QStringLiteral("org.kde.JobViewServer"),
                                               reply.value().path(),
                                               QDBusConnection::sessionBus());
        progressJobView.insert(kioid, jobView);
        qCDebug(KDESVN_LOG) << "Register " << kioid << endl;
    } else {
        qCDebug(KDESVN_LOG) << "Could not register " << kioid << endl;
    }
}

void kdesvnd::titleKioOperation(qulonglong kioid, const QString &title, const QString &label)
{
    CHECK_KIO;
    progressJobView[kioid]->setInfoMessage(title);
    progressJobView[kioid]->setDescriptionField(0, i18n("Current task"), label);
}

void kdesvnd::transferredKioOperation(qulonglong kioid, qulonglong transferred)
{
    CHECK_KIO;
    if (progressJobView[kioid]->max() > -1) {
        progressJobView[kioid]->setProcessedAmount(transferred, QStringLiteral("bytes"));
        progressJobView[kioid]->setPercent(progressJobView[kioid]->percent(transferred));
        progressJobView[kioid]->clearDescriptionField(1);
    } else {
        progressJobView[kioid]->setPercent(100.0);
        progressJobView[kioid]->setDescriptionField(1, i18n("Current transfer"), helpers::ByteToString(transferred));
    }
}

void kdesvnd::unRegisterKioFeedback(qulonglong kioid)
{
    CHECK_KIO;
    KsvnJobView *jobView = progressJobView.take(kioid);
    delete jobView;
    qCDebug(KDESVN_LOG) << "Removed " << kioid << endl;
}

void kdesvnd::notifyKioOperation(const QString &text)
{
    KNotification::event(
        QLatin1String("kdesvn-kio"), text,
        QPixmap(), nullptr, KNotification::CloseOnTimeout,
        QLatin1String("kdesvn"));
}

void kdesvnd::errorKioOperation(const QString &text)
{
    KNotification::event(
        KNotification::Error, text,
        QPixmap(), nullptr, KNotification::CloseOnTimeout
    );
}

void kdesvnd::setKioStatus(qulonglong kioid, int status, const QString &message)
{
    CHECK_KIO;
    switch (status) {
    case 0:
        progressJobView[kioid]->setState(KsvnJobView::STOPPED);
        progressJobView[kioid]->terminate(message);
        break;
    case 2:
        progressJobView[kioid]->setState(KsvnJobView::CANCELD);
        progressJobView[kioid]->terminate(message);
        break;
    case 1:
        progressJobView[kioid]->setState(KsvnJobView::RUNNING);
        progressJobView[kioid]->setSuspended(false);
        break;
    }
}

#include "kdesvnd.moc"
