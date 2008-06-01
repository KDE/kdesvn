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

#include "kdesvnfilelist.h"
#include "kdesvn_part.h"
#include "filelistviewitem.h"
#include "importdir_logmsg.h"
#include "copymoveview_impl.h"
#include "mergedlg_impl.h"
#include "svnactions.h"
#include "svnfiletip.h"
#include "keystatus.h"
#include "opencontextmenu.h"
#include "checkoutinfo_impl.h"
#include "stopdlg.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/revision.hpp"
#include "src/svnqt/dirent.hpp"
#include "src/svnqt/client.hpp"
#include "src/svnqt/status.hpp"
#include "src/svnqt/url.hpp"
#include "helpers/sshagent.h"
#include "helpers/sub2qt.h"
#include "fronthelpers/cursorstack.h"
#include "fronthelpers/widgetblockstack.h"

#include <kapplication.h>
#include <kiconloader.h>
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
#include <kurldrag.h>
#include <ktrader.h>

#include <qvbox.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qheader.h>
#include <qcheckbox.h>

#include <unistd.h>

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
    bool mdisp_unknown_files;
    bool mdisp_overlay;
    /* returns true if the display must refreshed */
    bool reReadSettings();

    bool intern_dropRunning;
    KURL::List intern_drops;
    QString intern_drop_target,merge_Src1, merge_Src2,merge_Target;
    QDropEvent::Action intern_drop_action;
    QPoint intern_drop_pos;
    QTimer drop_timer;
    QTimer dirwatch_timer;
    QTimer propTimer;

    bool mousePressed;
    QPoint presspos;

    QMap<QString,QChar> dirItems;

    void stopDirTimer()
    {
        dirwatch_timer.stop();
    }

    void startDirTimer()
    {
        dirwatch_timer.start(250,true);
    }

    void connectDirTimer(QObject*ob)
    {
        QObject::connect(&dirwatch_timer,SIGNAL(timeout()),ob,SLOT(_dirwatchTimeout()));
    }
    void stopScan()
    {
        if (m_DirWatch) {
            m_DirWatch->stopScan();
        }
    }
    void startScan()
    {
        if (m_DirWatch) {
            m_DirWatch->startScan();
        }
    }
    void startProptimer()
    {
        propTimer.start(100,true);
    }
    void stopProptimer()
    {
        propTimer.stop();
    }
    void connectPropTimer(QObject*ob)
    {
        QObject::connect(&propTimer,SIGNAL(timeout()),ob,SLOT(_propListTimeout()));
    }

private:
    void readSettings();
};

KdesvnFileListPrivate::KdesvnFileListPrivate()
    : dragOverItem(0),dragOverPoint(QPoint(0,0)),mOldDropHighlighter()
{
    m_remoteRevision = svn::Revision::HEAD;
    m_DirWatch = 0;
    intern_dropRunning=false;
    mousePressed = false;
    readSettings();
}

void KdesvnFileListPrivate::readSettings()
{
    mlist_icon_size = Kdesvnsettings::listview_icon_size();
    mdisp_ignored_files = Kdesvnsettings::display_ignored_files();
    mdisp_unknown_files = Kdesvnsettings::display_unknown_files();
    mdisp_overlay = Kdesvnsettings::display_overlays();
}

bool KdesvnFileListPrivate::reReadSettings()
{
    int _size = mlist_icon_size;
    bool _ignored = mdisp_ignored_files;
    bool _overlay = mdisp_overlay;
    bool _unknown = mdisp_unknown_files;
    readSettings();
    return (_size != mlist_icon_size||
            _ignored!=mdisp_ignored_files||
            _overlay!=mdisp_overlay||
            _unknown != mdisp_unknown_files);
}

