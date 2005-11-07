/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "importdir_logmsg.h"
#include "copymoveview_impl.h"
#include "mergedlg_impl.h"
#include "svnactions.h"
#include "svnfiletip.h"
#include "checkoutinfo_impl.h"
#include "fronthelpers/settings.h"
#include "svncpp/revision.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"
#include "helpers/sshagent.h"
#include "helpers/runtempfile.h"

#include <kapplication.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kmessagebox.h>
#include <kdirselectdialog.h>
#include <klineedit.h>
#include <kfileitem.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kurldrag.h>
#include <ktempfile.h>
#include <kio/job.h>
#include <krun.h>

#include <qvbox.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qregexp.h>

class KdesvnFileListPrivate{
public:
    KdesvnFileListPrivate();
    virtual ~KdesvnFileListPrivate()
    {
        if (m_DirWatch) {
            m_DirWatch->stopScan();
            delete m_DirWatch;
        }
        delete m_fileTip;
        kdDebug()<<"Destructor KdesvnFileListPrivate done"<<endl;
    };
    QListViewItem *dragOverItem;
    QPoint dragOverPoint;
    QRect mOldDropHighlighter;
    svn::Revision m_remoteRevision;
    KDirWatch*m_DirWatch;
    SvnFileTip*m_fileTip;
    int mlist_icon_size;
    bool mdisp_ignored_files;
    bool mdisp_overlay;
    /* returns true if the display must refreshed */
    bool reReadSettings();
private:
    void readSettings();
};

KdesvnFileListPrivate::KdesvnFileListPrivate()
    : dragOverItem(0),dragOverPoint(QPoint(0,0)),mOldDropHighlighter()
{
    m_remoteRevision = svn::Revision::HEAD;
    m_DirWatch = 0;
    readSettings();
}

void KdesvnFileListPrivate::readSettings()
{
    mlist_icon_size = Settings::listview_icon_size();
    mdisp_ignored_files = Settings::display_ignored_files();
    mdisp_overlay = Settings::display_overlays();
}

bool KdesvnFileListPrivate::reReadSettings()
{
    int _size = mlist_icon_size;
    bool _ignored = mdisp_ignored_files;
    bool _overlay = mdisp_overlay;
    readSettings();
    return (_size != mlist_icon_size||_ignored!=mdisp_ignored_files||_overlay!=mdisp_overlay);
}

kdesvnfilelist::kdesvnfilelist(KActionCollection*aCollect,QWidget *parent, const char *name)
 : KListView(parent, name),ItemDisplay(),m_SvnWrapper(new SvnActions(this))
{
    m_SelectedItems = 0;
    m_pList = new KdesvnFileListPrivate;
    m_filesAction = aCollect;
    m_pList->m_fileTip=new SvnFileTip(this);
    m_pList->m_fileTip->setOptions(Settings::display_file_tips()&&
        QToolTip::isGloballyEnabled(),true,6);

    SshAgent ssh;
    ssh.querySshAgent();

    setMultiSelection(true);
    setSelectionModeExt(FileManager);
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setRootIsDecorated(true);
    addColumn(i18n("Name"));
    addColumn(i18n("Status"));
    addColumn(i18n("Last changed Revision"));
    addColumn(i18n("Last author"));
    addColumn(i18n("Last change date"));
    addColumn(i18n("Locked by"));
    setSortColumn(FileListViewItem::COL_NAME);
    setupActions();

    connect(this,SIGNAL(clicked(QListViewItem*)),this,SLOT(slotItemClicked(QListViewItem*)));
    connect(this,SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),this,
        SLOT(slotRightButton(QListViewItem *, const QPoint &, int)));
    connect(this,SIGNAL(doubleClicked(QListViewItem*)),this,SLOT(slotItemDoubleClicked(QListViewItem*)));
    connect(this,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_SvnWrapper,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(m_SvnWrapper,SIGNAL(reinitItem(SvnItem*)),this,SLOT(slotReinitItem(SvnItem*)));
    connect(m_SvnWrapper,SIGNAL(sigRefreshAll()),this,SLOT(refreshCurrentTree()));
    connect(m_SvnWrapper,SIGNAL(sigRefreshCurrent(SvnItem*)),this,SLOT(refreshCurrent(SvnItem*)));
    connect(m_SvnWrapper,SIGNAL(sigRefreshIcons()),this,SLOT(slotRescanIcons()));
    connect(this,SIGNAL(dropped (QDropEvent*,QListViewItem*)),
            this,SLOT(slotDropped(QDropEvent*,QListViewItem*)));

    setDropHighlighter(true);
    setDragEnabled(true);
    setItemsMovable(true);
    setDropVisualizer(false);
    setAcceptDrops(true);
}

svn::Client*kdesvnfilelist::svnclient()
{
    return m_SvnWrapper->svnclient();
}

