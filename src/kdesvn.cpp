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

#include "kdesvn.h"
#include "urldlg.h"
#include "kdesvn_part.h"

#include <QTimer>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kbookmarkmenu.h>

#include <kapplication.h>
#include <kio/netaccess.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurlrequesterdlg.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <KActionCollection>
#include <KToggleAction>
#include <KRecentFilesAction>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kbookmarkmanager.h>
#include <kdebug.h>
#include <klibloader.h>
#include <kedittoolbar.h>
#include <KShortcutsDialog>

#ifdef TESTING_RC
#include <kcrash.h>
#endif

kdesvn::kdesvn()
    : KParts::MainWindow(),
      KBookmarkOwner()
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_part = 0;
#ifdef TESTING_RC
    setXMLFile(TESTING_RC);
    kDebug(9510) << "Using test rc file in " << TESTING_RC << endl;
    // I hate this crashhandler in development
    KCrash::setCrashHandler(0);
#else
    setXMLFile("kdesvnui.rc");
#endif
    setStandardToolBarMenuEnabled(true);
    // then, setup our actions
    setupActions();
    // and a status bar
    statusBar()->show();

    m_bookmarkFile = KStandardDirs::locateLocal("appdata", QString::fromLatin1("bookmarks.xml"), true);

    m_BookmarkManager = KBookmarkManager::managerForExternalFile(m_bookmarkFile);
    m_BookmarksActionmenu = new KBookmarkActionMenu(m_BookmarkManager->root(),
                                                    i18n("&Bookmarks"), this);

    actionCollection()->addAction("bookmarks", m_BookmarksActionmenu);
    m_Bookmarkactions = new KActionCollection(static_cast<QWidget *>(this));
    m_pBookmarkMenu = new KBookmarkMenu(m_BookmarkManager, this, m_BookmarksActionmenu->menu(), m_Bookmarkactions);
    m_pBookmarkMenu->setParent(this); // clear when kdesvn window gets destroyed

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = 0;
#ifdef EXTRA_KDE_LIBPATH
    factory = KLibLoader::self()->factory(EXTRA_KDE_LIBPATH + QString("/kdesvnpart.so"));
    if (!factory)
#endif
        factory = KLibLoader::self()->factory("kdesvnpart");

    if (factory) {
        m_part = factory->create<KParts::ReadOnlyPart>(this);
        if (m_part) {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());
            connect(this, SIGNAL(sigSavestate()), m_part->widget(), SLOT(slotSavestate()));
            connect(m_part->widget(), SIGNAL(sigExtraStatusMessage(QString)), this, SLOT(slotExtraStatus(QString)));

            KAction *tmpAction;
            tmpAction = actionCollection()->addAction("subversion_create_repo",
                                                      m_part->widget(),
                                                      SLOT(slotCreateRepo()));
            tmpAction->setText(i18n("Create and open new repository"));
            tmpAction->setToolTip(i18n("Create and opens a new local Subversion repository"));

            tmpAction = actionCollection()->addAction("subversion_dump_repo",
                                                      m_part->widget(),
                                                      SLOT(slotDumpRepo()));
            tmpAction->setText(i18n("Dump repository to file"));
            tmpAction->setToolTip(i18n("Dump a Subversion repository to a file"));

            tmpAction = actionCollection()->addAction("subversion_hotcopy_repo",
                                                      m_part->widget(),
                                                      SLOT(slotHotcopy()));
            tmpAction->setText(i18n("Hotcopy a repository"));
            tmpAction->setToolTip(i18n("Hotcopy a Subversion repository to a new folder"));

            tmpAction = actionCollection()->addAction("subversion_load_repo",
                                                      m_part->widget(),
                                                      SLOT(slotLoaddump()));
            tmpAction->setText(i18n("Load dump into repository"));
            tmpAction->setToolTip(i18n("Load a dump file into a repository."));

            tmpAction = actionCollection()->addAction("kdesvn_ssh_add",
                                                      m_part,
                                                      SLOT(slotSshAdd()));
            tmpAction->setText(i18n("Add ssh identities to ssh-agent"));
            tmpAction->setToolTip(i18n("Force add ssh-identities to ssh-agent for future use."));

            tmpAction = actionCollection()->addAction("help_about_kdesvnpart",
                                                      m_part,
                                                      SLOT(showAboutApplication()));
            tmpAction->setText(i18n("Info about kdesvn part"));
            tmpAction->setToolTip(i18n("Shows info about the kdesvn plugin and not the standalone application."));

            tmpAction = actionCollection()->addAction("db_show_status", m_part, SLOT(showDbStatus()));
            tmpAction->setText(i18n("Show database content"));
            tmpAction->setToolTip(i18n("Show the content of log cache database"));

            connectActionCollection(actionCollection());

            setHelpMenuEnabled(false);
            (void) new KHelpMenu(this, componentData().aboutData(), false, actionCollection());

            // and integrate the part's GUI with the shells
            createGUI(m_part);
            connectActionCollection(m_part->actionCollection());

        } else {
            KMessageBox::error(this, i18n("Could not load the part:\n%1", KLibLoader::self()->lastErrorMessage()));
            kapp->quit();
            return;
        }
    } else {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find our part:\n%1", KLibLoader::self()->lastErrorMessage()));
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }
    setAutoSaveSettings();
}