kdesvnfilelist::kdesvnfilelist(KActionCollection*aCollect,QWidget *parent, const char *name)
 : KListView(parent, name),ItemDisplay(),m_SvnWrapper(new SvnActions(this))
{
    m_SelectedItems = 0;
    m_pList = new KdesvnFileListPrivate;
    m_filesAction = aCollect;
    m_pList->m_fileTip=new SvnFileTip(this);
    m_pList->m_fileTip->setOptions(Kdesvnsettings::display_file_tips()&&
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

    connect(this,SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)),this,
        SLOT(slotContextMenuRequested(QListViewItem *, const QPoint &, int)));

    /* not via executed 'cause click may used for selection - single click execution
       just confuses in an application */
    connect(this,SIGNAL(doubleClicked(QListViewItem*)),this,SLOT(slotItemDoubleClicked(QListViewItem*)));
    connect(this,SIGNAL(returnPressed(QListViewItem*)),this,SLOT(slotItemDoubleClicked(QListViewItem*)));

    connect(this,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_SvnWrapper,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(m_SvnWrapper,SIGNAL(reinitItem(SvnItem*)),this,SLOT(slotReinitItem(SvnItem*)));
    connect(m_SvnWrapper,SIGNAL(sigRefreshAll()),this,SLOT(refreshCurrentTree()));
    connect(m_SvnWrapper,SIGNAL(sigRefreshCurrent(SvnItem*)),this,SLOT(refreshCurrent(SvnItem*)));
    connect(m_SvnWrapper,SIGNAL(sigRefreshIcons(bool)),this,SLOT(slotRescanIcons(bool)));
    connect(this,SIGNAL(dropped (QDropEvent*,QListViewItem*)),
            this,SLOT(slotDropped(QDropEvent*,QListViewItem*)));
    connect(m_SvnWrapper,SIGNAL(sigGotourl(const QString&)),this,SLOT(_openURL(const QString&)));
    m_pList->connectDirTimer(this);
    m_pList->connectPropTimer(this);

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
    //new KAction(i18n("Log..."),"kdesvnlog",KShortcut(SHIFT+CTRL+Key_L),this,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    new KAction(i18n("Full Log"),"kdesvnlog",KShortcut(CTRL+Key_L),this,SLOT(slotMakeLog()),m_filesAction,"make_svn_log_full");
    new KAction(i18n("Full revision tree"),"kdesvnlog",KShortcut(CTRL+Key_T),this,SLOT(slotMakeTree()),m_filesAction,"make_svn_tree");
    new KAction(i18n("Partial revision tree"),"kdesvnlog",KShortcut(SHIFT+CTRL+Key_T),
        this,SLOT(slotMakePartTree()),m_filesAction,"make_svn_partialtree");

    new KAction(i18n("Properties"),"edit",
        KShortcut(CTRL+Key_P),m_SvnWrapper,SLOT(slotProperties()),m_filesAction,"make_svn_property");
    new KAction(i18n("Display Properties"),"edit",
        KShortcut(Key_P),this,SLOT(slotDisplayProperties()),m_filesAction,"get_svn_property");

    tmp_action = new KAction(i18n("Display last changes"),"kdesvndiff",
                KShortcut(),this,SLOT(slotDisplayLastDiff()),m_filesAction,"make_last_change");
    tmp_action->setToolTip(i18n("Display last changes as difference to previous commit."));

    m_InfoAction = new KAction(i18n("Details"),"kdesvninfo",
        KShortcut(Key_I),this,SLOT(slotInfo()),m_filesAction,"make_svn_info");
    m_RenameAction = new KAction(i18n("Move"),"move",
        KShortcut(Key_F2),this,SLOT(slotRename()),m_filesAction,"make_svn_rename");
    m_CopyAction = new KAction(i18n("Copy"),"kdesvncopy",
        KShortcut(Key_C),this,SLOT(slotCopy()),m_filesAction,"make_svn_copy");
    tmp_action = new KAction(i18n("Check for updates"),"kdesvncheckupdates",KShortcut(),this,SLOT(slotCheckUpdates()),m_filesAction,"make_check_updates");
    tmp_action->setToolTip(i18n("Check if current working copy has items with newer version in repository"));

    /* 2. actions only on files */
    m_BlameAction = new KAction(i18n("Blame"),"kdesvnblame",
        KShortcut(),this,SLOT(slotBlame()),m_filesAction,"make_svn_blame");
    m_BlameAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_BlameRangeAction = new KAction(i18n("Blame range"),"kdesvnblame",
        KShortcut(),this,SLOT(slotRangeBlame()),m_filesAction,"make_svn_range_blame");
    m_BlameRangeAction->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    m_CatAction = new KAction(i18n("Cat head"), "kdesvncat",
        KShortcut(),this,SLOT(slotCat()),m_filesAction,"make_svn_cat");
    m_CatAction->setToolTip(i18n("Output the content of specified files or URLs."));
    tmp_action = new KAction(i18n("Cat revision..."),"kdesvncat",
        KShortcut(),this,SLOT(slotRevisionCat()),m_filesAction,"make_revisions_cat");
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs at specific revision."));

    m_LockAction = new KAction(i18n("Lock current items"),"kdesvnlock",
        KShortcut(),this,SLOT(slotLock()),m_filesAction,"make_svn_lock");
    m_UnlockAction = new KAction(i18n("Unlock current items"),"kdesvnunlock",
        KShortcut(),this,SLOT(slotUnlock()),m_filesAction,"make_svn_unlock");

    /* 3. actions only on dirs */
    m_MkdirAction = new KAction(i18n("New folder"),"folder_new",
        KShortcut(),this,SLOT(slotMkdir()),m_filesAction,"make_svn_mkdir");
    m_switchRepository = new KAction(i18n("Switch repository"),"kdesvnswitch",
        KShortcut(), m_SvnWrapper,SLOT(slotSwitch()),m_filesAction,"make_svn_switch");
    m_switchRepository->setToolTip(i18n("Switch repository path of current working copy path (\"svn switch\")"));
    tmp_action = new KAction(i18n("Relocate current working copy url"),"kdesvnrelocate",KShortcut(),
        this,SLOT(slotRelocate()),m_filesAction,"make_svn_relocate");
    tmp_action->setToolTip(i18n("Relocate url of current working copy path to other url"));
    tmp_action = new KAction(i18n("Check for unversioned items"),"kdesvnaddrecursive",KShortcut(),
        this,SLOT(slotCheckNewItems()),m_filesAction,"make_check_unversioned");
    tmp_action->setToolTip(i18n("Browse folder for unversioned items and add them if wanted."));

    m_changeToRepository = new KAction(i18n("Open repository of working copy"),"gohome",KShortcut(),
        this,SLOT(slotChangeToRepository()),m_filesAction,"make_switch_to_repo");
    m_changeToRepository->setToolTip(i18n("Opens the repository the current working copy was checked out from"));

    m_CleanupAction = new KAction(i18n("Cleanup"),"kdesvncleanup",
	KShortcut(),this,SLOT(slotCleanupAction()),m_filesAction,"make_cleanup");
    m_CleanupAction->setToolTip(i18n("Recursively clean up the working copy, removing locks, resuming unfinished operations, etc."));
    m_ImportDirsIntoCurrent  = new KAction(i18n("Import folders into current"),"fileimport",KShortcut(),
        this,SLOT(slotImportDirsIntoCurrent()),m_filesAction,"make_import_dirs_into_current");
    m_ImportDirsIntoCurrent->setToolTip(i18n("Import folder content into current url"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent = new KAction(i18n("Add selected files/dirs"),
        "kdesvnadd",KShortcut(Key_Insert),m_SvnWrapper,SLOT(slotAdd()),m_filesAction,"make_svn_add");
    m_AddCurrent->setToolTip(i18n("Adding selected files and/or directories to repository"));
    tmp_action = new KAction("Add selected files/dirs recursive",
        "kdesvnaddrecursive",KShortcut(CTRL+Key_Insert),m_SvnWrapper,SLOT(slotAddRec()),m_filesAction,"make_svn_addrec");
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository and all subitems of folders"));

    m_DelCurrent = new KAction(i18n("Delete selected files/dirs"),"kdesvndelete",
        KShortcut(Key_Delete),this,SLOT(slotDelete()),m_filesAction,"make_svn_remove");
    m_DelCurrent->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    m_RevertAction  = new KAction(i18n("Revert current changes"),"revert",
        KShortcut(),m_SvnWrapper,SLOT(slotRevert()),m_filesAction,"make_svn_revert");

    m_ResolvedAction = new KAction(i18n("Mark resolved"),KShortcut(),
        this,SLOT(slotResolved()),m_filesAction,"make_resolved");
    m_ResolvedAction->setToolTip(i18n("Marking files or dirs resolved"));

    tmp_action = new KAction(i18n("Resolve conflicts"),KShortcut(),
                             this,SLOT(slotTryResolve()),m_filesAction,"make_try_resolve");

    m_IgnoreAction = new KAction(i18n("Ignore/Unignore current item"),KShortcut(),this,SLOT(slotIgnore()),m_filesAction,"make_svn_ignore");

    m_UpdateHead = new KAction(i18n("Update to head"),"kdesvnupdate",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateHeadRec()),m_filesAction,"make_svn_headupdate");
    m_UpdateRev = new KAction(i18n("Update to revision..."),"kdesvnupdate",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateTo()),m_filesAction,"make_svn_revupdate");
    m_commitAction = new KAction(i18n("Commit"),"kdesvncommit",
        KShortcut("#"),m_SvnWrapper,SLOT(slotCommit()),m_filesAction,"make_svn_commit");

    tmp_action = new KAction(i18n("Diff local changes"),"kdesvndiff",
        KShortcut(CTRL+Key_D),this,SLOT(slotSimpleBaseDiff()),m_filesAction,"make_svn_basediff");
    tmp_action->setToolTip(i18n("Diff working copy against BASE (last checked out version) - doesn't require access to repository"));

    tmp_action = new KAction(i18n("Diff against HEAD"),"kdesvndiff",
        KShortcut(CTRL+Key_H),this,SLOT(slotSimpleHeadDiff()),m_filesAction,"make_svn_headdiff");
    tmp_action->setToolTip(i18n("Diff working copy against HEAD (last checked in version)- requires access to repository"));

    tmp_action = new KAction(i18n("Diff items"),"kdesvndiff",
                             KShortcut(),this,SLOT(slotDiffPathes()),m_filesAction,"make_svn_itemsdiff");
    tmp_action->setToolTip(i18n("Diff two items"));


    m_MergeRevisionAction = new KAction(i18n("Merge two revisions"),"kdesvnmerge",
        KShortcut(),this,SLOT(slotMergeRevisions()),m_filesAction,"make_svn_merge_revisions");
    m_MergeRevisionAction->setToolTip(i18n("Merge two revisions of this entry into itself"));

    tmp_action=new KAction(i18n("Merge..."),"kdesvnmerge",
        KShortcut(),this,SLOT(slotMerge()),m_filesAction,"make_svn_merge");
    tmp_action->setToolTip("Merge repository path into current worky copy path or current repository path into a target");
    tmp_action=new KAction( i18n( "Open With..." ), 0, this, SLOT( slotOpenWith() ), m_filesAction, "openwith" );

    /* remote actions only */
    m_CheckoutCurrentAction = new KAction(i18n("Checkout current repository path"),"kdesvncheckout",KShortcut(),
        m_SvnWrapper,SLOT(slotCheckoutCurrent()),m_filesAction,"make_svn_checkout_current");
    m_ExportCurrentAction = new KAction(i18n("Export current repository path"),"kdesvnexport",KShortcut(),
        m_SvnWrapper,SLOT(slotExportCurrent()),m_filesAction,"make_svn_export_current");
    new KAction(i18n("Select browse revision"),KShortcut(),this,SLOT(slotSelectBrowsingRevision()),m_filesAction,"switch_browse_revision");

    /* independe actions */
    m_CheckoutAction = new KAction(i18n("Checkout a repository"),"kdesvncheckout",
        KShortcut(),m_SvnWrapper,SLOT(slotCheckout()),m_filesAction,"make_svn_checkout");
    m_ExportAction = new KAction(i18n("Export a repository"),"kdesvnexport",
        KShortcut(),m_SvnWrapper,SLOT(slotExport()),m_filesAction,"make_svn_export");
    m_RefreshViewAction = new KAction(i18n("Refresh view"),"reload",KShortcut(Key_F5),this,SLOT(refreshCurrentTree()),m_filesAction,"make_view_refresh");

    new KAction(i18n("Diff revisions"),"kdesvndiff",KShortcut(),this,SLOT(slotDiffRevisions()),m_filesAction,"make_revisions_diff");

    /* folding options */
    tmp_action = new KAction( i18n("Unfold File Tree"), 0, this , SLOT(slotUnfoldTree()), m_filesAction, "view_unfold_tree" );
    tmp_action->setToolTip(i18n("Opens all branches of the file tree"));
    tmp_action = new KAction( i18n("Fold File Tree"), 0, this, SLOT(slotFoldTree()), m_filesAction, "view_fold_tree" );
    tmp_action->setToolTip(i18n("Closes all branches of the file tree"));

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

KURL::List kdesvnfilelist::selectedUrls()
{
    KURL::List lst;
    FileListViewItemList*ls = allSelected();
    FileListViewItemListIterator it(*ls);
    FileListViewItem*cur;
    while ( (cur=it.current())!=0) {
        ++it;
        /* for putting it to outside we must convert it to KIO urls */
        lst.append(cur->kdeName(m_pList->m_remoteRevision));
    }
    return lst;
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

void kdesvnfilelist::_openURL(const QString&url)
{
    openURL(url,true);
    emit sigUrlChanged(baseUri());
}

bool kdesvnfilelist::openURL( const KURL &url,bool noReinit )
{
    CursorStack a;
    m_SvnWrapper->killallThreads();
    clear();
    kdDebug()<<"Start open URL"<<endl;
    emit sigProplist(svn::PathPropertiesMapListPtr(new svn::PathPropertiesMapList()),false,QString(""));
    m_Dirsread.clear();
    if (m_SelectedItems) {
        m_SelectedItems->clear();
    }
    m_LastException="";
    delete m_pList->m_DirWatch;
    m_pList->m_DirWatch=0;
    m_pList->dirItems.clear();
    m_pList->stopDirTimer();

    if (!noReinit) m_SvnWrapper->reInitClient();

    QString query = url.query();

    KURL _url = url;
    QString proto = svn::Url::transformProtokoll(url.protocol());
    _url.cleanPath(true);
    _url.setProtocol(proto);
    proto = _url.url(-1);

    QStringList s = QStringList::split("?",proto);
    if (s.size()>1) {
        setBaseUri(s[0]);
    } else {
        setBaseUri(proto);
    }
    setWorkingCopy(false);
    setNetworked(false);

    m_pList->m_remoteRevision=svn::Revision::HEAD;


    QString _dummy;

    if (!QString::compare("svn+file",url.protocol())) {
        setBaseUri("file://"+url.path());
    } else {
        if (url.isLocalFile()) {
            QString s = url.path();
            while(s.endsWith("/")) {
                s.remove(s.length()-1,1);
            }
            QFileInfo fi(s);
            if (fi.exists() && fi.isSymLink()) {
                QString sl = fi.readLink();
                if (sl.startsWith("/")) {
                    setBaseUri(sl);
                } else {
                    fi.setFile(fi.dirPath()+"/"+sl);
                    setBaseUri(fi.absFilePath());
                }
            } else {
                setBaseUri(url.path());
            }
            if (m_SvnWrapper->isLocalWorkingCopy(baseUri(),_dummy)) {
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

    if (url.protocol()=="svn+ssh"||
        url.protocol()=="ksvn+ssh") {
        SshAgent ssh;
        ssh.addSshIdentities();
    }
    m_SvnWrapper->clearUpdateCache();
    if (isWorkingCopy()) {
        m_pList->m_DirWatch=new KDirWatch(this);
        connect(m_pList->m_DirWatch,SIGNAL(dirty(const QString&)),this,SLOT(slotDirItemDirty(const QString&)));
        connect(m_pList->m_DirWatch,SIGNAL(created(const QString&)),this,SLOT(slotDirItemCreated(const QString&)));
        connect(m_pList->m_DirWatch,SIGNAL(deleted(const QString&)),this,SLOT(slotDirItemDeleted(const QString&)));
        /* seems that recursive does not work */
        if (m_pList->m_DirWatch) {
            m_pList->m_DirWatch->addDir(baseUri()+"/",false,false);
            m_pList->m_DirWatch->startScan(true);
        }
    }
    bool result = checkDirs(baseUri(),0);
    if (result && isWorkingCopy()) {
        chdir(baseUri().local8Bit());
        if (firstChild()) firstChild()->setOpen(true);
    }
    if (!result) {
        setBaseUri("");
        setNetworked(false);
        clear();
    }
    enableActions();
    m_pList->m_fileTip->setOptions(!isNetworked()&&Kdesvnsettings::display_file_tips()&&
        QToolTip::isGloballyEnabled(),true,6);

    if (result && isWorkingCopy()) {
        m_SvnWrapper->createModifiedCache(baseUri());
        if (Kdesvnsettings::start_updates_check_on_open()) {
             slotCheckUpdates();
        }
    }
    m_SvnWrapper->startFillCache(baseUri());
    emit changeCaption(baseUri());
    emit sigUrlOpend(result);
    QTimer::singleShot(1,this,SLOT(readSupportData()));
    kdDebug()<<"End open URL"<<endl;
    return result;
}

void kdesvnfilelist::closeMe()
{
    kdDebug()<<"Close me"<<endl;
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
    // prevent this from checking unversioned folder. FIXME: what happen when we do open url on a non-working-copy folder??
    if (!isWorkingCopy()|| (!_parent) || ((_parent) && (_parent->isVersioned()))) {
        if (!m_SvnWrapper->makeStatus(what,dlist,m_pList->m_remoteRevision) ) {
            kdDebug() << "unable makeStatus" <<endl;
            return false;
        }
    } else {
        checkUnversionedDirs(_parent);
        return true;
    }
    svn::StatusEntries neweritems;
    m_SvnWrapper->getaddedItems(what,neweritems);
    dlist+=neweritems;
    bool ownupdates = true;
    //kdDebug() << "makeStatus on " << what << " created: " << dlist.count() << "items" <<endl;

    if (isUpdatesEnabled()) {
        viewport()->setUpdatesEnabled(false);
    } else {
        ownupdates=false;
    }
    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem * pitem = 0;
    bool main_found = false;
    for (;it!=dlist.end();++it) {
        //kdDebug() << "iterate over it: " << (*it)->entry().url() << endl;

        // current item is not versioned
        if (!(*it)->isVersioned() && !filterOut((*it))) {
            // if empty, we may want to create a default svn::Status for each folder inside this _parent
            // iterate over QDir and create new filelistviewitem
            checkUnversionedDirs(_parent);
        }

        if ((*it)->path()==what||QString::compare((*it)->entry().url(),what)==0){
            if (!_parent) {
                pitem = new FileListViewItem(this,*it);
                //kdDebug()<< "CheckDirs::creating new FileListViewitem as parent " + (*it)->path() << endl;
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
    if (ownupdates) {
        kdDebug()<<"Enable update"<<endl;
        viewport()->setUpdatesEnabled(true);
        viewport()->repaint();
    }
    return true;
}

void kdesvnfilelist::insertDirs(FileListViewItem * _parent,svn::StatusEntries&dlist)
{
    svn::StatusEntries::iterator it;
#if 0
    KFileItemList oneItem;
#endif

    QTime _t;
    _t.start();
    for (it = dlist.begin();it!=dlist.end();++it) {
/*        if (_t.elapsed()>300) {
           viewport()->setUpdatesEnabled(true);
            viewport()->repaint();
            viewport()->setUpdatesEnabled(false);
            _t.restart();
        }*/
        if (filterOut((*it)))
        {
            continue;
        }
        FileListViewItem * item;
        if (!_parent) {
            item = new FileListViewItem(this,*it);
        } else {
            if ( (item = _parent->findChild( (*it)->path() ))  ) {
                delete item;
            }
            item = new FileListViewItem(this,_parent,*it);
        }
        if (item->isDir()) {
            m_Dirsread[item->fullName()]=false;
            item->setDropEnabled(true);
            if (isWorkingCopy()) {
                m_pList->m_DirWatch->addDir(item->fullName());
            }
        } else if (isWorkingCopy()) {
            m_pList->m_DirWatch->addFile(item->fullName());
#if 0
            if (item->fileItem()) {
                oneItem.append(item->fileItem());
            }
#endif
//            kdDebug()<< "Watching file: " + item->fullName() << endl;
        }
    }
#if 0
    if (oneItem.count()>0) {
        int size = Kdesvnsettings::listview_icon_size();
        KIO::PreviewJob* m_previewJob= KIO::filePreview(oneItem, size, size, size, 70, true, true, 0);
        connect( m_previewJob, SIGNAL( gotPreview( const KFileItem *, const QPixmap & ) ),
                 this, SLOT( gotPreview( const KFileItem *, const QPixmap & ) ) );
        connect( m_previewJob, SIGNAL( result( KIO::Job * ) ),
                 this, SLOT( gotPreviewResult() ) );
    }
#endif
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
            if (checkDirs(k->fullName(),k)) {
                m_Dirsread[k->fullName()]=true;
            } else {
                kdDebug()<<"Checkdirs failed"<<endl;
            }
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
    svn::StatusPtr stat;
    try {
        stat = m_SvnWrapper->svnclient()->singleStatus(newdir);
    } catch (svn::ClientException e) {
        m_LastException = e.msg();
        kdDebug()<<"Catched on singlestatus"<< endl;
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

void kdesvnfilelist::slotItemRead(QListViewItem*aItem)
{
    if (!aItem) return;
    CursorStack a(Qt::BusyCursor);
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
        } else {
            emit sigListError();
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
}

void kdesvnfilelist::enableActions()
{
    bool isopen = baseUri().length()>0;
    int c = allSelected()->count();
    bool single = c==1&&isopen;
    bool multi = c>1&&isopen;
    bool none = c==0&&isopen;
    bool dir = false;
    bool unique = uniqueTypeSelected();
    if (single && allSelected()->at(0)->isDir()) {
        dir = true;
    }
    bool conflicted = single && allSelected()->at(0)->isConflicted();
    KAction * temp = 0;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    temp = filesActions()->action("make_svn_log");
    if (temp) {
        temp->setEnabled(single||none);
    }
    temp = filesActions()->action("make_last_change");
    if (temp) {
        temp->setEnabled(isopen);
    }

    temp = filesActions()->action("make_svn_log_full");
    if (temp) {
        temp->setEnabled(single||none);
    }
    temp = filesActions()->action("make_svn_tree");
    if (temp) {
        temp->setEnabled(single||none);
    }
    temp = filesActions()->action("make_svn_partialtree");
    if (temp) {
        temp->setEnabled(single||none);
    }

    temp = filesActions()->action("make_svn_property");
    if (temp) {
        temp->setEnabled(single);
    }
    temp = filesActions()->action("get_svn_property");
    if (temp) {
        temp->setEnabled(single);
    }
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
    m_MkdirAction->setEnabled(dir||none && isopen);
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
    temp = filesActions()->action("make_try_resolve");
    if (temp) {
        temp->setEnabled(conflicted && !dir);
    }

    m_InfoAction->setEnabled(isopen);
    m_MergeRevisionAction->setEnabled(single&&isWorkingCopy());
    temp = filesActions()->action("make_svn_merge");
    if (temp) {
        temp->setEnabled(single||none);
    }
    temp = filesActions()->action("make_svn_addrec");
    if (temp) {
        temp->setEnabled( (multi||single) && isWorkingCopy());
    }
    m_UpdateHead->setEnabled(isWorkingCopy()&&isopen);
    m_UpdateRev->setEnabled(isWorkingCopy()&&isopen);
    m_commitAction->setEnabled(isWorkingCopy()&&isopen);

    temp = filesActions()->action("make_svn_basediff");
    if (temp) {
        temp->setEnabled(isWorkingCopy()&&(single||none));
    }
    temp = filesActions()->action("make_svn_headdiff");
    if (temp) {
        temp->setEnabled(isWorkingCopy()&&(single||none));
    }

    /// @todo uberprüfen ob alle selektierten items den selben typ haben.
    temp = filesActions()->action("make_svn_itemsdiff");
    if (temp) {
        temp->setEnabled(multi && c==2 && unique);
    }

    /* 2. on dirs only */
    m_CleanupAction->setEnabled(isWorkingCopy()&& (dir||none));
    temp = filesActions()->action("make_check_unversioned");
    if (temp) {
        temp->setEnabled(isWorkingCopy()&& ((dir&&single) || none));
    }

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
    temp = filesActions()->action("openwith");
    if (temp) {
        temp->setEnabled(kapp->authorizeKAction("openwith")&&single&&!dir);
    }
}

void kdesvnfilelist::slotSelectionChanged()
{
    m_pList->stopProptimer();
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
    m_pList->startProptimer();
}

/*!
    \fn kdesvnfilelist::slotClientException(const QString&)
 */
void kdesvnfilelist::slotClientException(const QString&what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(KApplication::activeModalWidget(),what,i18n("SVN Error"));
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
    if (!k) return;
    svn::InfoEntry i;
    if (!m_SvnWrapper->singleInfo(k->Url(),svn::Revision::UNDEFINED,i)) {
        return;
    }
    if (i.reposRoot().isEmpty()) {
        KMessageBox::sorry(KApplication::activeModalWidget(),i18n("Could not retrieve repository of working copy."),i18n("SVN Error"));
    } else {
        sigSwitchUrl(i.reposRoot());
    }
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
    svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision);
    QString feditor = Kdesvnsettings::external_display();
    if ( feditor.compare("default") == 0 ) {
        KURL::List lst;
        lst.append(fki->kdeName(rev));
        KTrader::OfferList li = offersList(fki,true);
        if (li.count()==0||li.first()->exec().isEmpty()) {
            li = offersList(fki);
        }
        if (li.count()>0&&!li.first()->exec().isEmpty()) {
            KService::Ptr ptr = li.first();
            KRun::run( *ptr, lst);
        } else {
            KRun::displayOpenWithDialog(lst);
        }
    } else {
        if ( KRun::runCommand(feditor + " " +  fki->kdeName(rev).prettyURL()) <= 0) {
            KMessageBox::error(this,i18n("Failed: %1 %2").arg(feditor).arg(fki->fullName()));
        }
    }
}

void kdesvnfilelist::slotCleanupAction()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which||!which->isDir()) return;
    if (m_SvnWrapper->makeCleanup(which->fullName())) {
        which->refreshStatus(true);
    }
}

void kdesvnfilelist::slotResolved()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which) return;
    m_SvnWrapper->slotResolved(which->fullName());
    which->refreshStatus(true);
    slotRescanIcons(false);
}

void kdesvnfilelist::slotTryResolve()
{
    if (!isWorkingCopy()) return;
    FileListViewItem*which= singleSelected();
    if (!which || which->isDir()) {
        return;
    }
    m_SvnWrapper->slotResolve(which->fullName());
}

template<class T> KDialogBase* kdesvnfilelist::createDialog(T**ptr,const QString&_head,bool OkCancel,const char*name,bool showHelp)
{
    int buttons = KDialogBase::Ok;
    if (OkCancel) {
        buttons = buttons|KDialogBase::Cancel;
    }
    if (showHelp) {
        buttons = buttons|KDialogBase::Help;
    }
    KDialogBase * dlg = new KDialogBase(
        KApplication::activeModalWidget(),
        name,
        true,
        _head,
        buttons);

    if (!dlg) return dlg;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    *ptr = new T(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),name?name:"standard_size"));
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
        ptr->saveHistory(true);
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"import_log_msg",false);
        delete dlg;
        return;
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"import_log_msg",false);

    QString logMessage = ptr->getMessage();
    svn::Depth rec = ptr->getDepth();
    ptr->saveHistory(false);
    uri.setProtocol("");
    QString iurl = uri.path();
    while (iurl.endsWith("/")) {
        iurl.truncate(iurl.length()-1);
    }

    if (dirs && ptr2 && ptr2->createDir()) {
        targetUri+= "/"+uri.fileName(true);
    }
    if (ptr2) {
        m_SvnWrapper->slotImport(iurl,targetUri,logMessage,rec,ptr2->noIgnore(),ptr2->ignoreUnknownNodes());
    } else {
        m_SvnWrapper->slotImport(iurl,targetUri,logMessage,rec,false,false);
    }

    if (!isWorkingCopy()) {
        if (allSelected()->count()==0) {
            refreshCurrentTree();
        } else {
            refreshCurrent(allSelected()->at(0));
        }
    }
    delete dlg;
}

void kdesvnfilelist::readSupportData()
{
    /// this moment empty cause no usagedata explicit used by kdesvnfilelist
}

void kdesvnfilelist::refreshCurrentTree()
{
    QTime t;
    t.start();
    FileListViewItem*item = static_cast<FileListViewItem*>(firstChild());
    if (!item) return;
    //m_pList->stopScan();
    m_pList->m_fileTip->setItem(0);
    kapp->processEvents();
    setUpdatesEnabled(false);
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
    if (isWorkingCopy()) {
        m_SvnWrapper->createModifiedCache(baseUri());
    }
    kdDebug()<<"Refresh time: "<<t.elapsed()<<" ms"<<endl;
    setUpdatesEnabled(true);
    viewport()->repaint();
    QTimer::singleShot(1,this,SLOT(readSupportData()));
    //m_pList->startScan();
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

bool kdesvnfilelist::refreshRecursive(FileListViewItem*_parent,bool down)
{
    FileListViewItem*item;
    if (_parent) {
        item = static_cast<FileListViewItem*>(_parent->firstChild());
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }

    if (!item) return false;
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
        kdDebug()<<"Fehler bei makestatus fuer "<<what <<endl;
        return false;
    }
    if (isWorkingCopy()) {
        svn::StatusEntries neweritems;
        m_SvnWrapper->getaddedItems(what,neweritems);
        dlist+=neweritems;
    }

    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem*k;
    bool gotit = false;
    bool dispchanged = false;
    for (;it!=dlist.end();++it) {
        gotit = false;
        if ((*it)->path()==what) {
            continue;
        }
        FileListViewItemListIterator clistIter(currentSync);
        while ( (k=clistIter.current()) ) {
            ++clistIter;
            if (k->fullName()==(*it)->path()) {
                currentSync.removeRef(k);
                k->updateStatus(*it);
                if (filterOut(k)) {
                    dispchanged=true;
                    delete k;
                }
                gotit = true;
                break;
            }
        }
        if (!gotit &&!filterOut((*it)) ) {
            dispchanged = true;
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
        return dispchanged;
    }
    while (item) {
        if (item->isDir()) {
            if ((m_Dirsread.find(item->fullName())!=m_Dirsread.end()&&m_Dirsread[item->fullName()]==true)) {
                if (item->childCount()==0) {
                    checkDirs(item->fullName(),item);
                    dispchanged = true;
                } else {
                    dispchanged = refreshRecursive(item)?true:dispchanged;
                }
            }
        }
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }
    return dispchanged;
}

KTrader::OfferList kdesvnfilelist::offersList(SvnItem*item,bool execOnly)
{
    KTrader::OfferList offers;
    if (!item) {
        return offers;
    }
    QString constraint;
    if (execOnly) {
        constraint = "Type == 'Application' or (exist Exec)";
    } else {
        constraint = "Type == 'Application'";
    }
    offers = KTrader::self()->query(item->mimeType()->name(), constraint);

    return offers;
}

void kdesvnfilelist::slotContextMenuRequested(QListViewItem */* _item */, const QPoint &, int)
{
//    FileListViewItem*item = static_cast<FileListViewItem*>(_item);
    bool isopen = baseUri().length()>0;
    SvnItemList l;
    SelectionList(&l);

    QString menuname;

    if (!isopen) {
        menuname="empty";
    } else if (isWorkingCopy()) {
        menuname="local";
    } else {
        menuname="remote";
    }
    if (l.count()==0) {
        menuname+="_general";
    } else if (l.count()>1){
        menuname+="_context_multi";
    } else {
        menuname+="_context_single";
        if (isWorkingCopy()) {
            if (l.at(0)->isRealVersioned()) {
                if (l.at(0)->isConflicted()) {
                    menuname+="_conflicted";
                } else {
                    menuname+="_versioned";
                    if (l.at(0)->isDir()) {
                        menuname+="_dir";
                    }
                }
            } else {
                menuname+="_unversioned";
            }
        } else if (l.at(0)->isDir()) {
            menuname+="_dir";
        }
    }

    QWidget * target;
    emit sigShowPopup(menuname,&target);
    QPopupMenu *popup = static_cast<QPopupMenu *>(target);
    if (!popup) {
        kdDebug()<<"Error getting popupMenu"<<endl;
        return;
    }

    KTrader::OfferList offers;
    OpenContextmenu*me=0;
    KAction*temp = 0;

    int id = -1;

    if (l.count()==1) offers = offersList(l.at(0));

    if (l.count()==1&&!l.at(0)->isDir()) {
        temp = filesActions()->action("openwith");
        if (offers.count()>0) {
            svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision);
            me= new OpenContextmenu(l.at(0)->kdeName(rev),offers,0,0);
            id = popup->insertItem(i18n("Open With..."),me);
        } else {
            temp = filesActions()->action("openwith");
            if (temp) {
                temp->plug(popup);
            }
        }
    }
    popup->exec(QCursor::pos());
    if (id>-1) {
        popup->removeItem(id);
    }
    delete me;
    if (temp) {
        temp->unplug(popup);
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

//void kdesvnfilelist::startDrag()
QDragObject* kdesvnfilelist::dragObject()
{
    m_pList->m_fileTip->setItem(0);
    QListViewItem * m_pressedItem = currentItem();
    if (!m_pressedItem) {
        return 0;
    }
    QPixmap pixmap2;
    KURL::List urls = selectedUrls();
    if (urls.count()==0) {
        return 0;
    }
    if (!viewport()->hasFocus()) {
        kdDebug()<<"Set focus"<<endl;
        viewport()->setFocus();
    }
    kdDebug() << "dragObject: " << urls << endl;
    bool pixmap0Invalid = !m_pressedItem->pixmap(0) || m_pressedItem->pixmap(0)->isNull();
    if (( urls.count() > 1 ) || (pixmap0Invalid)) {
      int iconSize = Kdesvnsettings::listview_icon_size();;
      iconSize = iconSize ? iconSize : kdesvnPartFactory::instance()->iconLoader()->currentSize( KIcon::Small ); // Default = small
      pixmap2 = DesktopIcon( "kmultiple", iconSize );
      if ( pixmap2.isNull() ) {
          kdWarning() << "Could not find multiple pixmap" << endl;
      }
    }

    KURLDrag *drag;
    drag = new KURLDrag(urls,viewport());

    /* workaround for KURL::Drag - it always forget the revision part on drop :( */
    if (!isWorkingCopy()) {
        QStrList l;
        QString t;
        KURL::List::ConstIterator it = urls.begin();
        for (;it!=urls.end();++it) {
            l.append((*it).prettyURL());
        }
        drag->setUris(l);
    }

    drag->setExportAsText(true);
    if ( !pixmap2.isNull() )
        drag->setPixmap( pixmap2 );
    else if ( !pixmap0Invalid )
        drag->setPixmap( *m_pressedItem->pixmap( 0 ) );

    return drag;
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
    if (!isWorkingCopy()) {
        if (m_pList->m_remoteRevision!=svn::Revision::HEAD) {
            item = 0;
            return false;
        }
    }
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
                if (!isWorkingCopy()) {
                    if (event->source()!=viewport()){
                        ok = (!item || (which->isDir()))&&urlList[0].isLocalFile()&&count==1;
                    } else {
                        ok = (!item || (which->isDir() ));
                    }
                } else {
                    ok = (which && (which->isDir()));
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
        dropped(event,item);
    } else {
        event->ignore();
    }
}

void kdesvnfilelist::contentsDragMoveEvent( QDragMoveEvent* event)
{
    QListViewItem * item;
    bool ok = validDropEvent(event,item);

    if (item && item!=m_pList->dragOverItem) {
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
    bool force,dry,rec,irelated,useExternal;
    Rangeinput_impl::revision_range range;
    if (!MergeDlg_impl::getMergeRange(range,&force,&rec,&irelated,&dry,&useExternal,this,"merge_range")) {
        return;
    }
    if (!useExternal) {
        m_SvnWrapper->slotMergeWcRevisions(which->fullName(),range.first,range.second,rec,irelated,force,dry);
    } else {
        m_SvnWrapper->slotMergeExternal(which->fullName(),which->fullName(),which->fullName(),range.first,range.second,rec);
    }
    refreshItem(which);
    refreshRecursive(which);
}

void kdesvnfilelist::slotMerge()
{
    FileListViewItem*which= singleSelected();
    QString src1,src2,target;
    if (isWorkingCopy()) {
        if (m_pList->merge_Target.isEmpty()) {
            target = which?which->fullName():baseUri();
        } else {
            target = m_pList->merge_Target;
        }
        src1 = m_pList->merge_Src1;
    } else {
        if (m_pList->merge_Src1.isEmpty()){
            src1 = which?which->fullName():baseUri();
        } else {
            src1 = m_pList->merge_Src1;
        }
        target = m_pList->merge_Target;
    }
    src2 = m_pList->merge_Src2;
    bool force,dry,rec,irelated,useExternal;
    Rangeinput_impl::revision_range range;
    MergeDlg_impl*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Merge")),true,"merge_dialog",true);
    if (!dlg) {
        return;
    }
    dlg->setHelp("merging-items","kdesvn");
    ptr->setDest(target);
    ptr->setSrc1(src1);
    ptr->setSrc2(src1);
    if (dlg->exec()==QDialog::Accepted) {
        src1=ptr->Src1();
        src2=ptr->Src2();
        if (src2.isEmpty()) {
            src2 = src1;
        }
        target = ptr->Dest();
        m_pList->merge_Src2 = src2;
        m_pList->merge_Src1 = src1;
        m_pList->merge_Target = target;
        force = ptr->force();
        dry = ptr->dryrun();
        rec = ptr->recursive();
        irelated = ptr->ignorerelated();
        useExternal = ptr->useExtern();
        range = ptr->getRange();
        if (!useExternal) {
            m_SvnWrapper->slotMerge(src1,src2,target,range.first,range.second,rec,irelated,force,dry);
        } else {
            m_SvnWrapper->slotMergeExternal(src1,src2,target,range.first,range.second,rec);
        }
        if (isWorkingCopy()) {
//            refreshItem(which);
//            refreshRecursive(which);
            refreshCurrentTree();
        }
    }

    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"merge_dialog",false);

    delete dlg;
}

void kdesvnfilelist::slotDropped(QDropEvent* event,QListViewItem*item)
{
    KURL::List urlList;
    QMap<QString,QString> metaData;
    QDropEvent::Action action = event->action();
    if (!event || m_pList->intern_dropRunning||!KURLDrag::decode( event, urlList, metaData)||urlList.count()<1) {
        return;
    }
    kdDebug()<<"slotDropped"<<endl;
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

    if (event->source()!=viewport()) {
        kdDebug()<<"Dropped from outside" << endl;
        if (baseUri().length()==0) {
            openURL(urlList[0]);
            event->acceptAction();
            return;
        }
        if (baseUri().length()>0 /*&& urlList[0].isLocalFile()*/) {
            QString path = urlList[0].path();
            QFileInfo fi(path);
            if  (!isWorkingCopy()) {
                slotImportIntoDir(urlList[0],tdir,fi.isDir());
            } else {
                //m_pList->stopScan();
                KIO::Job * job = 0L;
                job = KIO::copy(urlList,tdir);
                connect( job, SIGNAL( result( KIO::Job * ) ),SLOT( slotCopyFinished( KIO::Job * ) ) );
                dispDummy();
                event->acceptAction();
                return;
            }
        }
    } else {
        kdDebug()<<"Dropped from inside " << action << endl;
        int root_x, root_y, win_x, win_y;
        uint keybstate;
        QDropEvent::Action action = QDropEvent::UserAction;
        KeyState::keystate(&root_x,&root_y,&win_x,&win_y,&keybstate);
        if (keybstate&Qt::ControlButton) {
            kdDebug()<<"Control pressed" << endl;
            action = QDropEvent::Copy;
        } else if (keybstate&Qt::ShiftButton) {
            kdDebug()<<"Shift pressed" << endl;
            action = QDropEvent::Move;
        }
        /* converting urls to interal style */
        QString nProto;
        if (isWorkingCopy()) {
            nProto="";
        } else {
            nProto = svn::Url::transformProtokoll(urlList[0].protocol());
        }
        KURL::List::Iterator it = urlList.begin();
        QStringList l;
        for (;it!=urlList.end();++it) {
            l = QStringList::split("?",(*it).prettyURL());
            if (l.size()>1) {
                (*it) = l[0];
            } else if (isWorkingCopy())
            {
                (*it) = KURL::fromPathOrURL( (*it).path());
            }
            (*it).setProtocol(nProto);
            kdDebug()<<"Dropped: "<<(*it)<<endl;
        }
        event->acceptAction();
        m_pList->intern_dropRunning=true;
        m_pList->intern_drops = urlList;
        m_pList->intern_drop_target=tdir;
        m_pList->intern_drop_action=action;
        m_pList->intern_drop_pos=QCursor::pos();
        QTimer::singleShot(0,this,SLOT(slotInternalDrop()));

//        internalDrop(action,urlList,tdir);
    }
}

void kdesvnfilelist::slotInternalDrop()
{
    QDropEvent::Action action = m_pList->intern_drop_action;
    if (action==QDropEvent::UserAction) {
         QPopupMenu popup;
         popup.insertItem(SmallIconSet("goto"), i18n( "Move Here" ) + "\t" + KKey::modFlagLabel( KKey::SHIFT ), 2 );
         popup.insertItem(SmallIconSet("editcopy"), i18n( "Copy Here" ) + "\t" + KKey::modFlagLabel( KKey::CTRL ), 1 );
         popup.insertSeparator();
         popup.insertItem(SmallIconSet("cancel"), i18n( "Cancel" ) + "\t" + KKey( Qt::Key_Escape ).toString(), 5);
         int result = popup.exec(m_pList->intern_drop_pos);
         switch (result) {
            case 1 : action = QDropEvent::Copy; break;
            case 2 : action = QDropEvent::Move; break;
            default:
            {
                m_pList->intern_dropRunning=false;
                return;
            }
         }
    }
    if (action==QDropEvent::Move) {
        m_SvnWrapper->makeMove(m_pList->intern_drops,m_pList->intern_drop_target,false);
    } else {
        m_SvnWrapper->makeCopy(m_pList->intern_drops,m_pList->intern_drop_target,svn::Revision::HEAD);
    }
    m_pList->intern_dropRunning=false;
    refreshCurrentTree();
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
    if (move) {
        m_SvnWrapper->makeMove(which->fullName(),nName,force);
    } else {
        m_SvnWrapper->makeCopy(which->fullName(),nName, isWorkingCopy()?svn::Revision::HEAD:m_pList->m_remoteRevision);
    }
}

void kdesvnfilelist::slotCat()
{
    FileListViewItem*k = singleSelected();
    if (!k) return;
    m_SvnWrapper->slotMakeCat(isWorkingCopy()?svn::Revision::HEAD:m_pList->m_remoteRevision, k->fullName(),k->text(0),
        isWorkingCopy()?svn::Revision::HEAD:m_pList->m_remoteRevision,0);
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
            m_SvnWrapper->addItems(tmp,svn::DepthInfinity);
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
    //m_pList->stopScan();
    m_pList->m_fileTip->setItem(0);

    QValueList<svn::Path> items;
    QStringList displist;
    KURL::List kioList;
    while ((cur=liter.current())!=0){
        ++liter;
        if (!cur->isRealVersioned()) {
            KURL _uri; _uri.setPath(cur->fullName());
            kioList.append(_uri);
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
    //m_pList->startScan();
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
    ptr->hideDepth(true);
    QCheckBox*_stealLock = new QCheckBox("",ptr,"create_dir_checkbox");
    _stealLock->setText(i18n("Steal lock?"));
    ptr->addItemWidget(_stealLock);
    ptr->m_keepLocksButton->hide();

    if (dlg->exec()!=QDialog::Accepted) {
        ptr->saveHistory(true);
        delete dlg;
        return;
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"locking_log_msg",false);

    QString logMessage = ptr->getMessage();
    bool steal = _stealLock->isChecked();
    ptr->saveHistory(false);

    QStringList displist;
    while ((cur=liter.current())!=0){
        ++liter;
        displist.append(cur->fullName());
    }
    m_SvnWrapper->makeLock(displist,logMessage,steal);
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
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);
    delete dlg;
}


void kdesvnfilelist::slotSimpleBaseDiff()
{
    FileListViewItem*kitem = singleSelected();
    if (isWorkingCopy())
    {
        chdir(baseUri().local8Bit());
    }

    QString what;
    if (!kitem) {
        what==".";
    } else {
        what = relativePath(kitem);
    }
    // only possible on working copies - so we may say this values
    m_SvnWrapper->makeDiff(what,svn::Revision::BASE,svn::Revision::WORKING,svn::Revision::UNDEFINED,kitem?kitem->isDir():true);
}

void kdesvnfilelist::slotSimpleHeadDiff()
{
    FileListViewItem*kitem = singleSelected();
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().local8Bit());
    }

    if (!kitem) {
        what=".";
    }else{
        what = relativePath(kitem);
    }
    // only possible on working copies - so we may say this values
    m_SvnWrapper->makeDiff(what,svn::Revision::WORKING,svn::Revision::HEAD,svn::Revision::UNDEFINED,kitem?kitem->isDir():true);
}

void kdesvnfilelist::slotDisplayLastDiff()
{
    FileListViewItem*kitem = singleSelected();
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().local8Bit());
    }
    svn::Revision end = svn::Revision::PREV;
    if (!kitem) {
        if (isWorkingCopy()) {
            QListViewItem*fi = firstChild();
            kitem = static_cast<FileListViewItem*>(fi);
            if (!kitem) {
                return;
            }
            what = relativePath(kitem);
        } else {
            what=baseUri();
        }
    }else{
        what = relativePath(kitem);
    }
    svn::Revision start;
    svn::InfoEntry inf;
    if (!kitem) {
        // it has to have an item when in working copy, so we know we are in repository view.
        if (!m_SvnWrapper->singleInfo(what,m_pList->m_remoteRevision,inf)) {
            return;
        }
        start = inf.cmtRev();
    } else {
        start = kitem->cmtRev();
    }
    if (!isWorkingCopy()) {
        if (!m_SvnWrapper->singleInfo(what,start.revnum()-1,inf)) {
            return;
        }
        end = inf.cmtRev();
    }
    m_SvnWrapper->makeDiff(what,end,what,start,realWidget());
}

