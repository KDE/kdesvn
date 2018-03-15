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

#include "kdesvn_part.h"
#include "settings/kdesvnsettings.h"
#include "settings/displaysettings_impl.h"
#include "settings/dispcolorsettings_impl.h"
#include "settings/revisiontreesettingsdlg_impl.h"
#include "settings/diffmergesettings_impl.h"
#include "settings/subversionsettings_impl.h"
#include "settings/cmdexecsettings_impl.h"
#include "settings/polling_settings_impl.h"
#include "kdesvnview.h"
#include "commandline_part.h"
#include "svnqt/version_check.h"
#include "svnqt/url.h"
#include "helpers/kdesvn_debug.h"
#include "helpers/sshagent.h"
#include "svnfrontend/database/dboverview.h"

#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>
#include <kaboutapplicationdialog.h>
#include <kconfigdialog.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <khelpclient.h>
#include <kpluginfactory.h>

K_PLUGIN_FACTORY(KdesvnFactory, registerPlugin<kdesvnpart>(); registerPlugin<commandline_part>(QStringLiteral("commandline_part"));)

static const char version[] = KDESVN_VERSION;

kdesvnpart::kdesvnpart(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget, false);
}

kdesvnpart::kdesvnpart(QWidget *parentWidget, QObject *parent, bool ownapp, const QVariantList &args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget, ownapp);
}

void kdesvnpart::init(QWidget *parentWidget, bool full)
{
    m_aboutDlg = nullptr;
    // we need an instance
    // TODO: KF5 port
    //setComponentData(KdesvnFactory::componentData());

    m_browserExt = new KdesvnBrowserExtension(this);

    // this should be your custom internal widget
    m_view = new kdesvnView(actionCollection(), parentWidget, full);

    // notify the part that this is our internal widget
    setWidget(m_view);

    // create our actions
    setupActions();
    // set our XML-UI resource file
#ifdef TESTING_PARTRC
    setXMLFile(TESTING_PARTRC);
    qCDebug(KDESVN_LOG) << "Using test rc file in " << TESTING_PARTRC << endl;
#else
    setXMLFile(QStringLiteral("kdesvn_part.rc"));
#endif
    connect(m_view, SIGNAL(sigShowPopup(QString,QWidget**)), this, SLOT(slotDispPopup(QString,QWidget**)));
    connect(m_view, SIGNAL(sigSwitchUrl(QUrl)), this, SLOT(openUrl(QUrl)));
    connect(this, SIGNAL(refreshTree()), m_view, SLOT(refreshCurrentTree()));
    connect(m_view, SIGNAL(setWindowCaption(QString)), this, SIGNAL(setWindowCaption(QString)));
    connect(m_view, &kdesvnView::sigUrlChanged, this, &kdesvnpart::slotUrlChanged);
    connect(this, SIGNAL(settingsChanged()), widget(), SLOT(slotSettingsChanged()));
    SshAgent ssh;
    ssh.querySshAgent();
}

kdesvnpart::~kdesvnpart()
{
    ///@todo replace with KDE4 like stuff
    //kdesvnpartFactory::instance()->config()->sync();
}

void kdesvnpart::slotUrlChanged(const QUrl &url)
{
    setUrl(url);
}

bool kdesvnpart::openFile()
{
    m_view->openUrl(url());
    // just for fun, set the status bar
    emit setStatusBarText(url().toString());

    return true;
}

bool kdesvnpart::openUrl(const QUrl &aUrl)
{
    QUrl _url(aUrl);

    _url.setScheme(svn::Url::transformProtokoll(_url.scheme()));

    if (!_url.isValid() || !closeUrl()) {
        return false;
    }
    setUrl(_url);
    emit started(nullptr);
    bool ret = m_view->openUrl(url());
    if (ret) {
        emit completed();
        emit setWindowCaption(url().toString());
    }
    return ret;
}

void kdesvnpart::slotFileProperties()
{
}

void kdesvnpart::slotDispPopup(const QString &name, QWidget **target)
{
    *target = hostContainer(name);
}

/*!
    \fn kdesvnpart::setupActions()
 */