void kdesvn::connectActionCollection(KActionCollection *coll)
{
    if (!coll) {
        return;
    }
#if 0
    connect(coll, SIGNAL(actionHovered(QAction*)), this, SLOT(actionHovered(QAction*)));
#endif
}

kdesvn::~kdesvn()
{
}

void kdesvn::loadRescent(const KUrl &url)
{
    load(url, true);
}

void kdesvn::load(const KUrl &url, bool addRescent)
{
    QTimer::singleShot(100, this, SLOT(slotResetExtraStatus()));
    if (m_part) {
        bool ret = m_part->openUrl(url);
        KRecentFilesAction *rac = 0;
        if (addRescent) {
//             KAction * ac = actionCollection()->action("file_open_recent");
            QAction *ac = actionCollection()->action("file_open_recent");
            if (ac) {
                rac = (KRecentFilesAction *)ac;
            }
        }
        if (!ret) {
            changeStatusbar(i18n("Could not open URL %1", url.prettyUrl()));
            if (rac) {
                rac->removeUrl(url);
            }
        } else {
            resetStatusBar();
            if (rac) {
                rac->addUrl(url);
            }
        }
        if (rac) {
            KConfigGroup cg(KGlobal::config(), "recent_files");
//             rac->saveEntries(KGlobal::config(),"recent_files");
            rac->saveEntries(cg);
        }
    }
}

void kdesvn::setupActions()
{
    KAction *ac;
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    ac = KStandardAction::close(this, SLOT(fileClose()), actionCollection());
//     ac->setEnabled(getMemberList()->count()>1);
    ac->setEnabled(memberList().count() > 1);
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    KRecentFilesAction *rac = KStandardAction::openRecent(this, SLOT(loadRescent(KUrl)), actionCollection());
    if (rac) {
        rac->setMaxItems(8);
//         rac->loadEntries(KGlobal::config(),"recent_files");
        KConfigGroup cg(KGlobal::config(), "recent_files");
        rac->loadEntries(cg);
        rac->setText(i18n("Recent opened URLs"));
    }

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

    m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KToggleAction *toggletemp;
//     toggletemp = new KToggleAction(i18n("Load last opened URL on start"),KShortcut(),
//             actionCollection(),"toggle_load_last_url");
    toggletemp = new KToggleAction(i18n("Load last opened URL on start"), this);
    actionCollection()->addAction("toggle_load_last_url", toggletemp);
    toggletemp->setToolTip(i18n("Reload last opened URL if no one is given on command line"));
    KConfigGroup cs(KGlobal::config(), "startup");
//     toggletemp->setChecked(cs.readBoolEntry("load_last_on_start",false));
    toggletemp->setChecked(cs.readEntry("load_last_on_start", false));
    connect(toggletemp, SIGNAL(toggled(bool)), this, SLOT(slotLoadLast(bool)));
}

void kdesvn::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked()) {
        statusBar()->show();
    } else {
        statusBar()->hide();
    }
}

