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
#include "src/ksvnwidgets/authdialogimpl.h"
#include "src/ksvnwidgets/ssltrustprompt_impl.h"
#include "src/ksvnwidgets/commitmsg_impl.h"
#include "src/ksvnwidgets/pwstorage.h"

#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/client.h"
#include "src/svnqt/revision.h"
#include "src/svnqt/status.h"
#include "src/svnqt/url.h"
#include "src/svnqt/svnqttypes.h"
#include "src/svnqt/client_parameter.h"
#include "helpers/ktranslateurl.h"
#include "src/helpers/stringhelper.h"
#include "kdesvndadaptor.h"
#include "ksvnjobview.h"

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kpassworddialog.h>
#include <kaboutdata.h>

#include <kpluginfactory.h>
#include <knotification.h>

#include <QVariant>
#include <QList>
#include <QDBusConnection>

K_PLUGIN_FACTORY(KdeSvndFactory,
                 registerPlugin<kdesvnd>();
                )
K_EXPORT_PLUGIN(KdeSvndFactory("kio_kdesvn"))

#define CHECK_KIO     if (!progressJobView.contains(kioid)) { \
        return;\
    }

kdesvnd::kdesvnd(QObject *parent, const QList<QVariant> &) : KDEDModule(parent), m_componentData("kdesvn"),
    m_uiserver("org.kde.JobViewServer", "/JobViewServer", QDBusConnection::sessionBus())
{
    KGlobal::locale()->insertCatalog("kdesvn");
    m_Listener = new KdesvndListener(this);
    new KdesvndAdaptor(this);
}

kdesvnd::~kdesvnd()
{
    delete m_Listener;
}

QStringList kdesvnd::getTopLevelActionMenu(const KUrl::List &list)
{
    return getActionMenu(list, true);
}

QStringList kdesvnd::getActionMenu(const KUrl::List &list)
{
    return getActionMenu(list, false);
}

QStringList kdesvnd::getActionMenu(const KUrl::List &list, bool toplevel)
{
    QStringList result;
    Kdesvnsettings::self()->readConfig();
    if (Kdesvnsettings::no_konqueror_contextmenu() || list.count() == 0 ||
            (toplevel && Kdesvnsettings::no_konqueror_toplevelmenu())) {
        return result;
    }
    QString base;

    bool itemIsWc = isWorkingCopy(list[0], base);
    bool itemIsRepository = false;

    QString _par = list[0].directory(KUrl::IgnoreTrailingSlash);
    bool parentIsWc = isWorkingCopy(_par, base);

    if (!parentIsWc && !itemIsWc) {
        itemIsRepository = isRepository(list[0]);
    }

    if (!itemIsWc) {
        if (itemIsRepository) {
            result << "Export"
                   << "Checkout";
        } else {
            result << "Exportto"
                   << "Checkoutto";
        }
    } else {
        result << "Update"
               << "Commit";
    }

    if (!parentIsWc && !itemIsWc) {
        if (itemIsRepository) {
            result << "Log";
            if (!toplevel) {
                result << "Info";
                if (isRepository(list[0].upUrl())) {
                    result << "Blame"
                           << "Rename";
                }
                result << "Tree";
            }
        }
    } else if (!toplevel) {
        if (!itemIsWc) {
            result << "Add";
            return result;
        }

        result << "Log"
               << "Tree"
               << "Info"
               << "Diff"
               << "Rename"
               << "Revert";

        KUrl url = helpers::KTranslateUrl::translateSystemUrl(list[0]);

        QFileInfo f(url.path());
        if (f.isFile()) {
            result << "Blame";
        }

        if (f.isDir()) {
            result << "Addnew";
            result << "Switch";
        }
    }
    return result;
}

QStringList kdesvnd::getSingleActionMenu(const QString &what)
{
    KUrl::List l; l.append(KUrl(what));
    return getActionMenu(l);
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
            res.append("true");
        } else {
            res.append("false");
        }
    }
    delete auth;
    return res;
}