void kdesvnfilelist::setupActions()
{
    if (!m_filesAction) return;
    KAction*tmp_action;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    m_LogRangeAction = new KAction(i18n("&Log..."),"svnlog",KShortcut(SHIFT+CTRL+Key_L),m_SvnWrapper,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    m_LogFullAction = new KAction(i18n("&Full Log"),"svnlog",KShortcut(CTRL+Key_L),m_SvnWrapper,SLOT(slotMakeLog()),m_filesAction,"make_svn_log_full");
    m_propertyAction = new KAction(i18n("Properties"),"edit",
        KShortcut(Key_P),m_SvnWrapper,SLOT(slotProperties()),m_filesAction,"make_svn_property");
    m_InfoAction = new KAction(i18n("Details"),"svninfo",
        KShortcut(Key_I),this,SLOT(slotInfo()),m_filesAction,"make_svn_info");
    m_RenameAction = new KAction(i18n("Move"),"move",
        KShortcut(Key_F2),this,SLOT(slotRename()),m_filesAction,"make_svn_rename");
    m_CopyAction = new KAction(i18n("Copy"),"svncopy",
        KShortcut(Key_C),this,SLOT(slotCopy()),m_filesAction,"make_svn_copy");
    tmp_action = new KAction(i18n("Check for updates"),"svncheckupdates",KShortcut(),this,SLOT(slotCheckUpdates()),m_filesAction,"make_check_updates");
    tmp_action->setToolTip(i18n("Check if current working copy has items with newer version in repository"));

    /* 2. actions only on files */
    m_BlameAction = new KAction("&Blame","svnblame",KShortcut(),this,SLOT(slotBlame()),m_filesAction,"make_svn_blame");
    m_BlameAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_BlameRangeAction = new KAction("Blame range","svnblame",KShortcut(),this,SLOT(slotRangeBlame()),m_filesAction,"make_svn_range_blame");
    m_BlameRangeAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_CatAction = new KAction(i18n("Cat head"),
        "svncat",KShortcut(),this,SLOT(slotCat()),m_filesAction,"make_svn_cat");
    m_CatAction->setToolTip(i18n("Output the content of specified files or URLs."));
    tmp_action = new KAction(i18n("Cat revision..."),"svncat",KShortcut(),this,SLOT(slotRevisionCat()),m_filesAction,"make_revisions_cat");
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs at specific revision."));

    m_LockAction = new KAction(i18n("Lock current items"),"svnlock",KShortcut(),this,SLOT(slotLock()),m_filesAction,"make_svn_lock");
    m_UnlockAction = new KAction(i18n("Unlock current items"),"svnunlock",KShortcut(),this,SLOT(slotUnlock()),m_filesAction,"make_svn_unlock");

    /* 3. actions only on dirs */
    m_MkdirAction = new KAction("New folder","folder_new",
        KShortcut(),this,SLOT(slotMkdir()),m_filesAction,"make_svn_mkdir");
    m_switchRepository = new KAction("Switch repository","svnswitch",KShortcut(),
        m_SvnWrapper,SLOT(slotSwitch()),m_filesAction,"make_svn_switch");
    m_switchRepository->setToolTip(i18n("Switch repository of working copy (\"svn switch\")"));
    tmp_action = new KAction(i18n("Relocate working copy url"),"svnrelocate",KShortcut(),
        this,SLOT(slotRelocate()),m_filesAction,"make_svn_relocate");
    tmp_action->setToolTip(i18n("Relocate url of current working copy to a new one"));

    m_changeToRepository = new KAction(i18n("Open repository of working copy"),"gohome",KShortcut(),
        this,SLOT(slotChangeToRepository()),m_filesAction,"make_switch_to_repo");
    m_changeToRepository->setToolTip(i18n("Opens the repository the current working copy was checked out from"));

    m_CleanupAction = new KAction("Cleanup","cleanup",KShortcut(),
        this,SLOT(slotCleanupAction()),m_filesAction,"make_cleanup");
    m_ImportDirsIntoCurrent  = new KAction(i18n("Import folders into current"),"fileimport",KShortcut(),
        this,SLOT(slotImportDirsIntoCurrent()),m_filesAction,"make_import_dirs_into_current");
    m_ImportDirsIntoCurrent->setToolTip(i18n("Import folder content into current url"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent = new KAction("Add selected files/dirs",
        "svnadd",KShortcut(Key_Insert),m_SvnWrapper,SLOT(slotAdd()),m_filesAction,"make_svn_add");
    m_AddCurrent->setToolTip(i18n("Adding selected files and/or directories to repository"));
    tmp_action = new KAction("Add selected files/dirs recursive",
        "svnaddrecursive",KShortcut(CTRL+Key_Insert),m_SvnWrapper,SLOT(slotAddRec()),m_filesAction,"make_svn_addrec");
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository and all subitems of folders"));

    m_DelCurrent = new KAction("Delete selected files/dirs","svndelete",
        KShortcut(Key_Delete),this,SLOT(slotDelete()),m_filesAction,"make_svn_remove");
    m_DelCurrent->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    m_RevertAction  = new KAction("Revert current changes","revert",
        KShortcut(),m_SvnWrapper,SLOT(slotRevert()),m_filesAction,"make_svn_revert");
    m_ResolvedAction = new KAction("Resolve recursive",KShortcut(),
        this,SLOT(slotResolved()),m_filesAction,"make_resolved");
    m_ResolvedAction->setToolTip(i18n("Marking files or dirs resolved"));
    m_IgnoreAction = new KAction(i18n("Ignore/Unignore current item"),KShortcut(),this,SLOT(slotIgnore()),m_filesAction,"make_svn_ignore");

    m_UpdateHead = new KAction("Update to head","svnupdate",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateHeadRec()),m_filesAction,"make_svn_headupdate");
    m_UpdateRev = new KAction("Update to revision...","svnupdate",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateTo()),m_filesAction,"make_svn_revupdate");
    m_commitAction = new KAction("Commit","svncommit",
        KShortcut("#"),m_SvnWrapper,SLOT(slotCommit()),m_filesAction,"make_svn_commit");
    m_simpleDiffHead = new KAction(i18n("Diff against head"),"svndiff",
        KShortcut(CTRL+Key_H),this,SLOT(slotSimpleDiff()),m_filesAction,"make_svn_headdiff");
    m_MergeRevisionAction = new KAction(i18n("Merge two revisions"),"merge",
        KShortcut(),this,SLOT(slotMergeRevisions()),m_filesAction,"make_svn_merge_revisions");
    m_MergeRevisionAction->setToolTip(i18n("Merge two revisions of these entry into itself"));

    /* remote actions only */
    m_CheckoutCurrentAction = new KAction("Checkout current repository path","svncheckout",KShortcut(),
        m_SvnWrapper,SLOT(slotCheckoutCurrent()),m_filesAction,"make_svn_checkout_current");
    m_ExportCurrentAction = new KAction(i18n("Export current repository path"),"svnexport",KShortcut(),
        m_SvnWrapper,SLOT(slotExportCurrent()),m_filesAction,"make_svn_export_current");
    new KAction(i18n("Select browse revision"),KShortcut(),this,SLOT(slotSelectBrowsingRevision()),m_filesAction,"switch_browse_revision");

    /* independe actions */
    m_CheckoutAction = new KAction(i18n("Checkout a repository"),"svncheckout",
        KShortcut(),m_SvnWrapper,SLOT(slotCheckout()),m_filesAction,"make_svn_checkout");
    m_ExportAction = new KAction(i18n("Export a repository"),"svnexport",
        KShortcut(),m_SvnWrapper,SLOT(slotExport()),m_filesAction,"make_svn_export");
    m_RefreshViewAction = new KAction(i18n("Refresh view"),"reload",KShortcut(Key_F5),this,SLOT(refreshCurrentTree()),m_filesAction,"make_view_refresh");

    new KAction(i18n("Diff revisions"),"svndiff",KShortcut(),this,SLOT(slotDiffRevisions()),m_filesAction,"make_revisions_diff");

    enableActions();
    m_filesAction->setHighlightingEnabled(true);
}

KActionCollection*kdesvnfilelist::filesActions()
{
    return m_filesAction;
}

FileListViewItemList* kdesvnfilelist::allSelected()
{
    if (!m_SelectedItems) {
        m_SelectedItems = new FileListViewItemList;
    }
    return m_SelectedItems;
}

void kdesvnfilelist::SelectionList(SvnItemList*target)
{
    if (!m_SelectedItems||!target) return;
    FileListViewItemListIterator iter(*m_SelectedItems);
    FileListViewItem*cur;
    while ( (cur=iter.current())!=0) {
        ++iter;
        target->append(cur);
    }
}

SvnItem*kdesvnfilelist::SelectedOrMain()
{
    if (singleSelected()!=0) {
        return singleSelected();
    }
    if (isWorkingCopy()&&firstChild()) {
        return static_cast<FileListViewItem*>(firstChild());
    }
    return 0;
}

QWidget*kdesvnfilelist::realWidget()
{
    return this;
}

FileListViewItem* kdesvnfilelist::singleSelected()
{
    if (m_SelectedItems && m_SelectedItems->count()==1) {
        return m_SelectedItems->at(0);
    }
    return 0;
}

SvnItem*kdesvnfilelist::Selected()
{
    return singleSelected();
}

bool kdesvnfilelist::openURL( const KURL &url,bool noReinit )
{
    m_SvnWrapper->killallThreads();
    clear();
    m_Dirsread.clear();
    if (m_SelectedItems) {
        m_SelectedItems->clear();
    }
    m_LastException="";
    delete m_pList->m_DirWatch;
    m_pList->m_DirWatch=0;

    if (!noReinit) m_SvnWrapper->reInitClient();

    setBaseUri(url.url());
    setWorkingCopy(false);
    setNetworked(false);

    m_pList->m_remoteRevision=svn::Revision::HEAD;

    QString query = url.query();

    if (!QString::compare("svn+file",url.protocol())) {
        setBaseUri("file://"+url.path());
    } else {
        if (url.isLocalFile()) {
            setBaseUri(url.path());
            if (m_SvnWrapper->isLocalWorkingCopy(url)) {
                setWorkingCopy(true);
            } else {
                // yes! KURL sometimes makes a correct localfile url (file:///)
                // to a simple file:/ - that brakes subverision lib. so we make sure
                // that we have a correct url
                setBaseUri("file://"+baseUri());
            }
        } else {
            setNetworked(true);
        }
    }
    if (query.length()>1) {
        QMap<QString,QString> q = url.queryItems();
        if (q.find("rev")!=q.end()) {
            QString v = q["rev"];
            svn::Revision tmp;
            m_SvnWrapper->svnclient()->url2Revision(v,m_pList->m_remoteRevision,tmp);
            if (m_pList->m_remoteRevision==svn::Revision::UNDEFINED) {
                m_pList->m_remoteRevision = svn::Revision::HEAD;
            }
        }
    }

    if (url.protocol()=="svn+ssh") {
        SshAgent ssh;
        ssh.addSshIdentities();
    }
    m_SvnWrapper->clearUpdateCache();
    if (isWorkingCopy()) {
        m_SvnWrapper->createModifiedCache(baseUri());
        m_pList->m_DirWatch=new KDirWatch(this);
        kdDebug()<<"Create dirwatch"<<endl;
        connect(m_pList->m_DirWatch,SIGNAL(dirty(const QString&)),this,SLOT(slotDirItemDirty(const QString&)));
        connect(m_pList->m_DirWatch,SIGNAL(created(const QString&)),this,SLOT(slotDirItemCreated(const QString&)));
        connect(m_pList->m_DirWatch,SIGNAL(deleted(const QString&)),this,SLOT(slotDirItemDeleted(const QString&)));
        /* seems that recursive does not work */
        m_pList->m_DirWatch->addDir(baseUri()+"/",false,false);
        m_pList->m_DirWatch->startScan(true);
    }
    bool result = checkDirs(baseUri(),0);
    if (result && isWorkingCopy()) {
        if (firstChild()) firstChild()->setOpen(true);
    }
    if (!result) {
        setBaseUri("");
        setNetworked(false);
        clear();
    }
    enableActions();
    m_pList->m_fileTip->setOptions(!isNetworked()&&Settings::display_file_tips()&&
        QToolTip::isGloballyEnabled(),true,6);

    if (isWorkingCopy()) {
        slotCheckUpdates();
    }
    emit changeCaption(baseUri());
    emit sigUrlOpend(result);
    return result;
}

void kdesvnfilelist::closeMe()
{
    m_SvnWrapper->killallThreads();
    selectAll(false);
    clear();
    setWorkingCopy("");
    setNetworked(false);
    setWorkingCopy(false);
    setBaseUri("");

    emit changeCaption("");
    emit sigUrlOpend(false);

    enableActions();
    m_SvnWrapper->reInitClient();
    delete m_pList->m_DirWatch;
    m_pList->m_DirWatch = 0;
    m_pList->m_fileTip->setItem(0);
}

bool kdesvnfilelist::checkDirs(const QString&_what,FileListViewItem * _parent)
{
    QString what = _what;
    svn::StatusEntries dlist;
    while (what.endsWith("/")) {
        what.truncate(what.length()-1);
    }
    kdDebug()<<"checkDirs()" << endl;
    // prevent this from checking unversioned folder. FIXME: what happen when we do open url on a non-working-copy folder??
    if (!isWorkingCopy()|| (!_parent) || ((_parent) && (_parent->isVersioned()))) {
        if (!m_SvnWrapper->makeStatus(what,dlist,m_pList->m_remoteRevision)) {
            kdDebug() << "unable makeStatus" <<endl;
            return false;
        }
    } else {
        checkUnversionedDirs(_parent);
        return true;
    }

    kdDebug() << "makeStatus on " << what << " created: " << dlist.count() << "items" <<endl;

    viewport()->setUpdatesEnabled(false);
    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem * pitem = 0;
    bool main_found = false;
    for (;it!=dlist.end();++it) {
        kdDebug() << "iterate over it: " << (*it).entry().url() << endl;

        // current item is not versioned
        if (!(*it).isVersioned()) {
            kdDebug()<< "Found non versioned item" << endl;
            // if empty, we may want to create a default svn::Status for each folder inside this _parent
            // iterate over QDir and create new filelistviewitem
            checkUnversionedDirs(_parent);
        }

        if ((*it).path()==what||QString::compare((*it).entry().url(),what)==0){
            if (!_parent) {
                pitem = new FileListViewItem(this,*it);
//                kdDebug()<< "CheckDirs::creating new FileListViewitem as parent " + (*it).path() << endl;
                m_Dirsread[pitem->fullName()]=true;
                pitem->setDropEnabled(true);
            }
            dlist.erase(it);
            main_found = true;
            break;
        }
    }
    if (_parent) {
        pitem = _parent;
    }
    insertDirs(pitem,dlist);
    viewport()->setUpdatesEnabled(true);
    viewport()->repaint();
    return true;
}

void kdesvnfilelist::insertDirs(FileListViewItem * _parent,svn::StatusEntries&dlist)
{
    svn::StatusEntries::iterator it;
    for (it = dlist.begin();it!=dlist.end();++it) {
        FileListViewItem * item;
        if (!_parent) {
            item = new FileListViewItem(this,*it);
//            kdDebug()<< "creating new FileListViewitem " + item->fullName() << endl;
        } else {
            item = new FileListViewItem(this,_parent,*it);
//            kdDebug()<< "creating new FileListViewitem (with parent) " + item->fullName() << endl;
        }
        if (item->isDir()) {
            m_Dirsread[item->fullName()]=false;
            item->setDropEnabled(true);
            if (isWorkingCopy()) {
                m_pList->m_DirWatch->addDir(item->fullName());
            }
//            kdDebug()<< "Watching folder: " + item->fullName() << endl;
        } else if (isWorkingCopy()) {
            m_pList->m_DirWatch->addFile(item->fullName());
//            kdDebug()<< "Watching file: " + item->fullName() << endl;
        }
    }
}

/* newdir is the NEW directory! just required if local */
void kdesvnfilelist::slotDirAdded(const QString&newdir,FileListViewItem*k)
{
    if (k) {
        k->refreshStatus();
    }
    if (!isWorkingCopy()) {
        if (k) {
            k->removeChilds();
            m_Dirsread[k->fullName()]=false;
            slotItemClicked(k);
            return;
        }
        QListViewItem*temp;
        while ((temp=firstChild())) {
            delete temp;
        }
        m_Dirsread.clear();
        checkDirs(baseUri(),0);
        return;
    }
    svn::Status stat;
    try {
        stat = m_SvnWrapper->svnclient()->singleStatus(newdir);
    } catch (svn::ClientException e) {
        m_LastException = QString::fromUtf8(e.message());
        emit sigLogMessage(m_LastException);
        return;
    }
    FileListViewItem * item,*pitem;
    pitem = k;
    if (!pitem) {
        pitem = (FileListViewItem*)firstChild();
        if (pitem->fullName()!=baseUri()) {
            pitem = 0;
        }
    }
    if (!pitem) {
        item = new FileListViewItem(this,stat);
    } else {
        item = new FileListViewItem(this,pitem,stat);
    }
    if (item->isDir()) {
        m_Dirsread[item->fullName()]=false;
        item->setDropEnabled(true);
        if (isWorkingCopy()) {
            m_pList->m_DirWatch->addDir(item->fullName());
        }
    } else if (isWorkingCopy()) {
        m_pList->m_DirWatch->addFile(item->fullName());
    }
}

kdesvnfilelist::~kdesvnfilelist()
{
    delete m_pList;
    delete m_SelectedItems;
    SshAgent ssh;
    ssh.killSshAgent();
}

void kdesvnfilelist::slotItemClicked(QListViewItem*aItem)
{
    if (!aItem) return;
    FileListViewItem* k = static_cast<FileListViewItem*>( aItem );
    bool _ex = true;
    if (isWorkingCopy()) {
        QDir d(k->fullName()); //FIXME: remove this as soon as we've been able to set entry->kind in Checkdirs
        _ex = k->isDir()||d.exists();
    } else {
        _ex = k->isDir();
    }

    if (_ex &&(m_Dirsread.find(k->fullName())==m_Dirsread.end()||m_Dirsread[k->fullName()]==false)) {
        if (checkDirs(k->fullName(),k)) {
            m_Dirsread[k->fullName()]=true;
        }
    }
}

void kdesvnfilelist::slotReinitItem(SvnItem*item)
{
    if (!item) {
        kdDebug()<<"kdesvnfilelist::slotReinitItem(SvnItem*item): item == null" << endl;
        return;
    }
    FileListViewItem*k = item->fItem();
    if (!k) {
        kdDebug()<<"kdesvnfilelist::slotReinitItem(SvnItem*item): k == null" << endl;
    }
    refreshItem(k);
    if (!k) {
        return;
    }
    if (k->isDir()) {
        k->removeChilds();
        m_Dirsread[k->fullName()]=false;;
    }
    slotItemClicked(k);
}

void kdesvnfilelist::enableActions()
{
    bool isopen = baseUri().length()>0;
    bool single = allSelected()->count()==1&&isopen;
    bool multi = allSelected()->count()>1&&isopen;
    bool none = allSelected()->count()==0&&isopen;
    bool dir = false;
    if (single && allSelected()->at(0)->isDir()) {
        dir = true;
    }
    KAction * temp = 0;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    m_LogRangeAction->setEnabled(single||(isWorkingCopy()&&!single&&!multi&&isopen));
    m_LogFullAction->setEnabled(single||(isWorkingCopy()&&!single&&!multi&&isopen));
    m_propertyAction->setEnabled(single);
    m_DelCurrent->setEnabled( (multi||single));
    m_LockAction->setEnabled( (multi||single));
    m_UnlockAction->setEnabled( (multi||single));
    m_IgnoreAction->setEnabled((single)&&singleSelected()->parent()!=0&&!singleSelected()->isRealVersioned());

    m_RenameAction->setEnabled(single && (!isWorkingCopy()||singleSelected()!=firstChild()));
    m_CopyAction->setEnabled(single && (!isWorkingCopy()||singleSelected()!=firstChild()));

    /* 2. only on files */
    m_BlameAction->setEnabled(single&&!dir);
    m_BlameRangeAction->setEnabled(single&&!dir);
    m_CatAction->setEnabled(single&&!dir);
    /* 3. actions only on dirs */
    m_MkdirAction->setEnabled(dir||!isWorkingCopy()&&isopen);
    m_switchRepository->setEnabled(isWorkingCopy()&& (single||none));
    m_changeToRepository->setEnabled(isWorkingCopy());
    m_ImportDirsIntoCurrent->setEnabled(dir);
    temp = filesActions()->action("make_svn_relocate");
    if (temp) {
        temp->setEnabled(isWorkingCopy()&& (single||none));
    }
    m_ExportCurrentAction->setEnabled(((single&&dir)||none));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent->setEnabled( (multi||single) && isWorkingCopy());
    m_RevertAction->setEnabled( (multi||single) && isWorkingCopy());
    m_ResolvedAction->setEnabled( (multi||single) && isWorkingCopy());
    m_InfoAction->setEnabled( (single||multi));
    m_MergeRevisionAction->setEnabled(single&&isWorkingCopy());
    temp = filesActions()->action("make_svn_addrec");
    if (temp) {
        temp->setEnabled( (multi||single) && isWorkingCopy());
    }

    m_UpdateHead->setEnabled(isWorkingCopy()&&isopen);
    m_UpdateRev->setEnabled(isWorkingCopy()&&isopen);
    m_commitAction->setEnabled(isWorkingCopy()&&isopen);
    m_simpleDiffHead->setEnabled(isWorkingCopy()&&isopen);

    /* 2. on dirs only */
    m_CleanupAction->setEnabled(isWorkingCopy()&&dir);

    /* remote actions only */
    m_CheckoutCurrentAction->setEnabled( ((single&&dir)||none) && !isWorkingCopy());
    /* independ actions */
    m_CheckoutAction->setEnabled(true);
    m_ExportAction->setEnabled(true);
    m_RefreshViewAction->setEnabled(isopen);

    temp = filesActions()->action("make_revisions_diff");
    if (temp) {
        temp->setEnabled(isopen);
    }
    temp = filesActions()->action("make_revisions_cat");
    if (temp) {
        temp->setEnabled(isopen && !dir && single);
    }
    temp = filesActions()->action("switch_browse_revision");
    if (temp) {
        temp->setEnabled(!isWorkingCopy()&&isopen);
    }
    temp = filesActions()->action("make_check_updates");
    if (temp) {
        temp->setEnabled(isWorkingCopy()&&isopen);
    }
}

void kdesvnfilelist::slotSelectionChanged()
{
    if (m_SelectedItems==0) {
        m_SelectedItems = new FileListViewItemList;
        m_SelectedItems->setAutoDelete(false);
    }
    m_SelectedItems->clear();

    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        m_SelectedItems->append( static_cast<FileListViewItem*>(it.current()) );
        ++it;
    }
    enableActions();
}