void kdesvn::fileClose()
{
    if (m_part) {
        m_part->closeUrl();
    }
//     if (getMemberList()->count()>1) {
    if (memberList().count() > 1) {
        close();
    } else {
        enableClose(false);
        QTimer::singleShot(100, this, SLOT(slotResetExtraStatus()));
        enableClose(false);
    }
}

void kdesvn::saveProperties(KConfigGroup &config)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
    if (!m_part) {
        return;
    }
    if (!m_part->url().isEmpty()) {
        config.writeEntry("lastURL", m_part->url().prettyUrl());
    }
}

void kdesvn::readProperties(const KConfigGroup &config)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

    QString url = config.readPathEntry("lastURL", QString());

    if (!url.isEmpty() && m_part) {
        m_part->openUrl(KUrl(url));
    }
}

void kdesvn::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new kdesvn)->show();
    enableClose(true);
}

void kdesvn::fileOpen()
{
    KUrl url = UrlDlg::getUrl(this);
    if (!url.isEmpty()) {
        load(url, true);
    }
}

void kdesvn::changeStatusbar(const QString &text)
{
    statusBar()->showMessage(text);
}

void kdesvn::resetStatusBar()
{
    statusBar()->showMessage(i18n("Ready"), 4000);
}

// kde4 port - pv
void kdesvn::openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    Q_UNUSED(mb);
    Q_UNUSED(km);
    if (!bm.url().isEmpty() && m_part) {
        load(bm.url(), false);
    }
}

QString kdesvn::currentUrl() const
{
    if (!m_part) {
        return QString();
    }
    return m_part->url().prettyUrl();
}

QString kdesvn::currentTitle() const
{
    if (!m_part) {
        return QString();
    }
    return m_part->url().fileName();
}

void kdesvn::enableClose(bool how)
{
    QAction *ac;
    if ((ac = actionCollection()->action("file_close"))) {
        ac->setEnabled(how);
    }
}

/*!
    \fn kdesvn::slotUrlOpened(bool)
 */
void kdesvn::slotUrlOpened(bool how)
{
    enableClose(how);
}

/*!
    \fn kdesvn::optionsConfigureToolbars()
 */
void kdesvn::optionsConfigureToolbars()
{
    KConfigGroup cg(KGlobal::config(), autoSaveGroup());
    saveMainWindowSettings(cg);

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

/*!
    \fn kdesvn::applyNewToolbarConfig()
 */
void kdesvn::applyNewToolbarConfig()
{
    KConfigGroup cg(KGlobal::config(), autoSaveGroup());
    applyMainWindowSettings(cg);
}

void kdesvn::optionsConfigureKeys()
{
    KShortcutsDialog kdlg(KShortcutsEditor::AllActions,
                          KShortcutsEditor::LetterShortcutsAllowed,
                          m_part->widget());
    kdlg.addCollection(actionCollection());
    kdlg.addCollection(m_part->actionCollection());

    kdlg.configure(true);
}

/*!
    \fn kdesvn::closeEvent()
 */
void kdesvn::closeEvent(QCloseEvent *ev)
{
    emit sigSavestate();
    if (m_part) {
        KConfigGroup cs(KGlobal::config(), "startup");
        cs.writeEntry("lastURL", m_part->url().prettyUrl());
        cs.sync();
    }
    return KParts::MainWindow::closeEvent(ev);
}

/*!
    \fn kdesvn::checkReload()
 */
void kdesvn::checkReload()
{
    KConfigGroup cs(KGlobal::config(), "startup");
    if (!cs.readEntry("load_last_on_start", false)) {
        return;
    }

    QString url = cs.readPathEntry("lastURL", QString());

    if (!url.isEmpty() && m_part) {
        load(KUrl(url), false);
    }
}

/*!
    \fn kdesvn::slotLoadLast(bool)
 */
void kdesvn::slotLoadLast(bool how)
{
    KConfigGroup cs(KGlobal::config(), "startup");
    cs.writeEntry("load_last_on_start", how);
    cs.sync();
}

void kdesvn::slotResetExtraStatus()
{
    if (statusBar()->hasItem(1)) {
        statusBar()->removeItem(1);
    }
}

void kdesvn::slotExtraStatus(const QString &what)
{
    if (!statusBar()->hasItem(1)) {
        statusBar()->insertItem(what, 1);
    } else {
        statusBar()->changeItem(what, 1);
    }
}
