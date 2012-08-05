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
#include "src/settings/kdesvnsettings.h"
#include "settings/displaysettings_impl.h"
#include "settings/dispcolorsettings_impl.h"
#include "settings/revisiontreesettingsdlg_impl.h"
#include "settings/diffmergesettings_impl.h"
#include "settings/subversionsettings_impl.h"
#include "settings/cmdexecsettings_impl.h"
#include "settings/polling_settings_impl.h"
#include "kdesvnview.h"
#include "commandline_part.h"
#include "src/svnqt/version_check.h"
#include "src/svnqt/url.h"
#include "helpers/ktranslateurl.h"
#include "helpers/sshagent.h"
#include "svnfrontend/database/dboverview.h"

#include <kcomponentdata.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kbugreport.h>
#include <kxmlguifactory.h>
#include <kaboutapplicationdialog.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <klocalizedstring.h>

#include <qcursor.h>
#include <ktoolinvocation.h>

K_PLUGIN_FACTORY(KdesvnFactory, registerPlugin<kdesvnpart>();registerPlugin<commandline_part>("commandline_part");)
K_EXPORT_PLUGIN(KdesvnFactory("kdesvnpart","kdesvn"))

static const char version[] = VERSION;

kdesvnpart::kdesvnpart( QWidget *parentWidget, QObject *parent, const QVariantList& args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget,false);
}

kdesvnpart::kdesvnpart(QWidget *parentWidget, QObject *parent, bool ownapp, const QVariantList& args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget,ownapp);
}

void kdesvnpart::init( QWidget *parentWidget, bool full)
{
    m_aboutDlg = 0;
    KGlobal::locale()->insertCatalog("kdesvn");
    // we need an instance
    setComponentData( KdesvnFactory::componentData() );

    m_browserExt = new KdesvnBrowserExtension( this );

    // this should be your custom internal widget
    m_view = new kdesvnView(actionCollection(),parentWidget,full);

    // notify the part that this is our internal widget
    setWidget(m_view);

    // create our actions
    setupActions();
    // set our XML-UI resource file
#ifdef TESTING_PARTRC
    setXMLFile(TESTING_PARTRC);
    kDebug(9510)<<"Using test rc file in " << TESTING_PARTRC << endl;
#else
    setXMLFile("kdesvn_part.rc");
#endif
    connect(m_view,SIGNAL(sigShowPopup(const QString&,QWidget**)),this,SLOT(slotDispPopup(const QString&,QWidget**)));
    connect(m_view,SIGNAL(sigSwitchUrl(const KUrl&)),this,SLOT(openUrl(const KUrl&)));
    connect(this,SIGNAL(refreshTree()),m_view,SLOT(refreshCurrentTree()));
    connect(m_view,SIGNAL(setWindowCaption(const QString&)),this,SIGNAL(setWindowCaption(const QString&)));
    connect(m_view,SIGNAL(sigUrlChanged( const QString&)),this,SLOT(slotUrlChanged(const QString&)));
    connect(this,SIGNAL(settingsChanged()),widget(),SLOT(slotSettingsChanged()));
    SshAgent ssh;
    ssh.querySshAgent();
}

kdesvnpart::~kdesvnpart()
{
    ///@todo replace with KDE4 like stuff
    //kdesvnpartFactory::instance()->config()->sync();
}

void kdesvnpart::slotUrlChanged(const QString&url)
{
    setUrl(url);
}

bool kdesvnpart::openFile()
{
    m_view->openUrl(url());
    // just for fun, set the status bar
    emit setStatusBarText( url().prettyUrl() );

    return true;
}

bool kdesvnpart::openUrl(const KUrl&aUrl)
{

    KUrl _url = helpers::KTranslateUrl::translateSystemUrl(aUrl);

    _url.setProtocol(svn::Url::transformProtokoll(_url.protocol()));

    if (!_url.isValid()||!closeUrl()) {
        return false;
    }
    setUrl(_url);
    emit started(0);
    bool ret = m_view->openUrl(url());
    if (ret) {
        emit completed();
        emit setWindowCaption(url().prettyUrl());
    }
    return ret;
}

void kdesvnpart::slotFileProperties()
{
}

