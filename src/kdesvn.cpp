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

#include <qdragobject.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

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

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstandarddirs.h>
#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>

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
/*
    connect( m_pBookmarkMenu,
        SIGNAL( aboutToShowContextMenu(const KBookmark &, QPopupMenu*) ),
        this, SLOT( slotFillContextMenu(const KBookmark &, QPopupMenu*) ));
    connect( m_pBookmarkMenu,
        SIGNAL( openBookmark(const QString &, Qt::ButtonState) ),
        this, SLOT( slotOpenBookmarkURL(const QString &, Qt::ButtonState) ));
*/
    m_BookmarksActionmenu->plug(menuBar());
    //menuBar()->insertItem(i18n("Bookmarks"),bookpopup);
    setHelpMenuEnabled(true);

    KPopupMenu *help = helpMenu();
    menuBar()->insertItem(i18n("&Help"),help);

    // Apply the create the main window and ask the mainwindow to
        // automatically save settings if changed: window size, toolbar
    // position, icon size, etc.  Also to add actions for the statusbar
        // toolbar, and keybindings if necessary.
    //setupGUI();
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled( true );
    KStdAction::configureToolbars(this,SLOT(configureToolbars() ), actionCollection());

    setAutoSaveSettings();
    //setupGUI(Save|StatusBar|ToolBar);

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
    setCaption(url.prettyURL());
    m_view->openURL(url);
}

void kdesvn::setupActions()
{
    KActionMenu*m_FileMenu=new KActionMenu(I18N_NOOP("&File"),this);
    m_DirOpen = KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    m_UrlOpen = new KAction(I18N_NOOP("Open remote repository"),KShortcut(),this,SLOT(urlOpen()),actionCollection());

    m_FileMenu->insert(m_DirOpen);
    m_FileMenu->insert(m_UrlOpen);
    m_FileMenu->insert(KStdAction::quit(kapp, SLOT(quit()), actionCollection()));

    m_FileMenu->plug(menuBar());

    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
    KActionCollection*t=m_view->filesActions();
    if (t) {
        KActionMenu*svnmenu = new KActionMenu(I18N_NOOP("Subversion"),this);
        KActionPtrList l = t->actions();
        KActionPtrList::iterator it;
        for (it=l.begin();it!=l.end();++it) {
            svnmenu->insert((*it));
        }
        svnmenu->plug(menuBar());
    }

    // this doesn't do anything useful.  it's just here to illustrate
    // how to insert a custom menu and menu item
/*
    KAction *custom = new KAction(i18n("Cus&tom Menuitem"), 0,
                                  this, SLOT(optionsPreferences()),
                                  actionCollection(), "custom_action");
*/
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

void kdesvn::dragEnterEvent(QDragEnterEvent *event)
{
    // accept uri drops only
    event->accept(KURLDrag::canDecode(event));
}

void kdesvn::dropEvent(QDropEvent *event)
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there
    KURL::List urls;

    // see if we can decode a URI.. if not, just ignore it
    if (KURLDrag::decode(event, urls) && !urls.isEmpty())
    {
        // okay, we have a URI.. process it
        const KURL &url = urls.first();

        // load in the file
        load(url);
    }
}

void kdesvn::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new kdesvn)->show();
}

void kdesvn::urlOpen()
{
    KURL url = KURLRequesterDlg::getURL("http://",this,i18n("Open remote repository"));
    if (!url.isEmpty())
        m_view->openURL(url);
}

void kdesvn::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
/*
    // this brings up the generic open dialog
    KURL url = KURLRequesterDlg::getURL(QString::null, this, i18n("Open Location") );
*/
    // standard filedialog
    KURL url = KFileDialog::getExistingDirectory(QString::null, this, i18n("Open working copy"));
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
    kdesvnPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
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
    return m_view->currentURL();
}

QString kdesvn::currentURL () const
{
    return m_view->currentURL();
}

#include "kdesvn.moc"
