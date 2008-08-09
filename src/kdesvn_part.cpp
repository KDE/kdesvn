/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "kdesvnview.h"
#include "commandline_part.h"
#include "src/svnqt/version_check.hpp"
#include "src/svnqt/url.hpp"
#include "helpers/ktranslateurl.h"
#include "helpers/sshagent.h"

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
#include <q3popupmenu.h>
#include <ktoolinvocation.h>

K_PLUGIN_FACTORY(KdesvnFactory, registerPlugin<kdesvnPart>();registerPlugin<commandline_part>();)
K_EXPORT_PLUGIN(KdesvnFactory( "kdesvnPart", "kdesvn" ) )

static const char version[] = VERSION;

kdesvnPart::kdesvnPart( QWidget *parentWidget, QObject *parent, const QVariantList& args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget,false);
}

kdesvnPart::kdesvnPart(QWidget *parentWidget, QObject *parent, bool ownapp, const QVariantList& args)
    : KParts::ReadOnlyPart(parent)
{
    Q_UNUSED(args);
    init(parentWidget,ownapp);
}

void kdesvnPart::init( QWidget *parentWidget, bool full)
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
    kDebug()<<"Using test rc file in " << TESTING_PARTRC << endl;
#else
    setXMLFile("kdesvn_part.rc");
#endif
    connect(m_view,SIGNAL(sigShowPopup(const QString&,QWidget**)),this,SLOT(slotDispPopup(const QString&,QWidget**)));
    connect(m_view,SIGNAL(sigSwitchUrl(const KUrl&)),this,SLOT(openURL(const KUrl&)));
    connect(this,SIGNAL(refreshTree()),m_view,SLOT(refreshCurrentTree()));
    connect(m_view,SIGNAL(setWindowCaption(const QString&)),this,SIGNAL(setWindowCaption(const QString&)));
    connect(m_view,SIGNAL(sigUrlChanged( const QString&)),this,SLOT(slotUrlChanged(const QString&)));
    connect(this,SIGNAL(settingsChanged()),widget(),SLOT(slotSettingsChanged()));

    m_browserExt->setPropertiesActionEnabled(false);
}

kdesvnPart::~kdesvnPart()
{
    ///@todo replace with KDE4 like stuff
    //kdesvnPartFactory::instance()->config()->sync();
}

void kdesvnPart::slotUrlChanged(const QString&url)
{
    setUrl(url);
}

bool kdesvnPart::openFile()
{
    m_view->openUrl(url());
    // just for fun, set the status bar
    emit setStatusBarText( url().prettyUrl() );

    return true;
}

bool kdesvnPart::openUrl(const KUrl&aUrl)
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

void kdesvnPart::slotFileProperties()
{
}

void kdesvnPart::slotDispPopup(const QString&name,QWidget**target)
{
    *target = hostContainer(name);
}

KAboutData* kdesvnPart::createAboutData()
{
    KLocalizedString m_Extratext = ki18n("Built with Subversion library: %1\nRunning Subversion library: %2").subs(svn::Version::linked_version()).subs(svn::Version::running_version());

    KAboutData*about = new KAboutData("kdesvnpart",
                                      "kdesvn",
                                      ki18n("kdesvn Part"),
                                      version,
                                      ki18n("A Subversion Client for KDE (dynamic Part component)"),
                                      KAboutData::License_LGPL_V2,
                                      ki18n("(C) 2005-2008 Rajko Albrecht"),
                                      KLocalizedString(),
                                      QByteArray(),
                                      "kdesvn-bugs@alwins-world.de");

    about->addAuthor(ki18n("Rajko Albrecht"), ki18n("Original author and maintainer"), "ral@alwins-world.de" );
    about->setOtherText(m_Extratext);
    about->setHomepage("http://kdesvn.alwins-world.de/");
    about->setBugAddress("kdesvn-bugs@alwins-world.de");
    return about;
}


/*!
    \fn kdesvnPart::setupActions()
 */
void kdesvnPart::setupActions()
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

    t->setText(i18n("&Configure %1...").arg("Kdesvn"));

    if (QString(kapp->applicationName())!=QString("kdesvn")) {
        t = new KAction(KIcon("kdesvn"),i18n("&About kdesvn part"),this);
        connect(t,SIGNAL(triggered(bool) ), SLOT(showAboutApplication()));
        actionCollection()->addAction("help_about_kdesvnpart",t);

        t = new KAction(KIcon("help"),i18n("Kdesvn &Handbook"), this);
        connect(t,SIGNAL(triggered(bool) ), SLOT(appHelpActivated()));
        actionCollection()->addAction("help_kdesvn",t);

        t = new KAction(i18n("Send Bugreport for kdesvn"), this);
        connect(t,SIGNAL(triggered(bool) ), SLOT(reportBug()));
        actionCollection()->addAction("report_bug",t);
    }
}

void kdesvnPart::slotSshAdd()
{
    SshAgent ag;
    ag.addSshIdentities(true);
}

/*!
    \fn kdesvnPart::slotLogFollowNodes(bool)
 */
void kdesvnPart::slotLogFollowNodes(bool how)
{
    Kdesvnsettings::setLog_follows_nodes(how);
    Kdesvnsettings::self()->writeConfig();
}


/*!
    \fn kdesvnPart::slotDiplayIgnored(bool)
 */