void kdesvnfilelist::slotDiffPathes()
{
    QPtrList<FileListViewItem>*lst = allSelected();

    if (lst->count()!=2 || !uniqueTypeSelected()) {
        return;
    }
    m_pList->m_fileTip->setItem(0);

    FileListViewItem*k1,*k2;
    k1 = lst->at(0);
    k2 = lst->at(1);
    QString w1,w2;
    svn::Revision r1;

    if (isWorkingCopy()) {
        chdir(baseUri().local8Bit());
        w1 = relativePath(k1);
        w2 = relativePath(k2);
        r1 = svn::Revision::WORKING;
    } else {
        w1 = k1->fullName();
        w2 = k2->fullName();
        r1 = m_pList->m_remoteRevision;
    }
    m_SvnWrapper->makeDiff(w1,r1,w2,r1);
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

void kdesvnfilelist::slotMkBaseDirs()
{
    bool isopen = baseUri().length()>0;
    if (!isopen) {
        return;
    }
    QString parentDir=baseUri();
    QStringList targets;
    targets.append(parentDir+"/trunk");
    targets.append(parentDir+"/branches");
    targets.append(parentDir+"/tags");
    QString msg = i18n("Automatic generated base layout by kdesvn");
    isopen = m_SvnWrapper->makeMkdir(targets,msg);
    if (isopen) {
        slotDirAdded(targets[0],0);
//        slotDirAdded(targets[1],0);
//        slotDirAdded(targets[2],0);
    }
}

/*!
    \fn kdesvnfilelist::slotDiffRevisions()
 */
void kdesvnfilelist::slotDiffRevisions()
{
    SvnItem*k = singleSelected();
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().local8Bit());
    }

    if (!k) {
        what=(isWorkingCopy()?".":baseUri());
    }else{
        what = relativePath(k);
    }
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        svn::Revision _peg=(isWorkingCopy()?svn::Revision::WORKING:remoteRevision());
        m_SvnWrapper->makeDiff(what,r.first,r.second,_peg,k?k->isDir():true);
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);
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
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);
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
        m_SvnWrapper->slotMakeCat(r.first, k->fullName(),k->shortName(),r.first,0);
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);
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
        item->setStat(new svn::Status());
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
    if (item->childCount()==0 && item->isOpen()) {
        m_Dirsread[item->fullName()]=false;;
        setEnabled(false);
        slotItemRead(item);
        setEnabled(true);
    } else {
        item = static_cast<FileListViewItem*>(item->firstChild());
        while(item) {
            reinitItems(item);
            item = static_cast<FileListViewItem*>(item->nextSibling());
        }
    }
}