#include "kdesvnfilelist.moc"


/*!
    \fn kdesvnfilelist::slotClientException(const QString&)
 */
void kdesvnfilelist::slotClientException(const QString&what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(0,what,i18n("SVN Error"));
}


/*!
    \fn kdesvnfilelist::slotNotifyMessage(const QString&)
 */
void kdesvnfilelist::slotNotifyMessage(const QString&what)
{
    emit sigLogMessage(what);
    kapp->processEvents(20);
}

void kdesvnfilelist::slotChangeToRepository()
{
    if (!isWorkingCopy()) {
        return;
    }
    FileListViewItem*k = static_cast<FileListViewItem*>(firstChild());
    /* huh... */
    if (!k||!k->isDir()) return;
    KURL nurl = k->Url();
    sigSwitchUrl(nurl);
}

void kdesvnfilelist::slotItemDoubleClicked(QListViewItem*item)
{
    if (!item) return;

    FileListViewItem*fki = static_cast<FileListViewItem*>(item);
    if (fki->isDir()) {
        if (fki->isOpen()) {
            fki->setOpen(false);
        } else {
            fki->setOpen(true);
        }
        return;
    }
    if (isWorkingCopy()) {
        KFileItem fitem(KFileItem::Unknown,KFileItem::Unknown,fki->fullName());
        fitem.run();
    } else {
        QByteArray content = m_SvnWrapper->makeGet(m_pList->m_remoteRevision,fki->fullName());
        if (content.size()==0) return;
        new RunTempfile(content);
    }
}

