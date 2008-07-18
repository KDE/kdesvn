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

#include <kinstance.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kbugreport.h>
#include <kxmlguifactory.h>
#include <k3aboutapplication.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <qcursor.h>
#include <q3popupmenu.h>
#include <ktoolinvocation.h>

//K_EXPORT_COMPONENT_FACTORY( libkdesvnpart, kdesvnPartFactory )

extern "C" { KDESVN_EXPORT void *init_libkdesvnpart() { return new kdesvnPartFactory; } }

static const char version[] = VERSION;
QString kdesvnPart::m_Extratext = "";

static const char description[] =
    I18N_NOOP("A Subversion Client for KDE (dynamic Part component)");

kdesvnPart::kdesvnPart( QWidget *parentWidget, const char *widgetName,
                                  QObject *parent, const char *name , const QStringList&)
    : KParts::ReadOnlyPart(parent, name)
{
    init(parentWidget,widgetName,false);
}

kdesvnPart::kdesvnPart(QWidget *parentWidget, const char *widgetName,
               QObject *parent, const char *name,bool ownapp, const QStringList&)
    : KParts::ReadOnlyPart(parent, name)
{
    init(parentWidget,widgetName,ownapp);
}

void kdesvnPart::init( QWidget *parentWidget, const char *widgetName,bool full)
{
    m_aboutDlg = 0;
    KGlobal::locale()->insertCatalog("kdesvn");
    // we need an instance
    setInstance( kdesvnPartFactory::instance() );
    m_browserExt = new KdesvnBrowserExtension( this );

    // this should be your custom internal widget
    m_view = new kdesvnView(actionCollection(),parentWidget,widgetName,full);

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
    kdesvnPartFactory::instance()->config()->sync();
}

void kdesvnPart::slotUrlChanged(const QString&url)
{
    m_url = url;
}

bool kdesvnPart::openFile()
{
    m_view->openURL(m_url);
    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyUrl() );

    return true;
}