/*!
    \fn kdesvnfilelist::slotInfo()
 */
void kdesvnfilelist::slotInfo()
{
    QPtrList<SvnItem> lst;
    SelectionList(&lst);
    svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision);
    if (!isWorkingCopy()) {
        rev = m_pList->m_remoteRevision;
    }
    if (lst.count()==0) {
        if (!isWorkingCopy()) {
            m_SvnWrapper->makeInfo(baseUri(),rev,svn::Revision::UNDEFINED,Kdesvnsettings::info_recursive());
        } else {
            lst.append(SelectedOrMain());
        }
    }
    if (lst.count()>0) {
        m_SvnWrapper->makeInfo(lst,rev,rev,Kdesvnsettings::info_recursive());
    }
}


/*!
    \fn kdesvnfilelist::slotDirItemCreated(const QString&)
 */
void kdesvnfilelist::slotDirItemCreated(const QString&what)
{
    m_pList->stopDirTimer();
    m_pList->dirItems[what]='C';
    kdDebug()<<"slotDirItemCreated "<<what<<endl;
    m_pList->startDirTimer();
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
    m_pList->stopDirTimer();
    m_pList->dirItems[what]='M';
    m_pList->startDirTimer();
}

void kdesvnfilelist::_propListTimeout()
{
    dispProperties(false);
}