void kdesvnpart::setupActions()
{
    KToggleAction *toggletemp;

    toggletemp = new KToggleAction(i18n("Logs follow node changes"), this);
    actionCollection()->addAction(QStringLiteral("toggle_log_follows"), toggletemp);
    toggletemp->setChecked(Kdesvnsettings::log_follows_nodes());
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotLogFollowNodes(bool)));

    toggletemp = new KToggleAction(i18n("Display ignored files"), this);
    actionCollection()->addAction(QStringLiteral("toggle_ignored_files"), toggletemp);
    toggletemp->setChecked(Kdesvnsettings::display_ignored_files());
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotDisplayIgnored(bool)));

    toggletemp = new KToggleAction(i18n("Display unknown files"), this);
    actionCollection()->addAction(QStringLiteral("toggle_unknown_files"), toggletemp);
    toggletemp->setChecked(Kdesvnsettings::display_unknown_files());
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotDisplayUnkown(bool)));

    toggletemp = new KToggleAction(i18n("Hide unchanged files"), this);
    actionCollection()->addAction(QStringLiteral("toggle_hide_unchanged_files"), toggletemp);
    toggletemp->setChecked(Kdesvnsettings::hide_unchanged_files());
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotHideUnchanged(bool)));

    toggletemp = new KToggleAction(i18n("Work online"), this);
    actionCollection()->addAction(QStringLiteral("toggle_network"), toggletemp);
    toggletemp->setChecked(Kdesvnsettings::network_on());
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotEnableNetwork(bool)));

    QAction *t = KStandardAction::preferences(this, SLOT(slotShowSettings()), actionCollection());

    t->setText(i18n("Configure Kdesvn..."));
    actionCollection()->addAction(QStringLiteral("kdesvnpart_pref"), t);

    if (QCoreApplication::applicationName() != QLatin1String("kdesvn")) {
        t = new QAction(QIcon::fromTheme(QStringLiteral("kdesvn")), i18n("About kdesvn part"), this);
        connect(t, SIGNAL(triggered(bool)), SLOT(showAboutApplication()));
        actionCollection()->addAction(QStringLiteral("help_about_kdesvnpart"), t);

        t = new QAction(QIcon::fromTheme(QStringLiteral("help-contents")), i18n("Kdesvn Handbook"), this);
        connect(t, SIGNAL(triggered(bool)), SLOT(appHelpActivated()));
        actionCollection()->addAction(QStringLiteral("help_kdesvn"), t);
    }
}

void kdesvnpart::slotSshAdd()
{
    SshAgent ag;
    ag.addSshIdentities(true);
}

/*!
    \fn kdesvnpart::slotLogFollowNodes(bool)
 */
void kdesvnpart::slotLogFollowNodes(bool how)
{
    Kdesvnsettings::setLog_follows_nodes(how);
    Kdesvnsettings::self()->save();
}

/*!
    \fn kdesvnpart::slotDiplayIgnored(bool)
 */
void kdesvnpart::slotDisplayIgnored(bool how)
{
    Kdesvnsettings::setDisplay_ignored_files(how);
    Kdesvnsettings::self()->save();
    emit settingsChanged();
}

/*!
    \fn kdesvnpart::slotDisplayUnknown(bool)
 */
void kdesvnpart::slotDisplayUnkown(bool how)
{
    Kdesvnsettings::setDisplay_unknown_files(how);
    Kdesvnsettings::self()->save();
    emit settingsChanged();
}

/*!
    \fn kdesvnpart::slotHideUnchanged(bool)
 */
void kdesvnpart::slotHideUnchanged(bool how)
{
    Kdesvnsettings::setHide_unchanged_files(how);
    Kdesvnsettings::self()->save();
    emit settingsChanged();
}

void kdesvnpart::slotEnableNetwork(bool how)
{
    Kdesvnsettings::setNetwork_on(how);
    Kdesvnsettings::self()->save();
    emit settingsChanged();
}

/*!
    \fn kdesvnpart::closeURL()
 */
