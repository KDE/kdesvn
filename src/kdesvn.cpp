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
#include <klibloader.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>

kdesvn::kdesvn()
    : KParts::MainWindow( 0, "kdesvn" ),
      KBookmarkOwner()
{
#ifdef TESTING_RC
    setXMLFile(TESTING_RC);
    kdDebug()<<"Using test rc file in " << TESTING_RC << endl;
#else
    setXMLFile("kdesvnui.rc");
#endif
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
//    m_BookmarksActionmenu->plug(menuBar());
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libkdesvnpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                                "kdesvn_part", "KParts::ReadOnlyPart" ));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            // and integrate the part's GUI with the shell's
            createGUI(m_part);
        }
        connectActionCollection(m_part->actionCollection());
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find our part."));
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

    setAutoSaveSettings();
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
    if (m_part) m_part->openURL(url);
}

void kdesvn::setupActions()
{
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::close(this,SLOT(fileClose()),actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

//    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
    // this doesn't do anything useful.  it's just here to illustrate
    // how to insert a custom menu and menu item
/*
    KAction *custom = new KAction(i18n("Cus&tom Menuitem"), 0,
                                  this, SLOT(optionsPreferences()),
                                  actionCollection(), "custom_action");
*/
}

void kdesvn::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void kdesvn::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void kdesvn::fileClose()
{
    if (m_part) m_part->closeURL();
}

void kdesvn::saveProperties(KConfig *config)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
    if (!m_part) return;
    if (!m_part->url().isEmpty()) {
#if KDE_IS_VERSION(3,1,3)
        config->writePathEntry("lastURL", m_part->url().prettyURL());
#else
        config->writeEntry("lastURL", m_part->url().prettyURL());
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

    if (!url.isEmpty() && m_part)
        m_part->openURL(KURL(url));
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
        m_part->openURL(url);
}

void kdesvn::optionsPreferences()
{
    // popup some sort of preference dialog, here
    KMessageBox::error(this,i18n("Sorry, the configuration dialog is not finished yet. It will be implemented as soon as possible."),i18n("Settings"));
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
    statusBar()->message(text);
}

void kdesvn::resetStatusBar()
{
    statusBar()->message(i18n("Ready"));
}

void kdesvn::openBookmarkURL (const QString &_url)
{
    if (!_url.isEmpty() && m_part)
        m_part->openURL(_url);
}

QString kdesvn::currentURL () const
{
    if (!m_part) return "";
    return m_part->url().prettyURL();
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


/*!
    \fn kdesvn::optionsConfigureToolbars()
 */
void kdesvn::optionsConfigureToolbars()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config() );
# endif
#else
    saveMainWindowSettings(KGlobal::config() );
#endif

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}


/*!
    \fn kdesvn::applyNewToolbarConfig()
 */
void kdesvn::applyNewToolbarConfig()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(kdesvnPart::config());
# endif
#else
    applyMainWindowSettings(kdesvnPart::config());
#endif
}

void kdesvn::optionsConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}
