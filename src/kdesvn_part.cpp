
#include "kdesvn_part.h"
#include "urldlg.h"
#include "svncpp/version_check.hpp"
#include "../config.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kdesvnview.h>
#include <qpopupmenu.h>
#include <kxmlguifactory.h>

#include <qcursor.h>

K_EXPORT_COMPONENT_FACTORY( libkdesvnpart, kdesvnPartFactory )

static const char version[] = VERSION;

static const char description[] =
    I18N_NOOP("A Subversion Client for KDE (dynamic Part component)");

kdesvnPart::kdesvnPart( QWidget *parentWidget, const char *widgetName,
                                  QObject *parent, const char *name , const QStringList&)
    : KParts::ReadOnlyPart(parent, name)
{
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
    connect(this,SIGNAL(refreshTree()),m_view,SLOT(refreshCurrentTree()));
    m_browserExt->setPropertiesActionEnabled(false);
}

kdesvnPart::~kdesvnPart()
{
}

bool kdesvnPart::openFile()
{
//    if (!m_file.isEmpty())
        //m_view->openURL(m_file);

    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

void kdesvnPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    KURL url = UrlDlg::getURL(m_view);

    if (url.isEmpty() == false)
        openURL(url);
}

bool kdesvnPart::openURL(const KURL &url)
{
    m_url=url;
    return m_view->openURL(url);
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
    QString text = QString(I18N_NOOP("Build with subversion lib: %1\n")).arg(svn::Version::linked_version());
    text+=QString(I18N_NOOP("Running subversion lib: %1")).arg(svn::Version::running_version());

    KAboutData*about = new KAboutData("kdesvnpart", I18N_NOOP("kdesvn Part"), version, description,
                     KAboutData::License_GPL, "(C) 2005 Rajko Albrecht",text,
                         0, "ral@alwins-world.de");
    about->addAuthor( "Rajko Albrecht", 0, "ral@alwins-world.de" );
    about->setHomepage("http://www.alwins-world.de/programs/kdesvn/");
    return about;
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

#if 0
KInstance*  kdesvnPartFactory::s_instance = 0L;
KAboutData* kdesvnPartFactory::s_about = 0L;

kdesvnPartFactory::kdesvnPartFactory()
    : KParts::Factory()
{
}

kdesvnPartFactory::~kdesvnPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* kdesvnPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &args )
{
    // Create an instance of our Part
    kdesvnPart* obj = new kdesvnPart( parentWidget, widgetName, parent, name );

    return obj;
}

KInstance* kdesvnPartFactory::instance()
{
    if( !s_instance )
    {
        s_about = new KAboutData("kdesvnpart", I18N_NOOP("kdesvnPart"), "0.1");
        s_about->addAuthor("Rajko Albrecht", 0, "rajko.albrecht@tecways.com");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
    void* init_libkdesvnpart()
    {
        return new kdesvnPartFactory;
    }
};

#endif

#include "kdesvn_part.moc"


/*!
    \fn kdesvnPart::setupActions()
 */
void kdesvnPart::setupActions()
{
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());

    KConfigGroup cs(KGlobal::config(), "general_items");
    KConfigGroup cs2(KGlobal::config(), "subversion");

    KToggleAction *toggletemp;
    toggletemp = new KToggleAction(i18n("Use \"Kompare\" for displaying diffs"),KShortcut(),
            actionCollection(),"toggle_use_kompare");
    toggletemp->setChecked(cs.readBoolEntry("use_kompare_for_diff",true));
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotUseKompare(bool)));

    toggletemp = new KToggleAction(i18n("Logs follows node changes"),KShortcut(),
            actionCollection(),"toggle_log_follows");
    toggletemp->setChecked(cs.readBoolEntry("log_follows_nodes",true));
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotLogFollowNodes(bool)));

    toggletemp = new KToggleAction(i18n("Display ignored files"),KShortcut(),
            actionCollection(),"toggle_ignored_files");
    toggletemp->setChecked(cs2.readBoolEntry("display_ignored_files",true));
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayIgnored(bool)));
    toggletemp = new KToggleAction(i18n("Display unknown files"),KShortcut(),
            actionCollection(),"toggle_unknown_files");
    toggletemp->setChecked(cs2.readBoolEntry("display_unknown_files",true));
    connect(toggletemp,SIGNAL(toggled(bool)),this,SLOT(slotDisplayUnkown(bool)));
}


/*!
    \fn kdesvnPart::slotLogFollowNodes(bool)
 */
void kdesvnPart::slotLogFollowNodes(bool how)
{
    KConfigGroup cs(KGlobal::config(), "general_items");
    cs.writeEntry("toggle_log_follows",how);
}


/*!
    \fn kdesvnPart::slotDiplayIgnored(bool)
 */
void kdesvnPart::slotDisplayIgnored(bool how)
{
    KConfigGroup cs(KGlobal::config(), "subversion");
    cs.writeEntry("display_ignored_files",how);
    emit refreshTree();
}


/*!
    \fn kdesvnPart::slotDisplayUnknown(bool)
 */
void kdesvnPart::slotDisplayUnkown(bool how)
{
    KConfigGroup cs(KGlobal::config(), "subversion");
    cs.writeEntry("display_unknown_files",how);
}


/*!
    \fn kdesvnPart::slotUseKompare(bool)
 */
void kdesvnPart::slotUseKompare(bool how)
{
    /// @todo make it into a settings class
    KConfigGroup cs(KGlobal::config(), "general_items");
    cs.writeEntry("use_kompare_for_diff",how);
}


/*!
    \fn kdesvnPart::closeURL()
 */
bool kdesvnPart::closeURL()
{
    m_url=KURL();
    m_view->closeMe();
    return true;
}

KdesvnBrowserExtension::KdesvnBrowserExtension( kdesvnPart *p )
    : KParts::BrowserExtension( p, "CervisiaBrowserExtension" )
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