void kdesvnfilelist::slotDisplayProperties()
{
    dispProperties(true);
}

void kdesvnfilelist::dispProperties(bool force)
{
    CursorStack a(Qt::BusyCursor);
    bool cache_Only = (!force && isNetworked() && !Kdesvnsettings::properties_on_remote_items());
    svn::PathPropertiesMapListPtr pm;
    SvnItem*k = singleSelected();
    if (!k || !k->isRealVersioned()) {
        emit sigProplist(svn::PathPropertiesMapListPtr(),false,QString(""));
        return;
    }
    kdDebug()<<"Cacheonly: "<<cache_Only<<endl;
    svn::Revision rev(isWorkingCopy()?svn::Revision::WORKING:m_pList->m_remoteRevision);
    pm =m_SvnWrapper->propList(k->fullName(),rev,cache_Only);
    emit sigProplist(pm,isWorkingCopy(),k->fullName());
}

void kdesvnfilelist::_dirwatchTimeout()
{
    kdDebug()<<"dirtimer"<<endl;
    QMap<QString,QChar>::Iterator it;
    m_pList->m_fileTip->setItem(0);
    viewport()->setUpdatesEnabled(false);
    bool repaintit=false;
    for (it=m_pList->dirItems.begin();it!=m_pList->dirItems.end();++it)
    {
        QString what = it.key();
        QChar c = it.data();
        FileListViewItem*item = findEntryItem(what);
        if (!item) {
            m_pList->m_DirWatch->removeDir(what);
            m_pList->m_DirWatch->removeFile(what);
            m_SvnWrapper->deleteFromModifiedCache(what);
            continue;
        }
        if (c == 'M') {
            if (!item->isNormal() && item->isRealVersioned()) {
                m_SvnWrapper->addModifiedCache(item->stat());
            } else {
                m_SvnWrapper->deleteFromModifiedCache(what);
            }
            if (item->isDir()) {
                if (item->isRealVersioned()) {
                    repaintit = refreshRecursive(item,false);
                } else {
                    QListViewItem *_s;
                    while ( (_s=item->firstChild()))
                    {
                        delete _s;
                    }
                    checkUnversionedDirs(item);
                }
            }
        } else if (c=='D') {
            if (item->isDir()) {
                m_pList->m_DirWatch->removeDir(what);
            } else {
                m_pList->m_DirWatch->removeFile(what);
            }
            if (item->isDeleted()) {
                m_SvnWrapper->addModifiedCache(item->stat());
            } else if (!item->isMissing()) {
                QFileInfo fi(what);
                if (!fi.exists()) {
                    FileListViewItem*p = static_cast<FileListViewItem*>(item->parent());
                    delete item;
                    repaintit=true;
                    item = 0;
                    if (p && p->isVersioned()) {
                        p->update();
                        updateParents(p);
                    }
                }
            }
        }
#if 0
        when add dirItemDirty is send for folder above so no need for checking add-flag.
        else {
            kdDebug()<<"Entry added: "<<what << endl;
        }
#endif
        if (item) {
            refreshItem(item);
        }
    }
    m_pList->dirItems.clear();
    viewport()->setUpdatesEnabled(true);
    if (repaintit) {
//        viewport()->repaint();
    }
}