void kdesvnfilelist::slotCleanupAction()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which||!which->isDir()) return;
    m_SvnWrapper->slotCleanup(which->fullName());
    which->refreshStatus(true);
}

void kdesvnfilelist::slotResolved()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which) return;
    m_SvnWrapper->slotResolved(which->fullName());
    which->refreshStatus(true);
}

template<class T> KDialogBase* kdesvnfilelist::createDialog(T**ptr,const QString&_head,bool OkCancel,const char*name)
{
    KDialogBase * dlg = new KDialogBase(
        0,
        name,
        true,
        _head,
        (OkCancel?KDialogBase::Ok|KDialogBase::Cancel:KDialogBase::Ok));

    if (!dlg) return dlg;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    *ptr = new T(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Settings::self()->config()),name?name:"standard_size"));
    return dlg;
}

void kdesvnfilelist::slotImportDirsIntoCurrent()
{
    slotImportIntoCurrent(true);
}

/*!
    \fn kdesvnfilelist::slotImportIntoCurrent()
 */
void kdesvnfilelist::slotImportIntoCurrent(bool dirs)
{
    if (allSelected()->count()>1) {
        KMessageBox::error(this,i18n("Cannot import into multiple targets!"));
        return;
    }
    QString targetUri;
    if (allSelected()->count()==0) {
        targetUri=baseUri();
    } else {
        targetUri = allSelected()->at(0)->Url();
    }
    KURL uri;
    if (dirs) uri = KFileDialog::getExistingDirectory(QString::null,this,"Import files from folder");
    else uri = KFileDialog::getImageOpenURL(QString::null,this,"Import file");

    if (uri.url().isEmpty()) return;

    if ( !uri.protocol().isEmpty() && uri.protocol()!="file") {
        KMessageBox::error(this,i18n("Cannot import into remote targets!"));
        return;
    }
    slotImportIntoDir(uri,targetUri,dirs);
}