void kdesvnpart::slotDispPopup(const QString&name,QWidget**target)
{
    *target = hostContainer(name);
}

KAboutData* kdesvnpart::createAboutData()
{
    static KLocalizedString m_Extratext = ki18n("Built with Subversion library: %1\nRunning Subversion library: %2").subs(svn::Version::linked_version()).subs(svn::Version::running_version());

    static KAboutData about("kdesvnpart",
                                      "kdesvn",
                                      ki18n("kdesvn Part"),
                                      version,
                                      ki18n("A Subversion Client for KDE (dynamic Part component)"),
                                      KAboutData::License_LGPL_V2,
                                      ki18n("(C) 2005-2009 Rajko Albrecht"),
                                      KLocalizedString(),
                                      QByteArray(),
                                      "kdesvn-bugs@alwins-world.de");

    about.addAuthor(ki18n("Rajko Albrecht"), ki18n("Original author and maintainer"), "ral@alwins-world.de" );
    about.setOtherText(m_Extratext);
    about.setHomepage("http://kdesvn.alwins-world.de/");
    about.setBugAddress("kdesvn-bugs@alwins-world.de");
    about.setProgramIconName("kdesvn");
    about.setTranslator(ki18n("kdesvn: NAME OF TRANSLATORS\\nYour names"),
        ki18n("kdesvn: EMAIL OF TRANSLATORS\\nYour emails"));
    return &about;
}


/*!
    \fn kdesvnpart::setupActions()
 */
void kdesvnpart::setupActions()
{
    KToggleAction *toggletemp;

    toggletemp = new KToggleAction(i18n("Logs follow node changes"),this);
    actionCollection()->addAction("toggle_log_follows",toggletemp);
    toggletemp->setChecked(Kdesvnsettings::log_follows_nodes());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotLogFollowNodes(bool)));

    toggletemp = new KToggleAction(i18n("Display ignored files"),this);
    actionCollection()->addAction("toggle_ignored_files",toggletemp);
    toggletemp->setChecked(Kdesvnsettings::display_ignored_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayIgnored(bool)));


    toggletemp = new KToggleAction(i18n("Display unknown files"),this);
    actionCollection()->addAction("toggle_unknown_files",toggletemp);
    toggletemp->setChecked(Kdesvnsettings::display_unknown_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayUnkown(bool)));

    toggletemp = new KToggleAction(i18n("Hide unchanged files"),this);
    actionCollection()->addAction("toggle_hide_unchanged_files",toggletemp);
    toggletemp->setChecked(Kdesvnsettings::hide_unchanged_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotHideUnchanged(bool)));

    toggletemp = new KToggleAction(i18n("Work online"),this);
    actionCollection()->addAction("toggle_network",toggletemp);
    toggletemp->setChecked(Kdesvnsettings::network_on());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotEnableNetwork(bool)));


    KAction * t = KStandardAction::preferences(this, SLOT(slotShowSettings()), actionCollection());

    t->setText(i18n("Configure %1...",QString::fromLatin1("Kdesvn")));
    actionCollection()->addAction("kdesvnpart_pref",t);

    if (QString(kapp->applicationName())!=QString("kdesvn")) {
        t = new KAction(KIcon("kdesvn"),i18n("About kdesvn part"),this);
        connect(t,SIGNAL(triggered(bool) ), SLOT(showAboutApplication()));
        actionCollection()->addAction("help_about_kdesvnpart",t);

        t = new KAction(KIcon("help-contents"),i18n("Kdesvn Handbook"), this);
        connect(t,SIGNAL(triggered(bool) ), SLOT(appHelpActivated()));
        actionCollection()->addAction("help_kdesvn",t);
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
    Kdesvnsettings::self()->writeConfig();
}


/*!
    \fn kdesvnpart::slotDiplayIgnored(bool)
 */
void kdesvnpart::slotDisplayIgnored(bool how)
{
    Kdesvnsettings::setDisplay_ignored_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit settingsChanged();
}


/*!
    \fn kdesvnpart::slotDisplayUnknown(bool)
 */
void kdesvnpart::slotDisplayUnkown(bool how)
{
    Kdesvnsettings::setDisplay_unknown_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit settingsChanged();
}