/*!
    \fn kdesvnfilelist::slotDirItemDeleted(const QString&)
 */
void kdesvnfilelist::slotDirItemDeleted(const QString&what)
{
    m_pList->stopDirTimer();
    m_pList->m_fileTip->setItem(0);
    QMap<QString,QChar>::Iterator it = m_pList->dirItems.find(what);
    if (it!=m_pList->dirItems.end() && m_pList->dirItems[what]=='A')  {
        m_pList->dirItems.erase(it);
    } else {
        m_pList->dirItems[what]='D';
    }
    m_pList->startDirTimer();
}


void kdesvnfilelist::gotPreview( const KFileItem*, const QPixmap&)
{
#if 0
    FileListViewItem*which = findEntryItem(item->localPath());
    if (which) {
        which->setPreviewPix(pixmap);
    }
//    m_previewJob = 0;
//    if (m_svnitem || item != m_svnitem->fileItem()) return;

//    m_iconLabel -> setPixmap(pixmap);
#endif
}

void kdesvnfilelist::gotPreviewResult()
{
//    m_previewJob = 0;
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
    if (!m_pList->mousePressed)
    {
        if (Kdesvnsettings::display_file_tips()) {

            QPoint vp = contentsToViewport( e->pos() );
            FileListViewItem*item = isExecuteArea( vp ) ? static_cast<FileListViewItem*>(itemAt( vp )) : 0L;

            if (item) {
                vp.setY( itemRect( item ).y() );
                QRect rect( viewportToContents( vp ), QSize(20, item->height()) );
                m_pList->m_fileTip->setItem( static_cast<SvnItem*>(item), rect, item->pixmap(0));
                m_pList->m_fileTip->setPreview(KGlobalSettings::showFilePreview(item->fullName())/*&&isWorkingCopy()*/
                        &&Kdesvnsettings::display_previews_in_file_tips());
                setShowToolTips(false);
            } else {
                m_pList->m_fileTip->setItem(0);
                setShowToolTips(true);
            }
        } else {
            m_pList->m_fileTip->setItem(0);
            setShowToolTips(true);
        }
    }
    else
    {
        if (( m_pList->presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance())
        {
            m_pList->m_fileTip->setItem(0);
            m_pList->mousePressed=false;
            //beginDrag();
        }
    }
    KListView::contentsMouseMoveEvent( e );
}

void kdesvnfilelist::contentsMousePressEvent(QMouseEvent*e)
{
    KListView::contentsMousePressEvent(e);
    m_pList->m_fileTip->setItem(0);
    QPoint p(contentsToViewport( e->pos()));
    QListViewItem *i = itemAt( p );
    // this is from qt the example - hopefully I got my problems with drag&drop fixed.
    if ( i ) {
        // if the user clicked into the root decoration of the item, don't try to start a drag!
        if ( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
                          treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
             p.x() < header()->cellPos( header()->mapToActual( 0 ) ) )
        {
            m_pList->presspos = e->pos();
            m_pList->mousePressed = true;
        }
    }
}

void kdesvnfilelist::contentsMouseReleaseEvent(QMouseEvent*e)
{
    KListView::contentsMouseReleaseEvent(e);
    m_pList->mousePressed = false;
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
    m_pList->m_fileTip->setOptions(!isNetworked()&&Kdesvnsettings::display_file_tips()&&
        QToolTip::isGloballyEnabled(),true,6);
    if (m_pList->reReadSettings()) {
        refreshCurrentTree();
    } else {
        viewport()->repaint();
    }
    sort();
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
    KDialogBase * dlg = createDialog(&ptr,i18n("Relocate path %1").arg(path),true,"relocate_dlg");
    if (dlg) {
        ptr->setStartUrl(fromUrl);
        ptr->disableAppend(true);
        ptr->disableTargetDir(true);
        ptr->disableRange(true);
        ptr->disableOpen(true);
        ptr->disableExternals(true);
        ptr->hideDepth(true,true);
        bool done = false;
        dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"relocate_dlg"));
        if (dlg->exec()==QDialog::Accepted) {
            done = m_SvnWrapper->makeRelocate(fromUrl,ptr->reposURL(),path,ptr->overwrite());
        }
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"relocate_dlg",false);
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
    if (!list) {
        return;
    }
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

            svn::StatusPtr stat(new svn::Status(fi->absFilePath()));

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