void kdesvnfilelist::slotImportIntoDir(const KURL&importUrl,const QString&target,bool dirs)
{
    Logmsg_impl*ptr;
    Importdir_logmsg*ptr2 = 0;

    KDialogBase*dlg;
    KURL uri = importUrl;
    QString targetUri = target;
    while (targetUri.endsWith("/")) {
        targetUri.truncate(targetUri.length()-1);
    }

    if (dirs) {
         dlg = createDialog(&ptr2,QString(i18n("Import log")),true,"import_log_msg");
         ptr = ptr2;
         ptr2->createDirboxDir("\""+uri.fileName(true)+"\"");
    } else {
        dlg = createDialog(&ptr,QString(i18n("Import log")),true,"import_log_msg");
    }

    if (!dlg) return;

    ptr->initHistory();

    if (dlg->exec()!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"import_log_msg",false);

    QString logMessage = ptr->getMessage();
    bool rec = ptr->isRecursive();
    ptr->saveHistory();
    uri.setProtocol("");
    QString iurl = uri.path();
    while (iurl.endsWith("/")) {
        iurl.truncate(iurl.length()-1);
    }

    if (dirs && ptr2 && ptr2->createDir()) {
        targetUri+= "/"+uri.fileName(true);
    }
    m_SvnWrapper->slotImport(iurl,targetUri,logMessage,rec);

    if (!isWorkingCopy()) {
        if (allSelected()->count()==0) {
            refreshCurrentTree();
        } else {
            slotReinitItem(allSelected()->at(0));
        }
    }
    delete dlg;
}

void kdesvnfilelist::refreshCurrentTree()
{
    FileListViewItem*item = static_cast<FileListViewItem*>(firstChild());
    if (!item) return;
    kapp->processEvents();
    setUpdatesEnabled(false);
    if (isWorkingCopy()) {
        m_SvnWrapper->createModifiedCache(baseUri());
    }
    if (item->fullName()==baseUri()) {
        if (!refreshItem(item)) {
            setUpdatesEnabled(true);
            viewport()->repaint();
            return;
        } else {
            refreshRecursive(item);
        }
    } else {
        refreshRecursive(0);
    }
    setUpdatesEnabled(true);
    viewport()->repaint();
}

void kdesvnfilelist::refreshCurrent(SvnItem*cur)
{
    if (!cur||!cur->fItem()) {
        refreshCurrentTree();
        return;
    }
    kapp->processEvents();
    setUpdatesEnabled(false);
    refreshRecursive(cur->fItem());
    setUpdatesEnabled(true);
    viewport()->repaint();
}

void kdesvnfilelist::refreshRecursive(FileListViewItem*_parent,bool down)
{
    FileListViewItem*item;
    if (_parent) {
        item = static_cast<FileListViewItem*>(_parent->firstChild());
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }

    if (!item) return;
    kapp->processEvents();

    FileListViewItemList currentSync;
    currentSync.setAutoDelete(false);

    while (item) {
        currentSync.append(item);
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }

    QString what = (_parent!=0?_parent->fullName():baseUri());
    svn::StatusEntries dlist;

    if (!m_SvnWrapper->makeStatus(what,dlist,m_pList->m_remoteRevision)) {
        return;
    }

    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem*k;
    bool gotit = false;
    for (;it!=dlist.end();++it) {
        gotit = false;
        if ((*it).path()==what) {
            continue;
        }
        FileListViewItemListIterator clistIter(currentSync);
        while ( (k=clistIter.current()) ) {
            ++clistIter;
            if (k->fullName()==(*it).path()) {
                currentSync.removeRef(k);
                k->updateStatus(*it);
                gotit = true;
                break;
            }
        }
        if (!gotit) {
            FileListViewItem * item;
            if (!_parent) {
                item = new FileListViewItem(this,*it);
            } else {
                item = new FileListViewItem(this,_parent,*it);
            }
            if (item->isDir()) {
                m_Dirsread[item->fullName()]=false;
                item->setDropEnabled(true);
            }
            if (isWorkingCopy()) {
                if (item->isDir()) {
                    m_pList->m_DirWatch->addDir(item->fullName());
                } else {
                    m_pList->m_DirWatch->addFile(item->fullName());
                }
            }
        }
    }
    FileListViewItemListIterator dIter(currentSync);
#ifndef NDEBUG
    slotSelectionChanged();
    kdDebug() << "Selected items " << m_SelectedItems->count()<< endl;
#endif
    while ( (k=dIter.current()) ) {
        ++dIter;
        delete k;
        // @todo just for debugging!
#ifndef NDEBUG
        m_SelectedItems->clear();
        QListViewItemIterator qlvit( this, QListViewItemIterator::Selected );
        while ( qlvit.current() ) {
            m_SelectedItems->append( static_cast<FileListViewItem*>(qlvit.current()) );
            ++qlvit;
        }
        kdDebug() << "Selected items " << m_SelectedItems->count() << endl;
#endif
    }
    if (_parent) {
        item = static_cast<FileListViewItem*>(_parent->firstChild());
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }
    if (!down) {
        return;
    }
    while (item) {
        if (item->isDir()) {
            if ((m_Dirsread.find(item->fullName())!=m_Dirsread.end()&&m_Dirsread[item->fullName()]==true)) {
                if (item->childCount()==0) {
                    checkDirs(item->fullName(),item);
                } else {
                    refreshRecursive(item);
                }
            }
        }
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }
}

void kdesvnfilelist::slotRightButton(QListViewItem *_item, const QPoint &, int)
{
    FileListViewItem*item = static_cast<FileListViewItem*>(_item);
    bool isopen = baseUri().length()>0;
    if (item) {
        if (isWorkingCopy()) {
            emit sigShowPopup("local_context");
        } else {
            emit sigShowPopup("remote_context");
        }
    } else {
        if (!isopen) {
            emit sigShowPopup("general_empty");
        } else {
            if (isWorkingCopy()) {
                emit sigShowPopup("general_local");
            } else {
                emit sigShowPopup("general_remote");
            }
        }
    }
}

/**
* Overridden virtuals for Qt drag 'n drop (XDND)
*/
void kdesvnfilelist::contentsDragEnterEvent(QDragEnterEvent *event)
{
    QListViewItem*item;
    bool ok = validDropEvent(event,item);
    if (ok) {
        event->accept();
    } else {
        event->ignore();
    }
}

void kdesvnfilelist::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    cleanHighLighter();
}

bool kdesvnfilelist::acceptDrag(QDropEvent *event)const
{
    return KURLDrag::canDecode(event);
}

bool kdesvnfilelist::validDropEvent(QDropEvent*event,QListViewItem*&item)
{
    if (!event) return false;
    bool ok = false;
    item = 0;
    if (KURLDrag::canDecode(event)) {
        KURL::List urlList;
        KURLDrag::decode( event, urlList );
        int count = urlList.count();
        if (count>0) {
            if (baseUri().length()==0) {
                ok = true;
            } else {
                QPoint vp = contentsToViewport( event->pos() );
                item = isExecuteArea( vp ) ? itemAt( vp ) : 0L;
                FileListViewItem*which=static_cast<FileListViewItem*>(item);
                if (!isWorkingCopy()){
                    ok = (!item || (which->isDir()))&&urlList[0].isLocalFile()&&count==1;
                } else {
                    ok = (which && (which->isDir()))/*&&urlList[0].isLocalFile()*/;
                }
            }
        }
    }
    return ok;
}

void kdesvnfilelist::contentsDropEvent(QDropEvent * event)
{
    QListViewItem *item = 0;
    bool ok = validDropEvent(event,item);
    cleanHighLighter();
    if (ok) {
        event->accept();
        dropped(event,item);
    } else {
        event->ignore();
    }
}

void kdesvnfilelist::contentsDragMoveEvent( QDragMoveEvent* event)
{
    QListViewItem * item;
    bool ok = validDropEvent(event,item);

    if (item!=m_pList->dragOverItem) {
        QPoint vp = contentsToViewport( event->pos() );
        m_pList->dragOverItem=item;
        m_pList->dragOverPoint = vp;
        QRect tmpRect = drawItemHighlighter(0, m_pList->dragOverItem);
        if (tmpRect!=m_pList->mOldDropHighlighter) {
            cleanHighLighter();
            m_pList->mOldDropHighlighter=tmpRect;
            viewport()->repaint(tmpRect);
            kapp->processEvents();
        }
    }
    if (ok) {
        event->accept();
    } else {
        event->ignore();
    }
}