/*!
    \fn kdesvnpart::slotHideUnchanged(bool)
 */
void kdesvnpart::slotHideUnchanged(bool how)
{
    Kdesvnsettings::setHide_unchanged_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit settingsChanged();
}

void kdesvnpart::slotEnableNetwork(bool how)
{
    Kdesvnsettings::setNetwork_on(how);
    Kdesvnsettings::self()->writeConfig();
    emit settingsChanged();
}

/*!
    \fn kdesvnpart::closeURL()
 */
bool kdesvnpart::closeUrl()
{
    KParts::ReadOnlyPart::closeUrl();
    setUrl(KUrl());
    m_view->closeMe();
    emit setWindowCaption("");
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension( kdesvnpart *p )
    : KParts::BrowserExtension(p)
{
    KGlobal::locale()->insertCatalog("kdesvn");
}

KdesvnBrowserExtension::~KdesvnBrowserExtension()
{

}

void KdesvnBrowserExtension::properties()
{
    static_cast<kdesvnpart*>(parent())->slotFileProperties();
}

/*!
    \fn kdesvnpart::showAboutApplication()
 */
void kdesvnpart::showAboutApplication()
{
    if (!m_aboutDlg) m_aboutDlg = new KAboutApplicationDialog(createAboutData(), (QWidget *)0);
    if(m_aboutDlg == 0)
        return;
    if(!m_aboutDlg->isVisible())
        m_aboutDlg->show();
    else
        m_aboutDlg->raise();
}

/*!
    \fn kdesvnpart::appHelpActivated()
 */
void kdesvnpart::appHelpActivated()
{
    KToolInvocation::invokeHelp(QString(), "kdesvn");
}


/*!
    \fn kdesvnpart::slotShowSettings()
 */
void kdesvnpart::slotShowSettings()
{
    if (KConfigDialog::showDialog("kdesvnpart_settings")) {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(widget(),
         "kdesvnpart_settings",
         Kdesvnsettings::self());
    dialog->setFaceType(KPageDialog::List);

    dialog->setHelp("setup","kdesvn");
    dialog->addPage(new DisplaySettings_impl(0),
        i18n("General"),"configure",i18n("General Settings"),true);
    dialog->addPage(new SubversionSettings_impl(0),
        i18n("Subversion"),"kdesvn",i18n("Subversion Settings"),true);
    dialog->addPage(new PollingSettings_impl(0),
                    i18n("Timed jobs"),"kdesvnclock",i18n("Settings for timed jobs"),true);
    dialog->addPage(new DiffMergeSettings_impl(0),
        i18n("Diff & Merge"),"kdesvnmerge",i18n("Settings for diff and merge"),true);
    dialog->addPage(new DispColorSettings_impl(0),
        i18n("Colors"),"kdesvncolors",i18n("Color Settings"),true);
    dialog->addPage(new RevisiontreeSettingsDlg_impl(0),
                     i18n("Revision tree"),"kdesvntree",i18n("Revision tree Settings"),true);
    dialog->addPage(new CmdExecSettings_impl(0),
        "KIO/"+i18n("Commandline"),"kdesvnterminal",i18n("Settings for commandline and KIO execution"),true);

    connect(dialog,SIGNAL(settingsChanged(const QString&)),this,SLOT(slotSettingsChanged(const QString&)));
    dialog->show();
}


/*!
    \fn kdesvnpart::slotSettingsChanged()
 */
void kdesvnpart::slotSettingsChanged(const QString&)
{
    QAction * temp;
    temp = actionCollection()->action("toggle_log_follows");
    if (temp) {
        temp->setChecked(Kdesvnsettings::log_follows_nodes());
    }
    temp = actionCollection()->action("toggle_ignored_files");
    if (temp) {
        temp->setChecked(Kdesvnsettings::display_ignored_files());
    }
#if 0
    /// not needed this momenta
    temp = actionCollection()->action("toggle_unknown_files");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(kdesvnpart_Prefs::self()->mdisp_unknown_files);
    }
#endif
    emit settingsChanged();
}

void kdesvnpart::showDbStatus()
{
    if (m_view) {
        m_view->stopCacheThreads();
        DbOverview::showDbOverview();
    }
}

#include "kdesvn_part.moc"