void kdesvnPart::slotDisplayIgnored(bool how)
{
    Kdesvnsettings::setDisplay_ignored_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit refreshTree();
}


/*!
    \fn kdesvnPart::slotDisplayUnknown(bool)
 */
void kdesvnPart::slotDisplayUnkown(bool how)
{
    Kdesvnsettings::setDisplay_unknown_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit refreshTree();
}

/*!
    \fn kdesvnPart::slotHideUnchanged(bool)
 */
void kdesvnPart::slotHideUnchanged(bool how)
{
    Kdesvnsettings::setHide_unchanged_files(how);
    Kdesvnsettings::self()->writeConfig();
    emit refreshTree();
}

void kdesvnPart::slotEnableNetwork(bool how)
{
    Kdesvnsettings::setNetwork_on(how);
    Kdesvnsettings::self()->writeConfig();
    emit settingsChanged();
}

/*!
    \fn kdesvnPart::closeURL()
 */
bool kdesvnPart::closeUrl()
{
    KParts::ReadOnlyPart::closeUrl();
    setUrl(KUrl());
    m_view->closeMe();
    emit setWindowCaption("");
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension( kdesvnPart *p )
    : KParts::BrowserExtension(p)
{
    KGlobal::locale()->insertCatalog("kdesvn");
}

KdesvnBrowserExtension::~KdesvnBrowserExtension()
{

}


void KdesvnBrowserExtension::setPropertiesActionEnabled(bool enabled)
{
    emit enableAction("properties", enabled);
}


void KdesvnBrowserExtension::properties()
{
    static_cast<kdesvnPart*>(parent())->slotFileProperties();
}


#if KDE_VERSION_MAJOR<4
/*!
    \fn kdesvnPart::reportBug()
 */
void kdesvnPart::reportBug()
{
  KBugReport dlg(m_view, true, createAboutData());
  dlg.exec();
}
#endif

/*!
    \fn kdesvnPart::showAboutApplication()
 */
void kdesvnPart::showAboutApplication()
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
    \fn kdesvnPart::appHelpActivated()
 */
void kdesvnPart::appHelpActivated()
{
    KToolInvocation::invokeHelp(QString::null, "kdesvn");
}


/*!
    \fn kdesvnPart::slotShowSettings()
 */
void kdesvnPart::slotShowSettings()
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
        i18n("General"),"configure",i18n("General"),true);
    dialog->addPage(new SubversionSettings_impl(0),
        i18n("Subversion"),"kdesvn",i18n("Subversion Settings"),true);
    dialog->addPage(new DiffMergeSettings_impl(0),
        i18n("Diff & Merge"),"kdesvnmerge",i18n("Settings for diff and merge"),true);
    dialog->addPage(new DispColorSettings_impl(0),
        i18n("Colors"),"colorize",i18n("Color Settings"),true);
    dialog->addPage(new RevisiontreeSettingsDlg_impl(0),
        i18n("Revision tree"),"configure",i18n("Revision tree Settings"),true);
    dialog->addPage(new CmdExecSettings_impl(0),
        "KIO/"+i18n("Commandline"),"terminal",i18n("Settings for commandline and KIO execution"),true);

    connect(dialog,SIGNAL(settingsChanged()),this,SLOT(slotSettingsChanged()));
    dialog->show();
}


/*!
    \fn kdesvnPart::slotSettingsChanged()
 */
void kdesvnPart::slotSettingsChanged()
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
        ((KToggleAction*)temp)->setChecked(kdesvnPart_Prefs::self()->mdisp_unknown_files);
    }
#endif
    emit settingsChanged();
}

#if KDE_VERSION_MAJOR < 4
/*
 * we may not use generic factory 'cause we make some specials */
// KInstance*  cFactory::s_instance = 0L;
KComponentData* cFactory::s_instance = 0L;
KAboutData* cFactory::s_about = 0L;
commandline_part* cFactory::s_cline = 0L;

KParts::Part* cFactory::createPartObject( QWidget *parentWidget,
                                                        QObject *parent,
                                                        const char *classname, const QStringList &args )
{
    Q_UNUSED(classname);
    // Create an instance of our Part
    return new kdesvnPart( parentWidget, parent, args );
}

KParts::Part* cFactory::createAppPart( QWidget *parentWidget, const char *widgetName,
                                          QObject *parent, const char *name,
                                          const char *classname, const QStringList &args )
{
    Q_UNUSED(classname);
    // Create an instance of our Part
    kdesvnPart* obj = new kdesvnPart( parentWidget, widgetName, parent, name, false, args);
    emit objectCreated( obj );
    return obj;
}

cFactory::~cFactory()
{
    delete s_instance;
    delete s_about;
    delete s_cline;

    s_instance = 0L;
    s_cline = 0L;
}

// KInstance* cFactory::instance()
KComponentData* cFactory::instance()
{
    if( !s_instance ) {
        s_about = kdesvnPart::createAboutData();
//         s_instance = new KInstance(s_about);
        s_instance = new KComponentData(s_about);
    }
    return s_instance;
}

commandline_part*cFactory::createCommandIf(QObject*parent,const char*name, KCmdLineArgs *args)
{
    if (!s_cline) {
        // no emit of creation - we will delete this object in destructor
        s_cline = new commandline_part(parent,name,args);
    }
    return s_cline;
}
#endif

#include "kdesvn_part.moc"
