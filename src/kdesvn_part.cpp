
#include "kdesvn_part.h"
#include "kdesvn_part_config.h"
#include "displaysettings_impl.h"
#include "svncpp/version_check.hpp"
#include "../config.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kdesvnview.h>
#include <kbugreport.h>
#include <qpopupmenu.h>
#include <kxmlguifactory.h>
#include <kaboutapplication.h>
#include <kapp.h>
#include <kconfigdialog.h>
#include <kconfigskeleton.h>

#include <qcursor.h>

K_EXPORT_COMPONENT_FACTORY( libkdesvnpart, kdesvnPartFactory )

static const char version[] = VERSION;
QString kdesvnPart::m_Extratext = "";

static const char description[] =
    I18N_NOOP("A Subversion Client for KDE (dynamic Part component)");

 class kdesvnPart_Prefs : public KConfigSkeleton
 {
    public:
        kdesvnPart_Prefs(KSharedConfig::Ptr config)
            : KConfigSkeleton(config)
        {
            setCurrentGroup("general_items");
            addItemInt("listview_icon_size",mlist_icon_size,22);
            addItemBool("display_overlays",mdisp_overlay,true);
            addItemInt("use_kompare_for_diff",muse_kompare,1);
            addItemString("external_diff_display",mdiff_display,"kompare");
            addItemInt("max_log_messages",mmax_log_messages,20);

            setCurrentGroup("subversion");
            addItemBool("display_unknown_files",mdisp_unknown_files,true);
            addItemBool("display_ignored_files",mdisp_ignored_files,true);
            addItemBool("log_follows_nodes",mlog_follows_nodes,true);
        }

        static kdesvnPart_Prefs*self()
        {
            if (!_me) {
                _me = new kdesvnPart_Prefs(kdesvnPartFactory::instance()->sharedConfig());
                _me->readConfig();
            }
            return _me;
        }
        int mlist_icon_size;
        bool mdisp_overlay;
        int muse_kompare;
        bool mdisp_unknown_files;
        bool mdisp_ignored_files;
        bool mlog_follows_nodes;
        int mmax_log_messages;
        QString mdiff_display;

    private:
        static kdesvnPart_Prefs*_me;
 };

kdesvnPart_Prefs*kdesvnPart_Prefs::_me=0;

kdesvnPart::kdesvnPart( QWidget *parentWidget, const char *widgetName,
                                  QObject *parent, const char *name , const QStringList&)
    : KParts::ReadOnlyPart(parent, name)
{
    m_aboutDlg = 0;
    KGlobal::locale()->insertCatalogue("kdesvn");
    // we need an instance
    setInstance( kdesvnPartFactory::instance() );
    m_browserExt = new KdesvnBrowserExtension( this );

    // this should be your custom internal widget
    m_view = new kdesvnView(actionCollection(),parentWidget,widgetName);

    // notify the part that this is our internal widget
    setWidget(m_view);

    // create our actions
    setupActions();
    // set our XML-UI resource file
#ifdef TESTING_PARTRC
    setXMLFile(TESTING_PARTRC);
    kdDebug()<<"Using test rc file in " << TESTING_PARTRC << endl;
#else
    setXMLFile("kdesvn_part.rc");
#endif
    connect(m_view,SIGNAL(sigShowPopup(const QString&)),this,SLOT(slotDispPopup(const QString&)));
    connect(m_view,SIGNAL(sigSwitchUrl(const KURL&)),this,SLOT(openURL(const KURL&)));
    connect(this,SIGNAL(refreshTree()),m_view,SLOT(refreshCurrentTree()));
    m_browserExt->setPropertiesActionEnabled(false);
}

kdesvnPart::~kdesvnPart()
{
}