bool kdesvnPart::openURL(const KUrl&url)
{

    KUrl _url = helpers::KTranslateUrl::translateSystemUrl(url);

    _url.setProtocol(svn::Url::transformProtokoll(_url.protocol()));

    if (!_url.isValid()||!closeURL()) {
        return false;
    }
    m_url = _url;
    emit started(0);
    bool ret = m_view->openURL(m_url);
    if (ret) {
        emit completed();
        emit setWindowCaption(url.prettyUrl());
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
    m_Extratext = QString(I18N_NOOP("Built with Subversion library: %1\n")).arg(svn::Version::linked_version());
    m_Extratext+=QString(I18N_NOOP("Running Subversion library: %1")).arg(svn::Version::running_version());

    KAboutData*about = new KAboutData("kdesvnpart", I18N_NOOP("kdesvn Part"), version, description,
                     KAboutData::License_GPL, "(C) 2005-2007 Rajko Albrecht",0,
                         0, "ral@alwins-world.de");
    about->addAuthor( "Rajko Albrecht", 0, "ral@alwins-world.de" );
    about->setOtherText(m_Extratext);
    about->setHomepage("http://kdesvn.alwins-world.de/");
    about->setBugAddress("kdesvn-bugs@alwins-world.de");
    about->setTranslator(I18N_NOOP("kdesvn: NAME OF TRANSLATORS\\nYour names"),
        I18N_NOOP("kdesvn: EMAIL OF TRANSLATORS\\nYour emails"));
    return about;
}


/*!
    \fn kdesvnPart::setupActions()
 */
void kdesvnPart::setupActions()
{
    KToggleAction *toggletemp;

    toggletemp = new KToggleAction(i18n("Logs follow node changes"),KShortcut(),
            actionCollection(),"toggle_log_follows");
    toggletemp->setChecked(Kdesvnsettings::log_follows_nodes());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotLogFollowNodes(bool)));

    toggletemp = new KToggleAction(i18n("Display ignored files"),KShortcut(),
            actionCollection(),"toggle_ignored_files");
    toggletemp->setChecked(Kdesvnsettings::display_ignored_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayIgnored(bool)));


    toggletemp = new KToggleAction(i18n("Display unknown files"),KShortcut(),
            actionCollection(),"toggle_unknown_files");
    toggletemp->setChecked(Kdesvnsettings::display_unknown_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayUnkown(bool)));

    toggletemp = new KToggleAction(i18n("Hide unchanged files"),KShortcut(),
                                   actionCollection(),"toggle_hide_unchanged_files");
    toggletemp->setChecked(Kdesvnsettings::hide_unchanged_files());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotHideUnchanged(bool)));

    kDebug()<<"Appname = " << (QString)kapp->instanceName() << endl;

    toggletemp = new KToggleAction(i18n("Work online"),KShortcut(),
                                   actionCollection(),"toggle_network");
    toggletemp->setChecked(Kdesvnsettings::network_on());
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotEnableNetwork(bool)));


    KAction * t = KStandardAction::preferences(this, SLOT(slotShowSettings()), actionCollection(),"kdesvnpart_pref");
    t->setText(i18n("&Configure %1...").arg("Kdesvn"));
    if (QString(kapp->instanceName())!=QString("kdesvn")) {
        (void)new KAction(i18n("&About kdesvn part"), "kdesvn", 0, this, SLOT(showAboutApplication()), actionCollection(), "help_about_kdesvnpart");
        (void)new KAction(i18n("Kdesvn &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kdesvn");
        (void)new KAction(i18n("Send Bugreport for kdesvn"), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
    }
    actionCollection()->setHighlightingEnabled(true);
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
    Kdesvnsettings::writeConfig();
}


/*!
    \fn kdesvnPart::slotDiplayIgnored(bool)
 */
void kdesvnPart::slotDisplayIgnored(bool how)
{
    Kdesvnsettings::setDisplay_ignored_files(how);
    Kdesvnsettings::writeConfig();
    emit refreshTree();
}


/*!
    \fn kdesvnPart::slotDisplayUnknown(bool)
 */
void kdesvnPart::slotDisplayUnkown(bool how)
{
    Kdesvnsettings::setDisplay_unknown_files(how);
    Kdesvnsettings::writeConfig();
    emit refreshTree();
}

/*!
    \fn kdesvnPart::slotHideUnchanged(bool)
 */
void kdesvnPart::slotHideUnchanged(bool how)
{
    Kdesvnsettings::setHide_unchanged_files(how);
    Kdesvnsettings::writeConfig();
    emit refreshTree();
}

void kdesvnPart::slotEnableNetwork(bool how)
{
    Kdesvnsettings::setNetwork_on(how);
    Kdesvnsettings::writeConfig();
    emit settingsChanged();
}

/*!
    \fn kdesvnPart::closeURL()
 */
bool kdesvnPart::closeURL()
{
    m_url=KUrl();
    m_view->closeMe();
    emit setWindowCaption("");
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension( kdesvnPart *p )
    : KParts::BrowserExtension( p, "KdesvnBrowserExtension" )
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


/*!
    \fn kdesvnPart::reportBug()
 */
void kdesvnPart::reportBug()
{
  KBugReport dlg(m_view, true, createAboutData());
  dlg.exec();
}

/*!
    \fn kdesvnPart::showAboutApplication()
 */
void kdesvnPart::showAboutApplication()
{
    if (!m_aboutDlg) m_aboutDlg = new K3AboutApplication(createAboutData(), (QWidget *)0, (const char *)0, false);
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
         Kdesvnsettings::self(),
         KDialogBase::IconList);
    dialog->setHelp("setup","kdesvn");
    dialog->addPage(new DisplaySettings_impl(0,"general_items"),
        i18n("General"),"configure",i18n("General"),true);
    dialog->addPage(new SubversionSettings_impl(0,"subversion_items"),
        i18n("Subversion"),"kdesvn",i18n("Subversion Settings"),true);
    dialog->addPage(new DiffMergeSettings_impl(0,"diffmerge_items"),
        i18n("Diff & Merge"),"kdesvnmerge",i18n("Settings for diff and merge"),true);
    dialog->addPage(new DispColorSettings_impl(0,"color_items"),
        i18n("Colors"),"colorize",i18n("Color Settings"),true);
    dialog->addPage(new RevisiontreeSettingsDlg_impl(0,"revisiontree_items"),
        i18n("Revision tree"),"configure",i18n("Revision tree Settings"),true);
    dialog->addPage(new CmdExecSettings_impl(0,"cmdexec_items"),
        "KIO/"+i18n("Commandline"),"terminal",i18n("Settings for commandline and KIO execution"),true);

    connect(dialog,SIGNAL(settingsChanged()),this,SLOT(slotSettingsChanged()));
    dialog->show();
}


/*!
    \fn kdesvnPart::slotSettingsChanged()
 */
void kdesvnPart::slotSettingsChanged()
{
    KAction * temp;
    temp = actionCollection()->action("toggle_log_follows");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(Kdesvnsettings::log_follows_nodes());
    }
    temp = actionCollection()->action("toggle_ignored_files");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(Kdesvnsettings::display_ignored_files());
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

/*
 * we may not use generic factory 'cause we make some specials */
KInstance*  cFactory::s_instance = 0L;
KAboutData* cFactory::s_about = 0L;
commandline_part* cFactory::s_cline = 0L;

KParts::Part* cFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &args )
{
    Q_UNUSED(classname);
    // Create an instance of our Part
    return new kdesvnPart( parentWidget, widgetName, parent, name, args );
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

KInstance* cFactory::instance()
{
    if( !s_instance ) {
        s_about = kdesvnPart::createAboutData();
        s_instance = new KInstance(s_about);
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

#include "kdesvn_part.moc"