void kdesvnfilelist::viewportPaintEvent(QPaintEvent *ev)
{
    KListView::viewportPaintEvent(ev);
    if (m_pList->mOldDropHighlighter.isValid() && ev->rect().intersects(m_pList->mOldDropHighlighter)) {
        QPainter painter(viewport());
        style().drawPrimitive(QStyle::PE_FocusRect, &painter, m_pList->mOldDropHighlighter, colorGroup(),
                QStyle::Style_FocusAtBorder);
    }
}

void kdesvnfilelist::cleanHighLighter()
{
    if (m_pList->mOldDropHighlighter.isValid()) {
        QRect rect=m_pList->mOldDropHighlighter;
        m_pList->mOldDropHighlighter=QRect();
        viewport()->repaint(rect, true);
    }
}

/*!
    \fn kdesvnfilelist::slotMergeRevisions()
 */
void kdesvnfilelist::slotMergeRevisions()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which) {
        return;
    }
    bool force,dry,rec,irelated;
    Rangeinput_impl::revision_range range;
    if (!MergeDlg_impl::getMergeRange(range,&force,&rec,&irelated,&dry,this,"merge_range")) {
        return;
    }
    m_SvnWrapper->slotMergeWcRevisions(which->fullName(),range.first,range.second,rec,irelated,force,dry);
    refreshItem(which);
    refreshRecursive(which);
}

void kdesvnfilelist::slotDropped(QDropEvent* event,QListViewItem*item)
{
    KURL::List urlList;
    if (!KURLDrag::decode( event, urlList )||urlList.count()<1) {
        return;
    }

    if (event->source()!=this) {
        kdDebug()<<"Dropped from outside" << endl;
        if (baseUri().length()==0) {
            openURL(urlList[0]);
            return;
        }
        if (baseUri().length()>0 /*&& urlList[0].isLocalFile()*/) {
            QString path = urlList[0].path();
            QFileInfo fi(path);
            QString tdir;
            if (item) {
                FileListViewItem*which = static_cast<FileListViewItem*>(item);
                clearSelection();
                which->setSelected(true);
                kapp->processEvents();
                tdir = which->fullName();
            } else {
                tdir = baseUri();
            }
            if  (!isWorkingCopy()) {
                slotImportIntoDir(urlList[0],tdir,fi.isDir());
            } else {
                if (m_pList->m_DirWatch) {
                    m_pList->m_DirWatch->stopScan();
                }
                KIO::Job * job = 0L;
                job = KIO::copy(urlList,tdir);
                connect( job, SIGNAL( result( KIO::Job * ) ),SLOT( slotCopyFinished( KIO::Job * ) ) );
                dispDummy();
                return;
            }
        }
    }
}

/*!
    \fn kdesvnfilelist::slotRename()
 */
void kdesvnfilelist::slotRename()
{
    copy_move(true);
}
void kdesvnfilelist::slotCopy()
{
    copy_move(false);
}

void kdesvnfilelist::copy_move(bool move)
{
    if (isWorkingCopy()&&singleSelected()==firstChild()) {
        return;
    }
    bool ok, force;
    FileListViewItem*which = singleSelected();
    if (!which) return;
    QString nName =  CopyMoveView_impl::getMoveCopyTo(&ok,&force,move,
        which->fullName(),baseUri(),this,"move_name");
    if (!ok) {
        return;
    }
    m_SvnWrapper->slotCopyMove(move,which->fullName(),nName,force);
}

void kdesvnfilelist::slotCat()
{
    FileListViewItem*k = singleSelected();
    if (!k) return;
    m_SvnWrapper->makeCat(isWorkingCopy()?svn::Revision::HEAD:m_pList->m_remoteRevision, k->fullName(),k->text(0));
}


/*!
    \fn kdesvnfilelist::slotCopyFinished( KIO::Job *)
 */
void kdesvnfilelist::slotCopyFinished( KIO::Job * job)
{
    if (m_pList->m_DirWatch) {
        m_pList->m_DirWatch->startScan(false);
    }
    if (job) {
        bool ok = true;
        qApp->exit_loop();
        if (job->error()) {
            job->showErrorDialog(this);
            ok = false;
        }
        // always just connect a CopyJob here!!!!
        if (ok) {
            KURL::List lst = static_cast<KIO::CopyJob*>(job)->srcURLs();
            KURL turl = static_cast<KIO::CopyJob*>(job)->destURL();
            QString base = turl.path(1);
            KURL::List::iterator iter;
            QValueList<svn::Path> tmp;
            for (iter=lst.begin();iter!=lst.end();++iter) {
                tmp.push_back(svn::Path((base+(*iter).fileName(true))));
            }
            m_SvnWrapper->addItems(tmp,true);
        }
        refreshCurrentTree();
    }
}



/*!
    \fn kdesvnfilelist::slotDelete()
 */
void kdesvnfilelist::slotDelete()
{
    m_deletePerfect = true;
    QPtrList<FileListViewItem>*lst = allSelected();

    if (lst->count()==0) {
        KMessageBox::error(this,i18n("Nothing selected for delete"));
        return;
    }
    FileListViewItemListIterator liter(*lst);
    FileListViewItem*cur;

    QValueList<svn::Path> items;
    QStringList displist;
    KURL::List kioList;
    while ((cur=liter.current())!=0){
        ++liter;
        if (!cur->isRealVersioned()) {
            kioList.append(cur->fullName());
        } else {
            items.push_back(cur->fullName());
        }
        displist.append(cur->fullName());
    }
    int answer = KMessageBox::questionYesNoList(this,i18n("Really delete these entries?"),displist,i18n("Delete from repository"));
    if (answer!=KMessageBox::Yes) {
        return;
    }
    if (kioList.count()>0) {
        KIO::Job*aJob = KIO::del(kioList);
        connect(aJob,SIGNAL(result (KIO::Job *)),this,SLOT(slotDeleteFinished(KIO::Job*)));
        dispDummy();
    }
    if (m_deletePerfect && items.size()>0) {
        m_SvnWrapper->makeDelete(items);
    }
    refreshCurrentTree();
}

/*!
    \fn kdesvnfilelist::slotDeleteFinished(KIO::Job*)
 */
void kdesvnfilelist::slotDeleteFinished(KIO::Job*job)
{
    if (job) {
        qApp->exit_loop();
        if (job->error()) {
            job->showErrorDialog(this);
            m_deletePerfect = false;
        }
    }
}

/*!
    \fn kdesvnfilelist::dispDummy()
 */
void kdesvnfilelist::dispDummy()
{
    // wait for job
    QLabel dummy(this,0,WStyle_NoBorder|WShowModal);
    QSize csize = size();
    dummy.setText(i18n("Please wait until job is finished"));
    dummy.resize(dummy.minimumSizeHint());
    if (dummy.width()<=width()&&dummy.height()<=height()) {
        dummy.move(csize.width()/2-dummy.width()/2,csize.height()/2-dummy.height()/2);
    }
    dummy.show();
    qApp->enter_loop();
    dummy.hide();
}


/*!
    \fn kdesvnfilelist::slotLock()
 */
void kdesvnfilelist::slotLock()
{
    QPtrList<FileListViewItem>*lst = allSelected();
    FileListViewItemListIterator liter(*lst);
    FileListViewItem*cur;
    if (lst->count()==0) {
        KMessageBox::error(this,i18n("Nothing selected for lock"));
        return;
    }
    KDialogBase*dlg;
    Logmsg_impl*ptr;
    dlg = createDialog(&ptr,QString(i18n("Lock message")),true,"locking_log_msg");
    if (!dlg) return;
    ptr->initHistory();
    ptr->setRecCheckboxtext(i18n("Steal lock?"));

    if (dlg->exec()!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"locking_log_msg",false);

    QString logMessage = ptr->getMessage();
    bool rec = ptr->isRecursive();
    ptr->saveHistory();

    QStringList displist;
    while ((cur=liter.current())!=0){
        ++liter;
        displist.append(cur->fullName());
    }
    m_SvnWrapper->makeLock(displist,logMessage,rec);
    refreshCurrentTree();
}


