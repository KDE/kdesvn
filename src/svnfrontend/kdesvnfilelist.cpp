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
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "importdir_logmsg.h"
#include "copymoveview_impl.h"
#include "mergedlg_impl.h"
#include "svnactions.h"
#include "svncpp/revision.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"
#include "helpers/dirnotify.h"
#include "helpers/sshagent.h"
#include "helpers/stl2qt.h"
#include "helpers/runtempfile.h"
#include "kdesvn_part.h"

#include <qvbox.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kdirlister.h>
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

class KdesvnFileListPrivate{
public:
    KdesvnFileListPrivate();
    virtual ~KdesvnFileListPrivate(){};
    QListViewItem *dragOverItem;
    QPoint dragOverPoint;
    QRect mOldDropHighlighter;
    svn::Revision m_remoteRevision;
};

kdesvnfilelist::kdesvnfilelist(KActionCollection*aCollect,QWidget *parent, const char *name)
 : KListView(parent, name),ItemDisplay(),m_SvnWrapper(new SvnActions(this))
{
    m_SelectedItems = 0;
    m_baseUri="";
    m_pList = new KdesvnFileListPrivate;
    m_filesAction = aCollect;
    m_isLocal = false;

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
    connect(this,SIGNAL(dropped (QDropEvent*,QListViewItem*)),
            this,SLOT(slotDropped(QDropEvent*,QListViewItem*)));

//    m_DirNotify = new DirNotify();
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
    m_LogRangeAction = new KAction(i18n("&Log..."),"history",KShortcut(SHIFT+CTRL+Key_L),m_SvnWrapper,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    m_LogFullAction = new KAction(i18n("&Full Log"),"history",KShortcut(CTRL+Key_L),m_SvnWrapper,SLOT(slotMakeLog()),m_filesAction,"make_svn_log_full");
    m_propertyAction = new KAction(i18n("Properties"),"edit",
        KShortcut(Key_P),m_SvnWrapper,SLOT(slotProperties()),m_filesAction,"make_svn_property");
    m_InfoAction = new KAction(i18n("Details"),"svninfo",
        KShortcut(Key_I),m_SvnWrapper,SLOT(slotInfo()),m_filesAction,"make_svn_info");
    m_RenameAction = new KAction(i18n("Move"),"move",
        KShortcut(Key_F2),this,SLOT(slotRename()),m_filesAction,"make_svn_rename");
    m_CopyAction = new KAction(i18n("Copy"),"editcopy",
        KShortcut(Key_F5),this,SLOT(slotCopy()),m_filesAction,"make_svn_copy");
    new KAction(i18n("Check for updates"),KShortcut(),this,SLOT(slotCheckUpdates()),m_filesAction,"make_check_updates");

    /* 2. actions only on files */
    m_BlameAction = new KAction("&Blame","flag",KShortcut(),this,SLOT(slotBlame()),m_filesAction,"make_svn_blame");
    m_BlameAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_BlameRangeAction = new KAction("Blame range","flag",KShortcut(),this,SLOT(slotRangeBlame()),m_filesAction,"make_svn_range_blame");
    m_BlameRangeAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_CatAction = new KAction(i18n("Cat head"),
        "contents",KShortcut(),this,SLOT(slotCat()),m_filesAction,"make_svn_cat");
    m_CatAction->setToolTip(i18n("Output the content of specified files or URLs."));
    tmp_action = new KAction(i18n("Cat revision..."),"contents",KShortcut(),this,SLOT(slotRevisionCat()),m_filesAction,"make_revisions_cat");
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs at specific revision."));

    m_LockAction = new KAction(i18n("Lock current items"),"lock",KShortcut(),this,SLOT(slotLock()),m_filesAction,"make_svn_lock");
    m_UnlockAction = new KAction(i18n("Unlock current items"),"unlock",KShortcut(),this,SLOT(slotUnlock()),m_filesAction,"make_svn_unlock");

    /* 3. actions only on dirs */
    m_MkdirAction = new KAction("New folder","folder_new",
        KShortcut(),this,SLOT(slotMkdir()),m_filesAction,"make_svn_mkdir");
    m_switchRepository = new KAction("Switch repository","goto",KShortcut(),
        m_SvnWrapper,SLOT(slotSwitch()),m_filesAction,"make_svn_switch");
    m_switchRepository->setToolTip(i18n("Switch repository of working copy (\"svn switch\")"));

    m_changeToRepository = new KAction(i18n("Open repository of working copy"),"gohome",KShortcut(),
        this,SLOT(slotChangeToRepository()),m_filesAction,"make_switch_to_repo");
    m_changeToRepository->setToolTip(i18n("Opens the repository the current working copy was checked out from"));

    m_CleanupAction = new KAction("Cleanup",KShortcut(),
        this,SLOT(slotCleanupAction()),m_filesAction,"make_cleanup");
    m_ImportDirsIntoCurrent  = new KAction(i18n("Import folders into current"),"fileimport",KShortcut(),
        this,SLOT(slotImportDirsIntoCurrent()),m_filesAction,"make_import_dirs_into_current");
    m_ImportDirsIntoCurrent->setToolTip(i18n("Import folder content into current url"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent = new KAction("Add selected files/dirs","vcs_add",KShortcut(Key_Insert),m_SvnWrapper,SLOT(slotAdd()),m_filesAction,"make_svn_add");
    m_AddCurrent->setToolTip(i18n("Adding selected files and/or directories to repository"));
    m_DelCurrent = new KAction("Delete selected files/dirs","svndelete",
        KShortcut(Key_Delete),this,SLOT(slotDelete()),m_filesAction,"make_svn_remove");
    m_DelCurrent->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    m_RevertAction  = new KAction("Revert current changes","revert",
        KShortcut(),m_SvnWrapper,SLOT(slotRevert()),m_filesAction,"make_svn_revert");
    m_ResolvedAction = new KAction("Resolve recursive",KShortcut(),
        this,SLOT(slotResolved()),m_filesAction,"make_resolved");
    m_ResolvedAction->setToolTip(i18n("Marking files or dirs resolved"));
    m_IgnoreAction = new KAction(i18n("Ignore/Unignore current item"),KShortcut(),this,SLOT(slotIgnore()),m_filesAction,"make_svn_ignore");

    m_UpdateHead = new KAction("Update to head","vcs_update",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateHeadRec()),m_filesAction,"make_svn_headupdate");
    m_UpdateRev = new KAction("Update to revision...","vcs_update",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateTo()),m_filesAction,"make_svn_revupdate");
    m_commitAction = new KAction("Commit","vcs_commit",
        KShortcut("#"),m_SvnWrapper,SLOT(slotCommit()),m_filesAction,"make_svn_commit");
    m_simpleDiffHead = new KAction(i18n("Diff against head"),"vcs_diff",
        KShortcut(CTRL+Key_H),this,SLOT(slotSimpleDiff()),m_filesAction,"make_svn_headdiff");
    m_MergeRevisionAction = new KAction(i18n("Merge two revisions"),"merge",
        KShortcut(),this,SLOT(slotMergeRevisions()),m_filesAction,"make_svn_merge_revisions");
    m_MergeRevisionAction->setToolTip(i18n("Merge two revisions of these entry into itself"));

    /* remote actions only */
    m_CheckoutCurrentAction = new KAction("Checkout current repository path",KShortcut(),
        m_SvnWrapper,SLOT(slotCheckoutCurrent()),m_filesAction,"make_svn_checkout_current");
    m_ExportCurrentAction = new KAction(i18n("Export current repository path"),"fileexport",KShortcut(),
        m_SvnWrapper,SLOT(slotExportCurrent()),m_filesAction,"make_svn_export_current");
    new KAction(i18n("Select browse revision"),KShortcut(),this,SLOT(slotSelectBrowsingRevision()),m_filesAction,"switch_browse_revision");

    /* independe actions */
    m_CheckoutAction = new KAction(i18n("Checkout a repository"),"bottom",
        KShortcut(),m_SvnWrapper,SLOT(slotCheckout()),m_filesAction,"make_svn_checkout");
    m_ExportAction = new KAction(i18n("Export a repository"),"fileexport",
        KShortcut(),m_SvnWrapper,SLOT(slotExport()),m_filesAction,"make_svn_export");
    m_RefreshViewAction = new KAction(i18n("Refresh view"),"reload",KShortcut(CTRL+Key_U),this,SLOT(refreshCurrentTree()),m_filesAction,"make_view_refresh");

    new KAction(i18n("Diff revisions"),"vcs_diff",KShortcut(),this,SLOT(slotDiffRevisions()),m_filesAction,"make_revisions_diff");

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
    if (m_isLocal&&firstChild()) {
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
    clear();
    m_Dirsread.clear();
    if (m_SelectedItems) {
        m_SelectedItems->clear();
    }
    m_LastException="";

/*
    SshAgent ssh;
    ssh.addSshIdentities();
*/
    if (!noReinit) m_SvnWrapper->reInitClient();
    m_baseUri = url.url();
    m_isLocal = false;
    m_pList->m_remoteRevision=svn::Revision::HEAD;

    QString query = url.query();
    if (url.isLocalFile()) {
        m_baseUri = url.path();
        if (m_SvnWrapper->isLocalWorkingCopy(url)) {
            m_isLocal = true;
        } else {
            // yes! KURL sometimes makes a correct localfile url (file:///)
            // to a simple file:/ - that brakes subverision lib. so we make sure
            // that we have a correct url
            m_baseUri="file://"+m_baseUri;
        }
    }
    if (query.length()>1) {
        QMap<QString,QString> q = url.queryItems();
        if (q.find("rev")!=q.end()) {
            QString v = q["rev"];
            m_pList->m_remoteRevision=v.toInt();
        }
    }
    /* otherwise subversion lib asserts! */
    while (m_baseUri.endsWith("/")) {
        m_baseUri.truncate(m_baseUri.length()-1);
    }
    if (url.protocol()=="svn+ssh") {
        SshAgent ssh;
        ssh.addSshIdentities();
    }

    if (m_isLocal) {
        m_SvnWrapper->createModifiedCache(m_baseUri);
    }
    bool result = checkDirs(m_baseUri,0);
    if (result && m_isLocal) {
        if (firstChild()) firstChild()->setOpen(true);
    }
    if (!result) {
        m_baseUri="";
        m_isLocal=false;
        clear();
    }
    enableActions();
    emit changeCaption(m_baseUri);
    emit sigUrlOpend(result);
    return result;
}

void kdesvnfilelist::closeMe()
{
    clear();
    m_baseUri="";
    m_isLocal = false;
    m_SvnWrapper->reInitClient();
    emit changeCaption("");
    emit sigUrlOpend(false);
    enableActions();
}

bool kdesvnfilelist::checkDirs(const QString&_what,FileListViewItem * _parent)
{
    QString what = _what;
    svn::StatusEntries dlist;
    while (what.endsWith("/")) {
        what.truncate(what.length()-1);
    }

    if (!m_SvnWrapper->makeStatus(what,dlist,m_pList->m_remoteRevision)) {
        return false;
    }

    viewport()->setUpdatesEnabled(false);
    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem * pitem = 0;
    bool main_found = false;
    for (;it!=dlist.end();++it) {
        if ((*it).path()==what||QString::compare((*it).entry().url(),what)==0){
            if (!_parent) {
                pitem = new FileListViewItem(this,*it);
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
        } else {
            item = new FileListViewItem(this,_parent,*it);
        }
        if (item->isDir()) {
            m_Dirsread[item->fullName()]=false;
            item->setDropEnabled(true);
        }
    }
}

/* newdir is the NEW directory! just required if local */
void kdesvnfilelist::slotDirAdded(const QString&newdir,FileListViewItem*k)
{
    if (k) {
        k->refreshStatus();
    }
    if (!m_isLocal) {
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
        checkDirs(m_baseUri,0);
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
        if (pitem->fullName()!=m_baseUri) {
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
    if (k->isDir()&&(m_Dirsread.find(k->fullName())==m_Dirsread.end()||m_Dirsread[k->fullName()]==false)) {
        if (checkDirs(k->fullName(),k)) {
            m_Dirsread[k->fullName()]=true;
        }
    }
}

void kdesvnfilelist::slotReinitItem(SvnItem*item)
{
    if (!item) return;
    FileListViewItem*k = item->fItem();
    refreshItem(k);
    if (k->isDir()) {
        k->removeChilds();
        m_Dirsread[k->fullName()]=false;;
    }
    slotItemClicked(k);
}

void kdesvnfilelist::enableActions()
{
    bool isopen = m_baseUri.length()>0;
    bool single = allSelected()->count()==1&&isopen;
    bool multi = allSelected()->count()>1&&isopen;
    bool none = allSelected()->count()==0&&isopen;
    bool dir = false;
    if (single && allSelected()->at(0)->isDir()) {
        dir = true;
    }
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    m_LogRangeAction->setEnabled(single||(isLocal()&&!single&&!multi&&isopen));
    m_LogFullAction->setEnabled(single||(isLocal()&&!single&&!multi&&isopen));
    m_propertyAction->setEnabled(single);
    m_DelCurrent->setEnabled( (multi||single));
    m_LockAction->setEnabled( (multi||single));
    m_UnlockAction->setEnabled( (multi||single));
    m_IgnoreAction->setEnabled((single)&&singleSelected()->parent()!=0&&!singleSelected()->isRealVersioned());

    m_RenameAction->setEnabled(single && (!m_isLocal||singleSelected()!=firstChild()));
    m_CopyAction->setEnabled(single && (!m_isLocal||singleSelected()!=firstChild()));

    /* 2. only on files */
    m_BlameAction->setEnabled(single&&!dir);
    m_BlameRangeAction->setEnabled(single&&!dir);
    m_CatAction->setEnabled(single&&!dir);
    /* 3. actions only on dirs */
    m_MkdirAction->setEnabled(dir||!m_isLocal&&isopen);
    m_switchRepository->setEnabled(dir && isLocal());
    m_changeToRepository->setEnabled(isLocal());
    m_ImportDirsIntoCurrent->setEnabled(dir);
    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent->setEnabled( (multi||single) && isLocal());
    m_RevertAction->setEnabled( (multi||single) && isLocal());
    m_ResolvedAction->setEnabled( (multi||single) && isLocal());
    m_InfoAction->setEnabled( (single||multi));
    m_MergeRevisionAction->setEnabled(single&&m_isLocal);

    m_UpdateHead->setEnabled(isLocal()&&isopen);
    m_UpdateRev->setEnabled(isLocal()&&isopen);
    m_commitAction->setEnabled(isLocal()&&isopen);
    m_simpleDiffHead->setEnabled(isLocal()&&isopen);

    /* 2. on dirs only */
    m_CleanupAction->setEnabled(isLocal()&&dir);

    /* remote actions only */
    m_CheckoutCurrentAction->setEnabled( ((single&&dir)||none) && !isLocal());
    m_ExportCurrentAction->setEnabled( ((single&&dir)||none) && !isLocal());
    /* independ actions */
    m_CheckoutAction->setEnabled(true);
    m_ExportAction->setEnabled(true);
    m_RefreshViewAction->setEnabled(isopen);

    KAction * temp;
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
        temp->setEnabled(!isLocal()&&isopen);
    }
    temp = filesActions()->action("make_check_updates");
    if (temp) {
        temp->setEnabled(isLocal()&&isopen);
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
    if (!isLocal()) {
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
    if (isLocal()) {
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
    if (!isLocal()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which||!which->isDir()) return;
    m_SvnWrapper->slotCleanup(which->fullName());
    which->refreshStatus(true);
}

void kdesvnfilelist::slotResolved()
{
    if (!isLocal()) return;
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
    dlg->resize(dlg->configDialogSize(*(kdesvnPart::config()),name?name:"standard_size"));
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
    dlg->saveDialogSize(*(kdesvnPart::config()),"import_log_msg",false);

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

    if (!isLocal()) {
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
    if (m_isLocal) {
        m_SvnWrapper->createModifiedCache(m_baseUri);
    }
    if (item->fullName()==baseUri()) {
        if (!refreshItem(item)) {
            delete item;
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

void kdesvnfilelist::refreshRecursive(FileListViewItem*_parent)
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
        if (QString::compare((*it).path(),what)==0) {
            continue;
        }
        FileListViewItemListIterator clistIter(currentSync);
        gotit = false;
        while ( (k=clistIter.current()) ) {
            ++clistIter;
            if ( QString::compare(k->fullName(),(*it).path())==0) {
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
    bool isopen = m_baseUri.length()>0;
    if (item) {
        if (isLocal()) {
            emit sigShowPopup("local_context");
        } else {
            emit sigShowPopup("remote_context");
        }
    } else {
        if (!isopen) {
            emit sigShowPopup("general_empty");
        } else {
            if (isLocal()) {
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
    kdDebug()<<"Accept enter: " << ok << endl;
    if (ok) {
        event->accept();
    } else {
        event->ignore();
    }
}

void kdesvnfilelist::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    kdDebug()<<"Drag leave"<<endl;
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
            if (m_baseUri.length()==0) {
                ok = true;
            } else {
                QPoint vp = contentsToViewport( event->pos() );
                item = isExecuteArea( vp ) ? itemAt( vp ) : 0L;
                FileListViewItem*which=static_cast<FileListViewItem*>(item);
                if (!m_isLocal){
                    ok = (!item || (which->isDir()))&&urlList[0].isLocalFile()&&count==1;
                } else {
                    ok = (which && (which->isDir()))&&urlList[0].isLocalFile();
                }
            }
        }
    }
    return ok;
}

void kdesvnfilelist::contentsDropEvent(QDropEvent * event)
{
    kdDebug(0)<<"kdesvnfilelist::contentsDropEvent(QDropEvent *event)"<<endl;
    QListViewItem *item = 0;
    bool ok = validDropEvent(event,item);
    kdDebug()<<"Accept drop: " << ok << endl;
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
    if (!isLocal()) return;
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
        if (m_baseUri.length()==0) {
            openURL(urlList[0]);
            return;
        }
        if (m_baseUri.length()>0 && urlList[0].isLocalFile()) {
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
                tdir = m_baseUri;
            }
            if  (!m_isLocal) {
                slotImportIntoDir(urlList[0],tdir,fi.isDir());
            } else {
                KIO::Job * job = 0L;
                job = KIO::copy(urlList,tdir);
                connect( job, SIGNAL( result( KIO::Job * ) ),SLOT( slotCopyFinished( KIO::Job * ) ) );
                dispDummy();
                return;
            }
        }
    }
}

KdesvnFileListPrivate::KdesvnFileListPrivate()
    : dragOverItem(0),dragOverPoint(QPoint(0,0)),mOldDropHighlighter()
{
    m_remoteRevision = svn::Revision::HEAD;
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
    if (m_isLocal&&singleSelected()==firstChild()) {
        return;
    }
    bool ok, force;
    FileListViewItem*which = singleSelected();
    if (!which) return;
    QString nName =  CopyMoveView_impl::getMoveCopyTo(&ok,&force,move,
        which->fullName(),m_baseUri,this,"move_name");
    if (!ok) {
        return;
    }
    kdDebug()<<"Got out"<< endl;
    m_SvnWrapper->slotCopyMove(move,which->fullName(),nName,force);
}

void kdesvnfilelist::slotCat()
{
    FileListViewItem*k = singleSelected();
    if (!k) return;
    m_SvnWrapper->makeCat(isLocal()?svn::Revision::HEAD:m_pList->m_remoteRevision, k->fullName(),k->text(0));
}


/*!
    \fn kdesvnfilelist::slotCopyFinished( KIO::Job *)
 */
void kdesvnfilelist::slotCopyFinished( KIO::Job * job)
{
    if (job) {
        bool ok = true;
        qApp->exit_loop();
        if (job->error()) {
            job->showErrorDialog(this);
            ok = false;
        }
        kdDebug()<<"Copy was ok?"<<ok<<endl;
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
    int answer = KMessageBox::questionYesNoList(this,i18n("Really delete these entries?"),displist,"Delete from repository");
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
    dummy.setText(i18n("Please hold the line"));
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
    dlg->saveDialogSize(*(kdesvnPart::config()),"locking_log_msg",false);

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
    dlg->saveDialogSize(*(kdesvnPart::config()),"revisions_dlg",false);
    delete dlg;
}


/*!
    \fn kdesvnfilelist::slotSimpleDiff()
 */
void kdesvnfilelist::slotSimpleDiff()
{
    SvnItem*k = singleSelected();
    QString what;
    if (!k) {
        what=baseUri();
    }else{
        what=k->fullName();
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
    dlg->saveDialogSize(*(kdesvnPart::config()),"revisions_dlg",false);
    delete dlg;

}

void kdesvnfilelist::slotSelectBrowsingRevision()
{
    if (isLocal()) return;
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
            checkDirs(m_baseUri,0);
        } else {
            refreshCurrentTree();
        }
    }
    dlg->saveDialogSize(*(kdesvnPart::config()),"revisions_dlg",false);
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
    dlg->saveDialogSize(*(kdesvnPart::config()),"revisions_dlg",false);
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
        item->setStat(svnclient()->singleStatus(item->fullName(),m_pList->m_remoteRevision));
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
    reinitItems(0);
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
