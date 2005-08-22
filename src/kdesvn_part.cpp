
#include "kdesvn_part.h"
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

#include "kdesvn_part.moc"


/*!
    \fn kdesvnPart::setupActions()
 */
void kdesvnPart::setupActions()
{
    KConfigGroup cs(config(), "general_items");
    KConfigGroup cs2(config(), "subversion");

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
    KConfigGroup cs(config(), "general_items");
    cs.writeEntry("toggle_log_follows",how);
}


/*!
    \fn kdesvnPart::slotDiplayIgnored(bool)
 */
void kdesvnPart::slotDisplayIgnored(bool how)
{
    KConfigGroup cs(config(), "subversion");
    cs.writeEntry("display_ignored_files",how);
    emit refreshTree();
}


/*!
    \fn kdesvnPart::slotDisplayUnknown(bool)
 */
void kdesvnPart::slotDisplayUnkown(bool how)
{
    KConfigGroup cs(config(), "subversion");
    cs.writeEntry("display_unknown_files",how);
}


/*!
    \fn kdesvnPart::slotUseKompare(bool)
 */
void kdesvnPart::slotUseKompare(bool how)
{
    /// @todo make it into a settings class
    KConfigGroup cs(config(), "general_items");
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
    \fn kdesvnPart::config()
 */
KConfig* kdesvnPart::config()
{
    return kdesvnPartFactory::instance()->config();
}


/*!
    \fn kdesvnPart::iconloader()
 */
KIconLoader* kdesvnPart::iconLoader()
{
    return kdesvnPartFactory::instance()->iconLoader();
}