/*!
    \fn kdesvnfilelist::slotUnlock()
 */
void kdesvnfilelist::slotUnlock()
{
    QPtrList<FileListViewItem>*lst = allSelected();
    FileListViewItemListIterator liter(*lst);
    FileListViewItem*cur;
    if (lst->count()==0) {
        KMessageBox::error(this,i18n("Nothing selected for unlock"));
        return;
    }
    int res = KMessageBox::questionYesNoCancel(this,i18n("Break lock or ignore missing locks?"),i18n("Unlocking items"));
    if (res == KMessageBox::Cancel) {
        return;
    }
    bool breakit = res==KMessageBox::Yes;

    QStringList displist;
    while ((cur=liter.current())!=0){
        ++liter;
        displist.append(cur->fullName());
    }
    m_SvnWrapper->makeUnlock(displist,breakit);
    refreshCurrentTree();
}


/*!
    \fn kdesvnfilelist::slotIgnore()
 */
void kdesvnfilelist::slotIgnore()
{
    SvnItem*item = singleSelected();
    if (!item || item->isRealVersioned()) return;
    if (m_SvnWrapper->makeIgnoreEntry(item,item->isIgnored())) {
        refreshCurrentTree();
    }
}


/*!
    \fn kdesvnfilelist::slotBlame()
 */
void kdesvnfilelist::slotBlame()
{
    SvnItem*k = singleSelected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    m_SvnWrapper->makeBlame(start,end,k);
}


/*!
    \fn kdesvnfilelist::slotRangeBlame()
 */
void kdesvnfilelist::slotRangeBlame()
{
    SvnItem*k = singleSelected();
    if (!k) return;
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_SvnWrapper->makeBlame(r.first,r.second,k);
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"revisions_dlg",false);
    delete dlg;
}


/*!
    \fn kdesvnfilelist::slotSimpleDiff()
 */
void kdesvnfilelist::slotSimpleDiff()
{
    FileListViewItemList*klist = allSelected();
    QStringList what;

    if (!klist||klist->count()==0) {
        what<<baseUri();
    }else{
        FileListViewItemListIterator liter(*klist);
        FileListViewItem*cur;
        while ((cur=liter.current())!=0){
            ++liter;
            what<<cur->fullName();
        }
    }

    m_SvnWrapper->makeDiff(what,svn::Revision::WORKING,svn::Revision::HEAD);
}


/*!
    \fn kdesvnfilelist::slotMkdir()
 */
void kdesvnfilelist::slotMkdir()
{
    SvnItem*k = singleSelected();
    QString parentDir;
    if (k) {
        if (!k->isDir()) {
            KMessageBox::sorry(0,i18n("May not make subdirs of a file"));
            return;
        }
        parentDir=k->fullName();
    } else {
        parentDir=baseUri();
    }
    QString ex = m_SvnWrapper->makeMkdir(parentDir);
    if (!ex.isEmpty()) {
        slotDirAdded(ex,static_cast<FileListViewItem*>(k));
    }
}


/*!
    \fn kdesvnfilelist::slotDiffRevisions()
 */
void kdesvnfilelist::slotDiffRevisions()
{
    SvnItem*k = singleSelected();
    QString what;
    if (!k) {
        what=baseUri();
    }else{
        what=k->fullName();
    }
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_SvnWrapper->makeDiff(what,r.first,r.second);
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"revisions_dlg",false);
    delete dlg;

}

void kdesvnfilelist::slotSelectBrowsingRevision()
{
    if (isWorkingCopy()) return;
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    rdlg->setStartOnly(true);
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_pList->m_remoteRevision= r.first;
        if (childCount()==0) {
            checkDirs(baseUri(),0);
        } else {
            refreshCurrentTree();
        }
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"revisions_dlg",false);
    delete dlg;
}

/*!
    \fn kdesvnfilelist::slotRevisionCat()
 */
void kdesvnfilelist::slotRevisionCat()
{
    SvnItem*k = singleSelected();
    if (!k) return;
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    rdlg->setStartOnly(true);
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_SvnWrapper->makeCat(r.first, k->fullName(),k->shortName());
    }
    dlg->saveDialogSize(*(Settings::self()->config()),"revisions_dlg",false);
    delete dlg;
}


/*!
    \fn kdesvnfilelist::refreshItem(FileListViewItem*)
 */
bool kdesvnfilelist::refreshItem(FileListViewItem*item)
{
    if (!item) {
        return false;
    }
    try {
        item->setStat(svnclient()->singleStatus(item->fullName(),false,m_pList->m_remoteRevision));
    } catch (svn::ClientException e) {
        item->setStat(svn::Status());
        return false;
    }
    return true;
}


/*!
    \fn kdesvnfilelist::slotCheckUpdates()
 */
void kdesvnfilelist::slotCheckUpdates()
{
    m_SvnWrapper->createUpdateCache(baseUri());
    //reinitItems(0);
    //rescanIconsRec(0L);
}


/*!
    \fn kdesvnfilelist::reinitItems(FileListViewItem*_item = 0)
 */
void kdesvnfilelist::reinitItems(FileListViewItem*_item)
{
    FileListViewItem*item;
    if (_item) {
        item = _item;
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }
    if (!item) {
        return;
    }
    item->init();
    item = static_cast<FileListViewItem*>(item->firstChild());
    while(item) {
        reinitItems(item);
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }
}


/*!
    \fn kdesvnfilelist::slotInfo()
 */
void kdesvnfilelist::slotInfo()
{
    QPtrList<SvnItem> lst;
    SelectionList(&lst);
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    if (!isWorkingCopy()) {
        rev = svn::Revision::HEAD;
    }
    if (lst.count()==0) {
        lst.append(SelectedOrMain());
    }
    m_SvnWrapper->makeInfo(lst,rev,peg,Settings::info_recursive());
}


/*!
    \fn kdesvnfilelist::slotDirItemCreated(const QString&)
 */
void kdesvnfilelist::slotDirItemCreated(const QString&what)
{
    m_pList->m_DirWatch->stopScan();
    m_pList->m_fileTip->setItem(0);
    FileListViewItem*item = findEntryItem(what);
    if (item) {
        refreshItem(item);
    } else {
        m_pList->m_DirWatch->removeDir(what);
        m_pList->m_DirWatch->removeFile(what);
    }
    m_pList->m_DirWatch->startScan();
}


void kdesvnfilelist::updateParents(FileListViewItem*item)
{
    if (!item || !item->parent()) return;
    FileListViewItem*it = static_cast<FileListViewItem*>(item->parent());
    it->update();
    updateParents(it);
}

/*!
    \fn kdesvnfilelist::slotDirItemDirty(const QString&)
 */
void kdesvnfilelist::slotDirItemDirty(const QString&what)
{
    m_pList->m_DirWatch->stopScan();
    m_pList->m_fileTip->setItem(0);
    FileListViewItem*item = findEntryItem(what);
    if (item) {
        refreshItem(item);
        if (!item->isNormal() && item->isRealVersioned()) {
             m_SvnWrapper->addModifiedCache(item->stat());
        } else {
            m_SvnWrapper->deleteFromModifiedCache(item->fullName());
        }
        if (item->isDir()) {
            refreshRecursive(item,false);
        }
        updateParents(item);
    } else {
        m_pList->m_DirWatch->removeDir(what);
        m_pList->m_DirWatch->removeFile(what);
        m_SvnWrapper->deleteFromModifiedCache(what);
    }
    m_pList->m_DirWatch->startScan();
}