bool kdesvnPart::openFile()
{
    m_view->openURL(m_url);
    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

bool kdesvnPart::openURL(const KURL&url)
{
    KURL _url = url;
    if (_url.protocol()=="https+svn") {
        _url.setProtocol("https");
    } else if (_url.protocol()=="http+svn") {
        _url.setProtocol("http");
    }
    if (!_url.isValid()||!closeURL()) {
        return false;
    }
    m_url = _url;
    emit started(0);
    bool ret = m_view->openURL(m_url);
    if (ret) {
        emit completed();
        emit setWindowCaption(url.prettyURL());
    }
    return ret;
}

void kdesvnPart::slotFileProperties()
{
}

void kdesvnPart::slotDispPopup(const QString&name)
{
    QWidget *w = hostContainer(name);
    QPopupMenu *popup = static_cast<QPopupMenu *>(w);
    if (!popup) {
        kdDebug()<<"Error getting popupMenu"<<endl;
        return;
    }
    popup->exec(QCursor::pos());
}

KAboutData* kdesvnPart::createAboutData()
{
    m_Extratext = QString(I18N_NOOP("Built with Subversion library: %1\n")).arg(svn::Version::linked_version());
    m_Extratext+=QString(I18N_NOOP("Running Subversion library: %1")).arg(svn::Version::running_version());

    KAboutData*about = new KAboutData("kdesvnpart", I18N_NOOP("kdesvn Part"), version, description,
                     KAboutData::License_GPL, "(C) 2005 Rajko Albrecht",0,
                         0, "ral@alwins-world.de");
    about->addAuthor( "Rajko Albrecht", 0, "ral@alwins-world.de" );
    about->setOtherText(m_Extratext);
    about->setHomepage("http://www.alwins-world.de/programs/kdesvn/");
    about->setBugAddress("kdesvn-bugs@alwins-world.de");
    about->setTranslator(I18N_NOOP("kdesvn: NAME OF TRANSLATORS\\nYour names"),
        I18N_NOOP("kdesvn: EMAIL OF TRANSLATORS\\nYour emails"));
    return about;
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

#include "kdesvn_part.moc"


/*!
    \fn kdesvnPart::setupActions()
 */
void kdesvnPart::setupActions()
{
    KToggleAction *toggletemp;
    toggletemp = new KToggleAction(i18n("Use \"Kompare\" for displaying diffs"),KShortcut(),
            actionCollection(),"toggle_use_kompare");
    toggletemp->setChecked(kdesvnPart_Prefs::self()->muse_kompare);
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotUseKompare(bool)));

    toggletemp = new KToggleAction(i18n("Logs follow node changes"),KShortcut(),
            actionCollection(),"toggle_log_follows");
    toggletemp->setChecked(kdesvnPart_Prefs::self()->mlog_follows_nodes);
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotLogFollowNodes(bool)));

    toggletemp = new KToggleAction(i18n("Display ignored files"),KShortcut(),
            actionCollection(),"toggle_ignored_files");
    toggletemp->setChecked(kdesvnPart_Prefs::self()->mdisp_ignored_files);
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayIgnored(bool)));
    toggletemp = new KToggleAction(i18n("Display unknown files"),KShortcut(),
            actionCollection(),"toggle_unknown_files");
    toggletemp->setChecked(kdesvnPart_Prefs::self()->mdisp_unknown_files);
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayUnkown(bool)));

    kdDebug()<<"Appname = " << (QString)kapp->instanceName() << endl;

    KAction * t = KStdAction::preferences(this, SLOT(slotShowSettings()), actionCollection(),"kdesvnpart_pref");
    t->setText(i18n("&Configure %1...").arg("Kdesvn"));
    (void)new KAction(i18n("&About kdesvn part"), "kdesvn", 0, this, SLOT(showAboutApplication()), actionCollection(), "help_about_kdesvnpart");
    if (QString(kapp->instanceName())!=QString("kdesvn")) {
        (void)new KAction(i18n("Kdesvn &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kdesvn");
    }
    (void)new KAction(i18n("Send Bugreport for kdesvn"), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
    actionCollection()->setHighlightingEnabled(true);
}


/*!
    \fn kdesvnPart::slotLogFollowNodes(bool)
 */
void kdesvnPart::slotLogFollowNodes(bool how)
{
    kdesvnPart_Prefs::self()->mlog_follows_nodes=how;
    kdesvnPart_Prefs::self()->writeConfig();
}


/*!
    \fn kdesvnPart::slotDiplayIgnored(bool)
 */
void kdesvnPart::slotDisplayIgnored(bool how)
{
    kdesvnPart_Prefs::self()->mdisp_ignored_files = how;
    kdesvnPart_Prefs::self()->writeConfig();
    emit refreshTree();
}


/*!
    \fn kdesvnPart::slotDisplayUnknown(bool)
 */
void kdesvnPart::slotDisplayUnkown(bool how)
{
    kdesvnPart_Prefs::self()->mdisp_unknown_files=how;
    kdesvnPart_Prefs::self()->writeConfig();
}


/*!
    \fn kdesvnPart::slotUseKompare(bool)
 */
void kdesvnPart::slotUseKompare(bool how)
{
    kdesvnPart_Prefs::self()->muse_kompare=how?1:0;
    kdesvnPart_Prefs::self()->writeConfig();
}


/*!
    \fn kdesvnPart::closeURL()
 */
bool kdesvnPart::closeURL()
{
    m_url=KURL();
    m_view->closeMe();
    emit setWindowCaption("");
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension( kdesvnPart *p )
    : KParts::BrowserExtension( p, "KdesvnBrowserExtension" )
{
    KGlobal::locale()->insertCatalogue("kdesvn");
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
    if (!m_aboutDlg) m_aboutDlg = new KAboutApplication(createAboutData(), (QWidget *)0, (const char *)0, false);
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
    kapp->invokeHelp(QString::null, "kdesvn");
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
         kdesvnPart_Prefs::self(),
         KDialogBase::IconList);
    dialog->addPage(new DisplaySettings_impl(0,"general_items"),
        i18n("General"),"kdesvn",i18n("General"),true);
    connect(dialog,SIGNAL(settingsChanged()),this,SLOT(slotSettingsChanged()));
    connect(this,SIGNAL(settingsChanged()),widget(),SLOT(slotSettingsChanged()));
    dialog->show();
}


/*!
    \fn kdesvnPart::slotSettingsChanged()
 */
void kdesvnPart::slotSettingsChanged()
{
    KAction * temp;
    temp = actionCollection()->action("toggle_use_kompare");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(kdesvnPart_Prefs::self()->muse_kompare==1);
    }
    temp = actionCollection()->action("toggle_log_follows");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(kdesvnPart_Prefs::self()->mlog_follows_nodes);
    }
    temp = actionCollection()->action("toggle_ignored_files");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(kdesvnPart_Prefs::self()->mdisp_ignored_files);
    }
    temp = actionCollection()->action("toggle_unknown_files");
    if (temp) {
        ((KToggleAction*)temp)->setChecked(kdesvnPart_Prefs::self()->mdisp_unknown_files);
    }
    emit settingsChanged();
}

QVariant kdesvnPart_config::configItem(const QString& name)
{
    KConfigSkeletonItem*it = kdesvnPart_Prefs::self()->findItem(name);
    if (!it) {
        return QVariant();
    }
    return it->property();
}

/*!
    \fn kdesvnPart_config::config()
 */
KConfig* kdesvnPart_config::config()
{
    return kdesvnPartFactory::instance()->config();
}

/*!
    \fn kdesvnPart::iconloader()
 */
KIconLoader* kdesvnPart_config::iconLoader()
{
    return kdesvnPartFactory::instance()->iconLoader();
}