void kdesvnfilelist::rescanIconsRec(FileListViewItem*startAt,bool checkNewer,bool no_update)
{
    FileListViewItem*_s;
    if (!startAt) {
        _s = static_cast<FileListViewItem*>(firstChild());
    } else {
        _s = static_cast<FileListViewItem*>(startAt->firstChild());
    }
    if (!_s) {
        return;
    }
    svn::SharedPointer<svn::Status> d;
    while (_s) {
        //_s->makePixmap();

        if (!no_update) {
            if (m_SvnWrapper->getUpdated(_s->stat()->path(),d) && d) {
                _s->updateStatus(d);
            } else {
                _s->update();
            }
        }
        rescanIconsRec(_s,checkNewer,no_update);
        if (checkNewer && _s->isDir() && _s->isOpen()) {
            svn::StatusEntries target;
            m_SvnWrapper->getaddedItems(_s->stat()->path(),target);
            insertDirs(_s,target);
        }
        _s = static_cast<FileListViewItem*>(_s->nextSibling());
    }
}

void kdesvnfilelist::slotRescanIcons(bool checkNewer)
{
    rescanIconsRec(0L,checkNewer);
}


/*!
    \fn kdesvnfilelist::slotCheckNewItems()
 */
void kdesvnfilelist::slotCheckNewItems()
{
    if (!isWorkingCopy()) {
        KMessageBox::sorry(0,i18n("Only in working copy possible."),i18n("Error"));
        return;
    }
    if (allSelected()->count()>1) {
        KMessageBox::sorry(0,i18n("Only on single folder possible"),i18n("Error"));
        return;
    }
    SvnItem*w = SelectedOrMain();
    if (!w) {
        KMessageBox::sorry(0,i18n("Sorry - internal error!"),i18n("Error"));
        return;
    }
    m_SvnWrapper->checkAddItems(w->fullName(),true);
}