/*!
    \fn kdesvnfilelist::slotDirItemDeleted(const QString&)
 */
void kdesvnfilelist::slotDirItemDeleted(const QString&what)
{
    m_pList->m_DirWatch->stopScan();
    m_pList->m_fileTip->setItem(0);
    FileListViewItem*item = findEntryItem(what);
    if (item) {
        refreshItem(item);
        if (!item->isNormal() && item->isRealVersioned()) {
             m_SvnWrapper->addModifiedCache(item->stat());
        } else {
            m_SvnWrapper->deleteFromModifiedCache(item->fullName());
        }
        updateParents(item);
    } else {
        m_pList->m_DirWatch->removeDir(what);
        m_pList->m_DirWatch->removeFile(what);
        m_SvnWrapper->deleteFromModifiedCache(what);
    }
    m_pList->m_DirWatch->startScan();
}

FileListViewItem* kdesvnfilelist::findEntryItem(const QString&what,FileListViewItem*startAt)
{
    if (!startAt && !what.startsWith(baseUri())) return 0;
    QString _what = what;
    FileListViewItem*_s,*_temp;
    if (!startAt) {
        while (_what.endsWith("/")) {
            _what.truncate(_what.length()-1);
        }
        _s = static_cast<FileListViewItem*>(firstChild());
    } else {
        _s = static_cast<FileListViewItem*>(startAt->firstChild());
    }
    _temp = 0;
    while (_s) {
        if (_s->fullName()==_what) {
            return _s;
        }
        if (_what.startsWith(_s->fullName())) {
            _temp = findEntryItem(_what,_s);
            if (_temp) {
                return _temp;
            }
        }
        _s = static_cast<FileListViewItem*>(_s->nextSibling());
    }
    return 0;
}


/*!
    \fn kdesvnfilelist::contentsMouseMoveEvent( QMouseEvent *e )
 */
void kdesvnfilelist::contentsMouseMoveEvent( QMouseEvent *e )
{
    if (Settings::display_file_tips()) {

        QPoint vp = contentsToViewport( e->pos() );
        FileListViewItem*item = isExecuteArea( vp ) ? static_cast<FileListViewItem*>(itemAt( vp )) : 0L;

        if (item) {
            vp.setY( itemRect( item ).y() );
            QRect rect( viewportToContents( vp ), QSize(20, item->height()) );
            m_pList->m_fileTip->setItem( static_cast<SvnItem*>(item), rect, item->pixmap(0));
            m_pList->m_fileTip->setPreview(KGlobalSettings::showFilePreview(item->fullName())&&isWorkingCopy()
                &&Settings::display_previews_in_file_tips());
            setShowToolTips(false);
        } else {
            m_pList->m_fileTip->setItem(0);
            setShowToolTips(true);
        }
    } else {
        m_pList->m_fileTip->setItem(0);
        setShowToolTips(true);
    }
    KListView::contentsMouseMoveEvent( e );
}


/*!
    \fn kdesvnfilelist::contentsWheelEvent( QWheelEvent * e )
 */
void kdesvnfilelist::contentsWheelEvent( QWheelEvent * e )
{
   // when scrolling with mousewheel, stop possible pending filetip
   m_pList->m_fileTip->setItem(0);
   KListView::contentsWheelEvent( e );
}

void kdesvnfilelist::leaveEvent(QEvent*e)
{
    m_pList->m_fileTip->setItem( 0 );
    KListView::leaveEvent( e );
}

void kdesvnfilelist::slotSettingsChanged()
{
    m_pList->m_fileTip->setOptions(!isNetworked()&&Settings::display_file_tips()&&
        QToolTip::isGloballyEnabled(),true,6);
    if (m_pList->reReadSettings()) {
        refreshCurrentTree();
    }
}


/*!
    \fn kdesvnfilelist::slotRelocate()
 */
void kdesvnfilelist::slotRelocate()
{
    if (!isWorkingCopy()) return;
    SvnItem*k = SelectedOrMain();
    if (!k) {
        KMessageBox::error(0,i18n("Error getting entry to relocate"));
        return;
    }
    QString path,fromUrl;
    path = k->fullName();
    fromUrl = k->Url();
    CheckoutInfo_impl*ptr;
    KDialogBase * dlg = createDialog(&ptr,i18n("Relocate working copy"),true,"relocate_dlg");
    if (dlg) {
        ptr->setStartUrl(fromUrl);
        ptr->forceAsRecursive(true);
        ptr->disableTargetDir(true);
        ptr->disableRange(true);
        ptr->disableOpen(true);
        bool done = false;
        if (dlg->exec()==QDialog::Accepted) {
            done = m_SvnWrapper->makeRelocate(fromUrl,ptr->reposURL(),path,ptr->forceIt());
        }
        dlg->saveDialogSize(*(Settings::self()->config()),"relocate_dlg",false);
        delete dlg;
        if (!done) return;
    }
    refreshItem(k->fItem());
}

void kdesvnfilelist::checkUnversionedDirs( FileListViewItem * _parent )
{
    QDir d;
    if (_parent)
        d.setPath(_parent->fullName()); //FIXME: this one is not reliable, what if _parent == 0??
    //             else
    //                 d.setPath(this->firstChild()->fullName());

    d.setFilter( QDir::Files | QDir::Dirs );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator nonversioned_it( *list );
    QFileInfo *fi;

    svn::StatusEntries nonversioned_list;

    // FIXME: create a dlist and feed to insertDirs, mean while .. we are copying insertDirs since we weren't able to set svn_node_kind into appropriate value
    while ( (fi = nonversioned_it.current()) != 0 ) {
        if ((fi->fileName()!=".") && (fi->fileName()!="..")) {
            // trying to set entry->kind
//             svn_wc_status2_t wc_stat;
//             svn_wc_entry_t entry;
//             char *temp;
//             strcpy(temp, fi->fileName());
//             entry.name = temp;
//
//             wc_stat.entry = &entry;
//             if (fi->isDir())
//                 entry.kind = svn_node_dir;
//             else
//                 entry.kind = svn_node_file;
//
//             svn::Status stat(fi->fileName(), &wc_stat);

            svn::Status stat(fi->absFilePath());

            // start copying insertDirs
            FileListViewItem * item;
            if (!_parent) {
                item = new FileListViewItem(this, stat);
                kdDebug()<< "creating new FileListViewitem " + item->fullName() << endl;
            } else {
                item = new FileListViewItem(this,_parent, stat);
                kdDebug()<< "creating new FileListViewitem (with parent) " + item->fullName() << endl;
            }
            if (fi->isDir()) {
                m_Dirsread[item->fullName()]=false;
                item->setDropEnabled(true);
                if (isWorkingCopy()) {
                    m_pList->m_DirWatch->addDir(item->fullName());
                }
                kdDebug()<< "Watching folder: " + item->fullName() << endl;
            } else if (isWorkingCopy()) {
                m_pList->m_DirWatch->addFile(item->fullName());
                kdDebug()<< "Watching file: " + item->fullName() << endl;
            }
            // end of copying insertDirs

            nonversioned_list.append(stat);
            kdDebug() << "creating new FileListViewItem from QDir entry: " << fi->fileName() << endl;
        }
        ++nonversioned_it;
    }

    // uncomment this if you've ben able to set svn_node_kind (see above)
    //this->insertDirs(_parent, nonversioned_list);
}

void kdesvnfilelist::rescanIconsRec(FileListViewItem*startAt)
{
    FileListViewItem*_s,*_temp;
    if (!startAt) {
        _s = static_cast<FileListViewItem*>(firstChild());
    } else {
        _s = static_cast<FileListViewItem*>(startAt->firstChild());
    }
    if (!_s) {
        return;
    }
    while (_s) {
        _s->makePixmap();
        rescanIconsRec(_s);
        _s = static_cast<FileListViewItem*>(_s->nextSibling());
    }
}

void kdesvnfilelist::slotRescanIcons()
{
    rescanIconsRec(0L);
}