bool kdesvnpart::closeUrl()
{
    KParts::ReadOnlyPart::closeUrl();
    setUrl(QUrl());
    m_view->closeMe();
    emit setWindowCaption(QString());
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension(kdesvnpart *p)
    : KParts::BrowserExtension(p)
{}

KdesvnBrowserExtension::~KdesvnBrowserExtension()
{

}

void KdesvnBrowserExtension::properties()
{
    static_cast<kdesvnpart *>(parent())->slotFileProperties();
}

/*!
    \fn kdesvnpart::showAboutApplication()
 */
void kdesvnpart::showAboutApplication()
{
    if (!m_aboutDlg) {
        QString m_Extratext = i18n("Built with Subversion library: %1\nRunning Subversion library: %2", svn::Version::linked_version(), svn::Version::running_version());

        KAboutData about(QStringLiteral("kdesvnpart"),
                         i18n("kdesvn Part"),
                         QLatin1String(version),
                         i18n("A Subversion Client by KDE (dynamic Part component)"),
                         KAboutLicense::LGPL_V2,
                         i18n("(C) 2005-2009 Rajko Albrecht,\n(C) 2015-2016 Christian Ehrlicher"),
                         m_Extratext);

        about.addAuthor(QStringLiteral("Rajko Albrecht"), i18n("Original author and maintainer"), QStringLiteral("ral@alwins-world.de"));
        about.addAuthor(QStringLiteral("Christian Ehrlicher"), i18n("Developer"), QStringLiteral("ch.ehrlicher@gmx.de"));
        about.setHomepage(QStringLiteral("https://commits.kde.org/kdesvn"));
        qApp->setWindowIcon(QIcon::fromTheme(QStringLiteral("kdesvn"), qApp->windowIcon()));
        m_aboutDlg = new KAboutApplicationDialog(about);
    }
    if (m_aboutDlg == nullptr) {
        return;
    }
    if (!m_aboutDlg->isVisible()) {
        m_aboutDlg->show();
    } else {
        m_aboutDlg->raise();
    }
}

/*!
    \fn kdesvnpart::appHelpActivated()
 */
void kdesvnpart::appHelpActivated()
{
    KHelpClient::invokeHelp(QString(), QStringLiteral("kdesvn"));
}

/*!
    \fn kdesvnpart::slotShowSettings()
 */
void kdesvnpart::slotShowSettings()
{
    if (KConfigDialog::showDialog(QStringLiteral("kdesvnpart_settings"))) {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(widget(),
            QStringLiteral("kdesvnpart_settings"),
            Kdesvnsettings::self());
    dialog->setFaceType(KPageDialog::List);

    // TODO: KF5
    //dialog->setHelp("setup", "kdesvn");
    dialog->addPage(new DisplaySettings_impl(nullptr),
                    i18n("General"), QStringLiteral("configure"), i18n("General Settings"), true);
    dialog->addPage(new SubversionSettings_impl(nullptr),
                    i18n("Subversion"), QStringLiteral("kdesvn"), i18n("Subversion Settings"), true);
    dialog->addPage(new PollingSettings_impl(nullptr),
                    i18n("Timed jobs"), QStringLiteral("kdesvnclock"), i18n("Settings for timed jobs"), true);
    dialog->addPage(new DiffMergeSettings_impl(nullptr),
                    i18n("Diff & Merge"), QStringLiteral("kdesvnmerge"), i18n("Settings for diff and merge"), true);
    dialog->addPage(new DispColorSettings_impl(nullptr),
                    i18n("Colors"), QStringLiteral("kdesvncolors"), i18n("Color Settings"), true);
    dialog->addPage(new RevisiontreeSettingsDlg_impl(nullptr),
                    i18n("Revision tree"), QStringLiteral("kdesvntree"), i18n("Revision tree Settings"), true);
    dialog->addPage(new CmdExecSettings_impl(nullptr),
                    i18n("KIO / Command line"), QStringLiteral("kdesvnterminal"), i18n("Settings for command line and KIO execution"), true);

    connect(dialog, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged(QString)));
    dialog->show();
}

/*!
    \fn kdesvnpart::slotSettingsChanged()
 */
void kdesvnpart::slotSettingsChanged(const QString &)
{
    QAction *temp;
    temp = actionCollection()->action(QStringLiteral("toggle_log_follows"));
    if (temp) {
        temp->setChecked(Kdesvnsettings::log_follows_nodes());
    }
    temp = actionCollection()->action(QStringLiteral("toggle_ignored_files"));
    if (temp) {
        temp->setChecked(Kdesvnsettings::display_ignored_files());
    }
#if 0
    /// not needed this momenta
    temp = actionCollection()->action("toggle_unknown_files");
    if (temp) {
        ((KToggleAction *)temp)->setChecked(kdesvnpart_Prefs::self()->mdisp_unknown_files);
    }
#endif
    emit settingsChanged();
}

void kdesvnpart::showDbStatus()
{
    if (m_view) {
        m_view->stopCacheThreads();
        DbOverview::showDbOverview(svn::ClientP());
    }
}

#include "kdesvn_part.moc"