/*!
    \fn kdesvnfilelist::slotMakeRangeLog()
 */
void kdesvnfilelist::slotMakeRangeLog()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && allSelected()->count()==0){
        what = baseUri();
    } else {
        return;
    }
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    int i = dlg->exec();
    if (i==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_SvnWrapper->makeLog(r.first,r.second,(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision), what,list,0);
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);
}


void kdesvnfilelist::slotMakeTree()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && allSelected()->count()==0){
        what = baseUri();
    } else {
        return;
    }
    svn::Revision rev(isWorkingCopy()?svn::Revision::WORKING:m_pList->m_remoteRevision);

    m_SvnWrapper->makeTree(what,rev);
}

void kdesvnfilelist::slotMakePartTree()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && allSelected()->count()==0){
        what = baseUri();
    } else {
        return;
    }
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    int i = dlg->exec();
    Rangeinput_impl::revision_range r;
    if (i==QDialog::Accepted) {
        r = rdlg->getRange();
    }
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"revisions_dlg",false);

    if (i==QDialog::Accepted) {
        svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision);
        m_SvnWrapper->makeTree(what,rev,r.first,r.second);
    }
}

/*!
    \fn kdesvnfilelist::slotMakeLog()
 */
void kdesvnfilelist::slotMakeLog()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && allSelected()->count()==0){
        what = baseUri();
    } else {
        return;
    }
    // yes! so if we have a limit, the limit counts from HEAD
    // not from START
    svn::Revision start(svn::Revision::HEAD);
    if (!isWorkingCopy()) {
        start=m_pList->m_remoteRevision;
    }
    svn::Revision end(svn::Revision::START);
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    int l = Kdesvnsettings::self()->maximum_displayed_logs();
    m_SvnWrapper->makeLog(start,end,(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision),what,list,l);
}

const svn::Revision& kdesvnfilelist::remoteRevision()const
{
    return m_pList->m_remoteRevision;
}


/*!
    \fn kdesvnfilelist::slotOpenWith()
 */
void kdesvnfilelist::slotOpenWith()
{
    FileListViewItem* which = singleSelected();
    if (!which||which->isDir()) {
        return;
    }
    svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:m_pList->m_remoteRevision);
    KURL::List lst;
    lst.append(which->kdeName(rev));
    KRun::displayOpenWithDialog(lst);
}

void kdesvnfilelist::slotUnfoldTree()
{
    StopSimpleDlg sdlg(0,0,i18n("Unfold tree"),i18n("Unfold all folder"));

    connect(this,SIGNAL(sigListError()),
            &sdlg,SLOT(makeCancel()));

    QListViewItemIterator it(this);
    QTime t;t.start();

    setUpdatesEnabled(false);
    {
        WidgetBlockStack a(this);
        while (QListViewItem* item = it.current())
        {
            if (item->isExpandable()) {
                if (sdlg.isCanceld()) {
                    m_SvnWrapper->slotCancel(true);
                    break;
                }
                if (t.elapsed()>=200) {
                    sdlg.slotTick();
                    kapp->processEvents(20);
                    t.restart();
                }
                ((FileListViewItem*)item)->setOpenNoBlock(true);
            }
            ++it;
        }
    }
    setFocus();
    setUpdatesEnabled(true);
    viewport()->repaint();
    repaint();
    m_SvnWrapper->slotCancel(false);
}

void kdesvnfilelist::slotFoldTree()
{
    QListViewItemIterator it(this);
    while (QListViewItem* item = it.current())
    {
        // don't close the top level directory
        if (item->isExpandable() && item->parent())
            item->setOpen(false);

        ++it;
    }
}

/*!
    \fn kdesvnfilelist::uniqueSelected()
 */
bool kdesvnfilelist::uniqueTypeSelected()
{
    FileListViewItemList*ls = allSelected();
    FileListViewItemListIterator it(*ls);
    FileListViewItem*cur=it.current();
    if (!cur) {
        return false;
    }
    bool dir = cur->isDir();
    while ( (cur=it.current())!=0) {
        ++it;
        if (cur->isDir()!=dir) {
            return false;
        }
    }
    return true;
}

void kdesvnfilelist::slotChangeProperties(const svn::PropertiesMap&pm,const QValueList<QString>&dellist,const QString&path)
{
    m_SvnWrapper->changeProperties(pm,dellist,path);
    FileListViewItem* which = singleSelected();
    kdDebug()<<(which?which->fullName():"nix") << " -> " << path<<endl;
    if (which && which->fullName()==path) {
        which->refreshStatus();
        refreshCurrent(which);
        _propListTimeout();
    }
}

#include "kdesvnfilelist.moc"
