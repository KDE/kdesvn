/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "kdesvn.h"
#include "pref.h"
#include "urldlg.h"

#include <qdragobject.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qcursor.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <kactionclasses.h>
#include <kmessagebox.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include <kdebug.h>

kdesvn::kdesvn()
    : KMainWindow( 0, "kdesvn" ),
      KBookmarkOwner(),
      m_view(new kdesvnView(this)),
      m_printer(0),
      statusResetTimer(new QTimer(this))
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    m_bookmarkFile = locateLocal("appdata",QString::fromLatin1("bookmarks.xml"),true);

    m_BookmarkManager = KBookmarkManager::managerForFile(m_bookmarkFile,false);
    m_BookmarkManager->setShowNSBookmarks(false);
    m_BookmarkManager->setEditorOptions(QString::fromLatin1("KDE Svn"),false);

    m_BookmarksActionmenu = new KActionMenu(i18n("&Bookmarks"),"bookmark",actionCollection(),"bookmarks");
    m_BookmarksActionmenu->setDelayed(false);
    m_BookmarksActionmenu->setEnabled(true);

    m_Bookmarkactions = new KActionCollection( this );
    m_Bookmarkactions->setHighlightingEnabled( true );
    connectActionCollection( m_Bookmarkactions );

    m_pBookmarkMenu = new KBookmarkMenu(m_BookmarkManager,this,m_BookmarksActionmenu->popupMenu(),m_Bookmarkactions,true);
    m_BookmarksActionmenu->plug(menuBar());

    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
#ifdef TESTING_RC
    createGUI(TESTING_RC);
    kdDebug()<<"Using test rc file in " << TESTING_RC << endl;
#else
    createGUI();
#endif
    KStdAction::configureToolbars(this,SLOT(configureToolbars() ), actionCollection());

    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));
    connect(statusResetTimer,SIGNAL(timeout()),this,SLOT(resetStatusBar()));
}

void kdesvn::connectActionCollection( KActionCollection *coll )
{
    if (!coll)return;
    connect( coll, SIGNAL( actionStatusText( const QString & ) ),
             this, SLOT( changeStatusbar( const QString & ) ) );
    connect( coll, SIGNAL( clearStatusText() ),
             this, SLOT( resetStatusBar() ) );
}

void kdesvn::disconnectActionCollection( KActionCollection *coll )
{
    if (!coll)return;
}

kdesvn::~kdesvn()
{
}

void kdesvn::load(const KURL& url)
{
    m_view->openURL(url);
}

void kdesvn::setupActions()
{
    KAction*tmpAction;
    tmpAction = KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    tmpAction->setText(i18n("Open repository or working copy"));
    tmpAction = KStdAction::close(this,SLOT(fileClose()),actionCollection());
    tmpAction->setEnabled(false);

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

    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
    KActionCollection*t=m_view->filesActions();
    if (t) {
        connectActionCollection(t);
        (*actionCollection())+=(*t);
    }
    // this doesn't do anything useful.  it's just here to illustrate
    // how to insert a custom menu and menu item
/*
    KAction *custom = new KAction(i18n("Cus&tom Menuitem"), 0,
                                  this, SLOT(optionsPreferences()),
                                  actionCollection(), "custom_action");
*/
}

void kdesvn::fileClose()
{
    m_view->closeMe();
}

void kdesvn::saveProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored

    if (!m_view->currentURL().isEmpty()) {
#if KDE_IS_VERSION(3,1,3)
        config->writePathEntry("lastURL", m_view->currentURL());
#else
        config->writeEntry("lastURL", m_view->currentURL());
#endif
    }
}

void kdesvn::readProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

    QString url = config->readPathEntry("lastURL");

    if (!url.isEmpty())
        m_view->openURL(KURL(url));
}

void kdesvn::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new kdesvn)->show();
}

void kdesvn::fileOpen()
{
    KURL url = UrlDlg::getURL(this);
    if (!url.isEmpty())
        m_view->openURL(url);
}

void kdesvn::fileSave()
{
    // this slot is called whenever the File->Save menu is selected,
    // the Save shortcut is pressed (usually CTRL+S) or the Save toolbar
    // button is clicked

    // save the current file
}

void kdesvn::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    KURL file_url = KFileDialog::getSaveURL();
    if (!file_url.isEmpty() && file_url.isValid())
    {
        // save your info, here
    }
}

void kdesvn::filePrint()
{
    // this slot is called whenever the File->Print menu is selected,
    // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
    // button is clicked
    if (!m_printer) m_printer = new KPrinter;
    if (m_printer->setup(this))
    {
        // setup the printer.  with Qt, you always "print" to a
        // QPainter.. whether the output medium is a pixmap, a screen,
        // or paper
        QPainter p;
        p.begin(m_printer);

        // we let our view do the actual printing
        QPaintDeviceMetrics metrics(m_printer);
        m_view->print(&p, metrics.height(), metrics.width());

        // and send the result to the printer
        p.end();
    }
}

void kdesvn::optionsPreferences()
{
    // popup some sort of preference dialog, here
    KMessageBox::error(this,i18n("Sorry, not finished. Will follow as soon as possible."),i18n("Settings"));
#if 0
    kdesvnPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
#endif
}

void kdesvn::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusResetTimer->stop();
    statusBar()->message(text);
}

void kdesvn::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void kdesvn::resetStatusBar()
{
    statusBar()->message(i18n("Ready"));
}

void kdesvn::openBookmarkURL (const QString &_url)
{
    if (!_url.isEmpty())
        m_view->openURL(_url);
}

QString kdesvn::currentTitle () const
{
    return caption();
}

QString kdesvn::currentURL () const
{
    return m_view->currentURL();
}

void kdesvn::slotDispPopup(const QString&name)
{
    QWidget *w = factory()->container(name, this);
    QPopupMenu *popup = static_cast<QPopupMenu *>(w);
    if (!popup) {
        kdDebug()<<"Error getting popupMenu"<<endl;
        return;
    }
    popup->exec(QCursor::pos());
}

#include "kdesvn.moc"


/*!
    \fn kdesvn::slotUrlOpened(bool)
 */
void kdesvn::slotUrlOpened(bool how)
{
    KAction * ac;
    if ( (ac=actionCollection()->action("file_close"))) {
        ac->setEnabled(how);
    }
}

void kdesvn::slotUseKompare(bool how)
{
    /// @todo make it into a settings class
    KConfigGroup cs(KGlobal::config(), "general_items");
    cs.writeEntry("use_kompare_for_diff",how);
}


/*!
    \fn kdesvn::slotLogFollowNodes(bool)
 */
void kdesvn::slotLogFollowNodes(bool how)
{
    KConfigGroup cs(KGlobal::config(), "general_items");
    cs.writeEntry("toggle_log_follows",how);
}


/*!
    \fn kdesvn::slotDisplayIgnored(bool)
 */
void kdesvn::slotDisplayIgnored(bool how)
{
    KConfigGroup cs(KGlobal::config(), "subversion");
    cs.writeEntry("display_ignored_files",how);
    emit refreshTree();
}


/*!
    \fn kdesvn::slotDisplayUnkown(bool)
 */
void kdesvn::slotDisplayUnkown(bool how)
{
    KConfigGroup cs(KGlobal::config(), "subversion");
    cs.writeEntry("display_unknown_files",how);
}