int kdesvnd::get_sslaccept(const QString &hostname, const QString &fingerprint, const QString &validFrom, const QString &validUntil, const QString &issuerDName, const QString &realm)
{
    bool ok, saveit;
    if (!SslTrustPrompt_impl::sslTrust(
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
    QPointer<KPasswordDialog> dlg(new KPasswordDialog(0, KPasswordDialog::DomainReadOnly | KPasswordDialog::ShowKeepPassword));
    dlg->setDomain(realm);
    dlg->setCaption(i18n("Enter password for realm %1", realm));
    dlg->setKeepPassword(true);
    if (dlg->exec() == KPasswordDialog::Accepted) {
        resList.append(dlg->password());
        if (dlg->keepPassword()) {
            resList.append("true");
        } else {
            resList.append("false");
        }
    }
    delete dlg;
    return resList;
}

QString kdesvnd::get_sslclientcertfile()
{
    QString afile = KFileDialog::getOpenFileName(KUrl(),
                    QString(),
                    0,
                    i18n("Open a file with a #PKCS12 certificate"));
    return afile;
}

QStringList kdesvnd::get_logmsg()
{
    QStringList res;
    bool ok;
    QString logMessage = Commitmsg_impl::getLogmessage(&ok, 0, 0, 0);
    if (!ok) {
        return res;
    }
    res.append(logMessage);
    return res;
}

QString kdesvnd::cleanUrl(const KUrl &url)
{
    QString cleanpath = url.path(KUrl::RemoveTrailingSlash);
    return cleanpath;
}

/* just simple name check of course - no network access! */
bool kdesvnd::isRepository(const KUrl &url)
{
    QString proto = svn::Url::transformProtokoll(url.protocol());
    if (proto == QLatin1String("file")) {
        // local access - may a repository
        svn::StatusParameter params(svn::Path(QLatin1String("file://") + cleanUrl(url)));
        try {
            m_Listener->m_Svnclient->status(params.depth(svn::DepthEmpty).all(false).update(false).noIgnore(false).revision(svn::Revision::HEAD));
        } catch (const svn::ClientException &e) {
            kDebug(9510) << e.msg() << endl;
            return false;
        }
        return true;
    } else {
        return svn::Url::isValid(proto);
    }
}

bool kdesvnd::isWorkingCopy(const KUrl &_url, QString &base)
{
    base.clear();
    KUrl url = helpers::KTranslateUrl::translateSystemUrl(_url);

    if (url.isEmpty() || !url.isLocalFile() || url.protocol() != "file") {
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
    base = e[0].url();
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
    QString programIconName = m_componentData.aboutData()->programIconName();
    if (programIconName.isEmpty()) {
        programIconName = m_componentData.aboutData()->appName();
    }
    QDBusReply<QDBusObjectPath> reply = m_uiserver.requestView(m_componentData.aboutData()->programName(),
                                        programIconName,
                                        0x0003);
    if (reply.isValid()) {
        KsvnJobView *jobView = new KsvnJobView(kioid, "org.kde.JobViewServer",
                                               reply.value().path(),
                                               QDBusConnection::sessionBus());
        progressJobView.insert(kioid, jobView);
        kDebug() << "Register " << kioid << endl;
    } else {
        kDebug() << "Could not register " << kioid << endl;
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
        progressJobView[kioid]->setProcessedAmount(transferred, "bytes");
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
    kDebug() << "Removed " << kioid << endl;
}

void kdesvnd::notifyKioOperation(const QString &text)
{
    KNotification::event(
        "kdesvn-kio", text,
        QPixmap(), 0L, KNotification::CloseOnTimeout,
        m_componentData);
}

void kdesvnd::errorKioOperation(const QString &text)
{
    KNotification::event(
        KNotification::Error, text,
        QPixmap(), 0L, KNotification::CloseOnTimeout
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
