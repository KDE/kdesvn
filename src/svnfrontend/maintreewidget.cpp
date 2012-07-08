/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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

#include "maintreewidget.h"
#include "models/svnitemmodel.h"
#include "models/svnitemnode.h"
#include "models/svnsortfilter.h"
#include "models/svndirsortfilter.h"
#include "database/dbsettings.h"
#include "cursorstack.h"
#include "svnactions.h"
#include "copymoveview_impl.h"
#include "mergedlg_impl.h"
#include "checkoutinfo_impl.h"
#include "importdir_logmsg.h"
#include "src/settings/kdesvnsettings.h"
#include "helpers/sshagent.h"
#include "src/svnqt/url.h"
#include "src/svnqt/svnqttypes.h"
#include "fronthelpers/createdlg.h"
#include "fronthelpers/rangeinput_impl.h"
#include "fronthelpers/widgetblockstack.h"
#include "fronthelpers/fronthelpers.h"
#include "fronthelpers/DialogStack.h"
#include "src/ksvnwidgets/commitmsg_impl.h"
#include "src/ksvnwidgets/deleteform_impl.h"
#include "opencontextmenu.h"
#include "EditIgnorePattern.h"
#include "setpropertywidget.h"

#include <kmessagebox.h>
#include <kicon.h>
#include <kshortcut.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kauthorized.h>
#include <ktrader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kfiledialog.h>
#include <knotification.h>
#include <unistd.h>

#include <QSortFilterProxyModel>
#include <QEvent>
#include <QToolTip>
#include <QTimer>
#include <QHelpEvent>
#include <QMap>
#include <QCheckBox>

class MainTreeWidgetData
{
public:
    MainTreeWidgetData()
    {
        m_Collection=0;
        m_Model = 0;
        m_SortModel=0;
        m_DirSortModel = 0;
        m_remoteRevision=svn::Revision::UNDEFINED;
    }

    ~MainTreeWidgetData()
    {
        delete m_Model;
        delete m_SortModel;
        delete m_DirSortModel;
    }

    QModelIndex srcInd(const QModelIndex&ind)
    {
        return m_SortModel->mapToSource(ind);
    }

    QModelIndex srcDirInd(const QModelIndex&ind)
    {
        return m_DirSortModel->mapToSource(ind);
    }

    SvnItemModelNode*sourceNode(const QModelIndex&index,bool left)
    {
        if (!index.isValid()) {
            return 0;
        }
        QModelIndex ind = left?m_DirSortModel->mapToSource(index):m_SortModel->mapToSource(index);
        if (ind.isValid()) {
            return static_cast<SvnItemModelNode*>(ind.internalPointer());
        }
        return 0;
    }

    KActionCollection*m_Collection;
    SvnItemModel*m_Model;
    SvnSortFilterProxy*m_SortModel;
    SvnDirSortFilterProxy*m_DirSortModel;
    svn::Revision m_remoteRevision;
    QString merge_Target,merge_Src2,merge_Src1;

    QTimer m_TimeModified,m_TimeUpdates;
};

MainTreeWidget::MainTreeWidget(KActionCollection*aCollection,QWidget*parent,Qt::WindowFlags f)
    :QWidget(parent,f),m_Data(new MainTreeWidgetData)
{
    setObjectName("MainTreeWidget");
    setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    m_TreeView->setFocusPolicy(Qt::StrongFocus);
    m_Data->m_Collection = aCollection;
    m_Data->m_SortModel = new SvnSortFilterProxy();
    m_Data->m_SortModel->setDynamicSortFilter(true);
    m_Data->m_SortModel->setSortRole(SORT_ROLE);
    m_Data->m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort()?Qt::CaseSensitive:Qt::CaseInsensitive);
    m_Data->m_SortModel->sort(0);
    m_TreeView->setModel(m_Data->m_SortModel);
    m_TreeView->sortByColumn(0,Qt::AscendingOrder);
    m_Data->m_Model = new SvnItemModel(this);
    m_Data->m_SortModel->setSourceSvnModel(m_Data->m_Model);

    m_Data->m_DirSortModel = new SvnDirSortFilterProxy();
    m_Data->m_DirSortModel->setDynamicSortFilter(true);
    m_Data->m_DirSortModel->setSortRole(SORT_ROLE);
    m_Data->m_DirSortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort()?Qt::CaseSensitive:Qt::CaseInsensitive);

    m_DirTreeView->setModel(m_Data->m_DirSortModel);
    m_Data->m_DirSortModel->setSourceSvnModel(m_Data->m_Model);

    connect(m_TreeView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
        this,SLOT(slotSelectionChanged(const QItemSelection&,const QItemSelection&)));

    connect(m_DirTreeView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
        this,SLOT(slotDirSelectionChanged(const QItemSelection&,const QItemSelection&)));

    connect(m_Data->m_Model->svnWrapper(),SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_Data->m_Model,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(reinitItem(SvnItem*)),this,SLOT(slotReinitItem(SvnItem*)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigRefreshAll()),this,SLOT(refreshCurrentTree()));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigRefreshCurrent(SvnItem*)),this,SLOT(refreshCurrent(SvnItem*)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigRefreshIcons()),this,SLOT(slotRescanIcons()));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigGotourl(const QString&)),this,SLOT(_openUrl(const QString&)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigCacheStatus(qlonglong,qlonglong)),this,SIGNAL(sigCacheStatus(qlonglong,qlonglong)));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigThreadsChanged()),this,SLOT(enableActions()));
    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigCacheDataChanged()),this,SLOT(slotCacheDataChanged()));

    connect(m_Data->m_Model->svnWrapper(),SIGNAL(sigExtraStatusMessage(const QString&)),this,SIGNAL(sigExtraStatusMessage(const QString&)));

    connect(m_Data->m_Model,SIGNAL(urlDropped(const KUrl::List&,Qt::DropAction,const QModelIndex &,bool)),
            this,SLOT(slotUrlDropped(const KUrl::List&,Qt::DropAction,const QModelIndex &,bool)));

    connect(m_Data->m_Model,SIGNAL(itemsFetched(const QModelIndex&)),this,SLOT(slotItemsInserted(const QModelIndex&)));

    m_TreeView->sortByColumn(0,Qt::AscendingOrder);
    m_DirTreeView->sortByColumn(0,Qt::AscendingOrder);

    checkUseNavigation(true);
    setupActions();

    m_Data->m_TimeModified.setParent(this);
    connect(&(m_Data->m_TimeModified),SIGNAL(timeout()),this,SLOT(slotCheckModified()));
    m_Data->m_TimeUpdates.setParent(this);
    connect(&(m_Data->m_TimeUpdates),SIGNAL(timeout()),this,SLOT(slotCheckUpdates()));
}

MainTreeWidget::~MainTreeWidget()
{
    delete m_Data;
}

void MainTreeWidget::_openUrl(const QString&url)
{
    openUrl(url,true);
}

void MainTreeWidget::resizeAllColumns()
{
    m_TreeView->resizeColumnToContents(SvnItemModel::Name);
    m_TreeView->resizeColumnToContents(SvnItemModel::Status);
    m_TreeView->resizeColumnToContents(SvnItemModel::LastRevision);
    m_TreeView->resizeColumnToContents(SvnItemModel::LastAuthor);
    m_TreeView->resizeColumnToContents(SvnItemModel::LastDate);
    m_DirTreeView->resizeColumnToContents(SvnItemModel::Name);
}

bool MainTreeWidget::openUrl(const KUrl &url,bool noReinit)
{

#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif

    CursorStack a;
    m_Data->m_Model->svnWrapper()->killallThreads();
    clear();
    emit sigProplist(svn::PathPropertiesMapListPtr(new svn::PathPropertiesMapList()),false,false,QString(""));

    if (!noReinit) m_Data->m_Model->svnWrapper()->reInitClient();
    QString query = url.query();

    KUrl _url = url;
    QString proto = svn::Url::transformProtokoll(url.protocol());
    _url.cleanPath();
    _url.setProtocol(proto);
    proto = _url.url(KUrl::RemoveTrailingSlash);

    QStringList s = proto.split('?');
    if (s.size()>1) {
        setBaseUri(s[0]);
    } else {
        setBaseUri(proto);
    }
    setWorkingCopy(false);
    setNetworked(false);
    m_Data->m_remoteRevision=svn::Revision::HEAD;

    QString _dummy;

    if (!QString::compare("svn+file",url.protocol())) {
        setBaseUri("file://"+url.path());
    } else {
        if (url.isLocalFile()) {
            QString s = url.path();
            while(s.endsWith('/')) {
                s.remove(s.length()-1,1);
            }
            QFileInfo fi(s);
            if (fi.exists() && fi.isSymLink()) {
                QString sl = fi.readLink();
                if (sl.startsWith('/')) {
                    setBaseUri(sl);
                } else {
                    fi.setFile(fi.path()+'/'+sl);
                    setBaseUri(fi.absoluteFilePath());
                }
            } else {
                setBaseUri(url.path());
            }
            if (m_Data->m_Model->svnWrapper()->isLocalWorkingCopy(baseUri(),_dummy)) {
                setWorkingCopy(true);
            } else {
                // yes! KUrl sometimes makes a correct localfile url (file:///)
                // to a simple file:/ - that breakes subversion lib. so we make sure
                // that we have a correct url
                setBaseUri("file://"+baseUri());
            }
        } else {
            setNetworked(true);
            if (!Kdesvnsettings::network_on()) {
                setBaseUri("");
                setNetworked(false);
                clear();
                KMessageBox::error(this,i18n("Networked URL to open but networking is disabled!"));
                emit changeCaption("");
                emit sigUrlOpend(false);
                return false;
            }
        }
    }
    if (query.length()>1) {
        QMap<QString,QString> q = url.queryItems();
        if (q.find("rev")!=q.end()) {
            QString v = q["rev"];
            svn::Revision tmp;
            m_Data->m_Model->svnWrapper()->svnclient()->url2Revision(v,m_Data->m_remoteRevision,tmp);
            if (m_Data->m_remoteRevision==svn::Revision::UNDEFINED) {
                m_Data->m_remoteRevision = svn::Revision::HEAD;
            }
        }
    }
    if (url.protocol()=="svn+ssh"||
        url.protocol()=="ksvn+ssh") {
        SshAgent ssh;
        ssh.addSshIdentities();
    }
    m_Data->m_Model->svnWrapper()->clearUpdateCache();
    if (isWorkingCopy()) {
        m_Data->m_Model->initDirWatch();
    }
    bool result = m_Data->m_Model->checkDirs(baseUri(),0)>-1;
    if (result && isWorkingCopy()) {
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
        m_DirTreeView->expandToDepth(0);
        m_DirTreeView->selectionModel()->select(m_Data->m_DirSortModel->mapFromSource(m_Data->m_Model->firstRootIndex()),QItemSelectionModel::Select);
    }
    resizeAllColumns();

    if (!result) {
        setBaseUri("");
        setNetworked(false);
        clear();
    }
    if (result && isWorkingCopy()) {
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
        if (Kdesvnsettings::start_updates_check_on_open()) {
             slotCheckUpdates();
        }
    }
#ifdef DEBUG_TIMER
    _counttime.restart();
#endif

    if (result && Kdesvnsettings::log_cache_on_open()) {
        m_Data->m_Model->svnWrapper()->startFillCache(baseUri(),true);
    }
#ifdef DEBUG_TIMER
    kDebug()<<"Starting cache "<<_counttime.elapsed();
    _counttime.restart();
#endif
    emit changeCaption(baseUri());
    emit sigUrlOpend(result);
    emit sigUrlChanged(baseUri());
#ifdef DEBUG_TIMER
    kDebug()<<"Fired signals "<<_counttime.elapsed();
    _counttime.restart();
#endif

    QTimer::singleShot(1,this,SLOT(readSupportData()));
    enableActions();
#ifdef DEBUG_TIMER
    kDebug()<<"Enabled actions "<<_counttime.elapsed();
#endif
/*  KNotification * notification=new KNotification("kdesvn-open");
    notification->setText("Opened url");
    notification->sendEvent();
*/
    return result;
}

void MainTreeWidget::clear()
{
    m_Data->m_Model->clear();
}

const svn::Revision&MainTreeWidget::baseRevision()const
{
    return m_Data->m_remoteRevision;
}

QWidget*MainTreeWidget::realWidget()
{
    return this;
}

int MainTreeWidget::selectionCount()const
{
    int count = m_TreeView->selectionModel()->selectedRows(0).count();
    if (count == 0) {
        if (m_TreeView->rootIndex().isValid()) {
            return 1;
        }
    }
    return count;
}

int MainTreeWidget::DirselectionCount()const
{
    return m_DirTreeView->selectionModel()->selectedRows(0).count();
}

void MainTreeWidget::SelectionList(SvnItemList&target)const
{
    QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    if (_mi.count()<1) {
        QModelIndex ind = m_TreeView->rootIndex();
        if (ind.isValid()) {
            // realy! it will remapped to this before setRootIndex! (see below)
            target.push_back(m_Data->sourceNode(ind,false));
        }
        return;
    }
    for (int i = 0; i<_mi.count();++i) {
        target.push_back(m_Data->sourceNode(_mi[i],false));
    }
}

void MainTreeWidget::DirSelectionList(SvnItemList&target)const
{
    target.clear();
    QModelIndexList _mi = m_DirTreeView->selectionModel()->selectedRows(0);
    if (_mi.count()<1) {
        return;
    }
    for (int i = 0; i<_mi.count();++i) {
        target.push_back(m_Data->sourceNode(_mi[i],true));
    }
}

QModelIndex MainTreeWidget::SelectedIndex()const
{
    QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    if (_mi.count()!=1) {
        if (_mi.count()==0) {
            QModelIndex ind = m_TreeView->rootIndex();
            if (ind.isValid()) {
                ind = m_Data->m_SortModel->mapToSource(ind);
                return ind;
            }
        }
        return QModelIndex();
    }
    return m_Data->m_SortModel->mapToSource(_mi[0]);
}

QModelIndex MainTreeWidget::DirSelectedIndex()const
{
    QModelIndexList _mi = m_DirTreeView->selectionModel()->selectedRows(0);
    if (_mi.count()!=1) {
        return QModelIndex();
    }
    return m_Data->m_DirSortModel->mapToSource(_mi[0]);
}

SvnItemModelNode* MainTreeWidget::SelectedNode()const
{
    QModelIndex index = SelectedIndex();
    if (index.isValid()) {
        SvnItemModelNode*item = static_cast<SvnItemModelNode*>(index.internalPointer());
        return item;
    }
    return 0;
}

SvnItemModelNode* MainTreeWidget::DirSelectedNode()const
{
    QModelIndex index = DirSelectedIndex();
    if (index.isValid()) {
        SvnItemModelNode*item = static_cast<SvnItemModelNode*>(index.internalPointer());
        return item;
    }
    return 0;
}

void MainTreeWidget::slotSelectionChanged(const QItemSelection&,const QItemSelection&)
{
    enableActions();
    QTimer::singleShot(100,this,SLOT(_propListTimeout()));
}

SvnItem* MainTreeWidget::Selected()const
{
    return SelectedNode();
}

SvnItem* MainTreeWidget::DirSelected()const
{
    return DirSelectedNode();
}

SvnItem* MainTreeWidget::DirSelectedOrMain()const
{
    SvnItem*_item=DirSelected();
    if (_item==0 && isWorkingCopy()) {
        _item=m_Data->m_Model->firstRootChild();
    }
    return _item;
}

SvnItem* MainTreeWidget::SelectedOrMain()const
{
    SvnItem*_item=Selected();
    if (_item==0 && isWorkingCopy()) {
        _item=m_Data->m_Model->firstRootChild();
    }
    return _item;
}

void MainTreeWidget::setupActions()
{
    if (!m_Data->m_Collection) return;
    KAction*tmp_action;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    tmp_action = add_action("make_svn_log_full",i18n("History of item"),KShortcut(Qt::CTRL+Qt::Key_L),KIcon("kdesvnlog"),this,SLOT(slotMakeLog()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item"));
    tmp_action = add_action("make_svn_log_nofollow",i18n("History of item ignoring copies"),KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_L),KIcon("kdesvnlog"),this,SLOT(slotMakeLogNoFollow()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item without following copies"));

    tmp_action = add_action("make_svn_dir_log_nofollow",i18n("History of item ignoring copies"),KShortcut(),KIcon("kdesvnlog"),this,SLOT(slotDirMakeLogNoFollow()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item without following copies"));

    tmp_action = add_action("make_svn_tree",i18n("Full revision tree"),KShortcut(Qt::CTRL+Qt::Key_T),KIcon("kdesvntree"),this,SLOT(slotMakeTree()));
    tmp_action->setStatusTip(i18n("Shows history of item as linked tree"));
    tmp_action = add_action("make_svn_partialtree",i18n("Partial revision tree"),KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_T),KIcon("kdesvntree"),this,SLOT(slotMakePartTree()));
    tmp_action->setStatusTip(i18n("Shows history of item as linked tree for a revision range"));

    tmp_action = add_action("make_svn_property",i18n("Properties"),KShortcut(Qt::CTRL+Qt::Key_P),KIcon(),this,SLOT(slotRightProperties()));
    tmp_action = add_action("make_left_svn_property",i18n("Properties"),KShortcut(),KIcon(),this,SLOT(slotLeftProperties()));
    add_action("get_svn_property",i18n("Display Properties"),KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_P),KIcon(),this,SLOT(slotDisplayProperties()));
    tmp_action = add_action("make_last_change",i18n("Display last changes"),KShortcut(),KIcon("kdesvndiff"),this,SLOT(slotDisplayLastDiff()));
    tmp_action->setToolTip(i18n("Display last changes as difference to previous commit."));
    tmp_action = add_action("make_svn_info",i18n("Details"),KShortcut(Qt::CTRL+Qt::Key_I),KIcon("kdesvninfo"),this,SLOT(slotInfo()));
    tmp_action->setStatusTip(i18n("Show details about selected item"));
    tmp_action = add_action("make_svn_rename",i18n("Move"),KShortcut(Qt::Key_F2),KIcon("kdesvnmove"),this,SLOT(slotRename()));
    tmp_action->setStatusTip(i18n("Moves or renames current item"));
    tmp_action = add_action("make_svn_copy",i18n("Copy"),KShortcut(Qt::CTRL+Qt::Key_C),KIcon("kdesvncopy"),this,SLOT(slotCopy()));
    tmp_action->setStatusTip(i18n("Create a copy of current item"));
    tmp_action = add_action("make_check_updates",i18n("Check for updates"),KShortcut(),KIcon("kdesvncheckupdates"),this,SLOT(slotCheckUpdates()));
    tmp_action->setToolTip(i18n("Check if current working copy has items with newer version in repository"));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action->setIconText(i18n("Check updates"));

    /* 2. actions only on files */
    tmp_action = add_action("make_svn_blame",i18n("Blame"),KShortcut(),KIcon("kdesvnblame"),this,SLOT(slotBlame()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action("make_svn_range_blame",i18n("Blame range"),KShortcut(),KIcon("kdesvnblame"),this,SLOT(slotRangeBlame()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action("make_svn_cat",i18n("Cat head"), KShortcut(), KIcon("kdesvncat"),this,SLOT(slotCat()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs."));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action("make_revisions_cat",i18n("Cat revision..."),KShortcut(),KIcon("kdesvncat"),this,SLOT(slotRevisionCat()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs at specific revision."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action("make_svn_lock",i18n("Lock current items"),KShortcut(),KIcon("kdesvnlock"),this,SLOT(slotLock()));
    tmp_action->setToolTip(i18n("Try lock current item against changes from other users"));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action("make_svn_unlock",i18n("Unlock current items"),KShortcut(),KIcon("kdesvnunlock"),this,SLOT(slotUnlock()));
    tmp_action->setToolTip(i18n("Free existing lock on current item"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    /* 3. actions only on dirs */
    tmp_action = add_action("make_svn_mkdir",i18n("New folder"),KShortcut(),KIcon("kdesvnnewfolder"),this,SLOT(slotMkdir()));
    tmp_action->setStatusTip(i18n("Create a new folder"));
    tmp_action = add_action("make_svn_switch",i18n("Switch repository"),KShortcut(),KIcon("kdesvnswitch"), m_Data->m_Model->svnWrapper(),SLOT(slotSwitch()));
    tmp_action->setToolTip(i18n("Switch repository path of current working copy path (\"svn switch\")"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action("make_svn_relocate",i18n("Relocate current working copy url"),KShortcut(),KIcon("kdesvnrelocate"),this,SLOT(slotRelocate()));
    tmp_action->setToolTip(i18n("Relocate url of current working copy path to other url"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action("make_check_unversioned",i18n("Check for unversioned items"),KShortcut(),KIcon("kdesvnaddrecursive"),this,SLOT(slotCheckNewItems()));
    tmp_action->setIconText(i18n("Unversioned"));
    tmp_action->setToolTip(i18n("Browse folder for unversioned items and add them if wanted."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action("make_switch_to_repo",i18n("Open repository of working copy"),KShortcut(),KIcon("kdesvnrepository"),
                            this,SLOT(slotChangeToRepository()));
    tmp_action->setToolTip(i18n("Opens the repository the current working copy was checked out from"));

    tmp_action = add_action("make_cleanup",i18n("Cleanup"),KShortcut(),KIcon("kdesvncleanup"),this,SLOT(slotCleanupAction()));
    tmp_action->setToolTip(i18n("Recursively clean up the working copy, removing locks, resuming unfinished operations, etc."));
    tmp_action=add_action("make_import_dirs_into_current",i18n("Import folders into current"),KShortcut(), KIcon("kdesvnimportfolder"),
                            this,SLOT(slotImportDirsIntoCurrent()));
    tmp_action->setToolTip(i18n("Import folder content into current url"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    tmp_action = add_action("make_svn_add",i18n("Add selected files/dirs"),KShortcut(Qt::Key_Insert),KIcon("kdesvnadd"),m_Data->m_Model->svnWrapper(),SLOT(slotAdd()));
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository"));
    tmp_action->setIconText(i18n("Add"));
    tmp_action = add_action("make_svn_addrec",i18n("Add selected files/dirs recursive"),KShortcut(Qt::CTRL+Qt::Key_Insert),KIcon("kdesvnaddrecursive"),
                            m_Data->m_Model->svnWrapper(),SLOT(slotAddRec()));
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository and all subitems of folders"));

    tmp_action = add_action("make_svn_remove",i18n("Delete selected files/dirs"),KShortcut(Qt::Key_Delete),KIcon("kdesvndelete"),this,SLOT(slotDelete()));
    tmp_action->setIconText(i18n("Delete"));
    tmp_action->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    tmp_action = add_action("make_svn_remove_left",i18n("Delete folder"),KShortcut(),KIcon("kdesvndelete"),this,SLOT(slotLeftDelete()));
    tmp_action->setToolTip(i18n("Deleting selected directories from repository"));
    tmp_action->setIconText(i18n("Delete"));
    tmp_action  = add_action("make_svn_revert",i18n("Revert current changes"),KShortcut(),KIcon("kdesvnreverse"),m_Data->m_Model->svnWrapper(),SLOT(slotRevert()));

    tmp_action = add_action("make_resolved",i18n("Mark resolved"),KShortcut(),KIcon("kdesvnresolved"),this,SLOT(slotResolved()));
    tmp_action->setToolTip(i18n("Marking files or dirs resolved"));

    tmp_action = add_action("make_try_resolve",i18n("Resolve conflicts"),KShortcut(),KIcon("kdesvnresolved"),this,SLOT(slotTryResolve()));

    tmp_action = add_action("make_svn_ignore",i18n("Ignore/Unignore current item"),KShortcut(),KIcon(),this,SLOT(slotIgnore()));
    tmp_action = add_action("make_left_add_ignore_pattern",i18n("Add or Remove ignore pattern"),KShortcut(),KIcon(),this,SLOT(slotLeftRecAddIgnore()));
    tmp_action = add_action("make_right_add_ignore_pattern",i18n("Add or Remove ignore pattern"),KShortcut(),KIcon(),this,SLOT(slotRightRecAddIgnore()));

    tmp_action = add_action("make_svn_headupdate",i18n("Update to head"),KShortcut(),KIcon("kdesvnupdate"),m_Data->m_Model->svnWrapper(),SLOT(slotUpdateHeadRec()));
    tmp_action->setIconText(i18n("Update"));
    tmp_action = add_action("make_svn_revupdate",i18n("Update to revision..."),KShortcut(),KIcon("kdesvnupdate"),m_Data->m_Model->svnWrapper(),SLOT(slotUpdateTo()));
    tmp_action = add_action("make_svn_commit",i18n("Commit"),KShortcut("CTRL+#"),KIcon("kdesvncommit"),this,SLOT(slotCommit()));
    tmp_action->setIconText(i18n("Commit"));

    tmp_action = add_action("make_svn_basediff",i18n("Diff local changes"),KShortcut(Qt::CTRL+Qt::Key_D),KIcon("kdesvndiff"),this,SLOT(slotSimpleBaseDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against BASE (last checked out version) - doesn't require access to repository"));
    tmp_action = add_action("make_svn_dirbasediff",i18n("Diff local changes"),KShortcut(),KIcon("kdesvndiff"),this,SLOT(slotDirSimpleBaseDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against BASE (last checked out version) - doesn't require access to repository"));

    tmp_action =
        add_action("make_svn_headdiff",i18n("Diff against HEAD"),KShortcut(Qt::CTRL+Qt::Key_H),KIcon("kdesvndiff"),this,SLOT(slotSimpleHeadDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against HEAD (last checked in version)- requires access to repository"));

    tmp_action =
        add_action("make_svn_itemsdiff",i18n("Diff items"),KShortcut(),KIcon("kdesvndiff"),this,SLOT(slotDiffPathes()));
    tmp_action->setToolTip(i18n("Diff two items"));
    tmp_action =
        add_action("make_svn_diritemsdiff",i18n("Diff items"),KShortcut(),KIcon("kdesvndiff"),this,SLOT(slotDiffPathes()));
    tmp_action->setToolTip(i18n("Diff two items"));


    tmp_action =
        add_action("make_svn_merge_revisions",i18n("Merge two revisions"),KShortcut(),KIcon("kdesvnmerge"),this,SLOT(slotMergeRevisions()));
    tmp_action->setIconText(i18n("Merge"));
    tmp_action->setToolTip(i18n("Merge two revisions of this entry into itself"));

    tmp_action=
        add_action("make_svn_merge",i18n("Merge..."),KShortcut(),KIcon("kdesvnmerge"),this,SLOT(slotMerge()));
    tmp_action->setToolTip("Merge repository path into current worky copy path or current repository path into a target");
    tmp_action=add_action("openwith",i18n("Open With..."),KShortcut(),KIcon(),this, SLOT(slotOpenWith()));

    /* remote actions only */
    tmp_action =
        add_action("make_svn_checkout_current",i18n("Checkout current repository path"),KShortcut(),KIcon("kdesvncheckout"), m_Data->m_Model->svnWrapper(),SLOT(slotCheckoutCurrent()));
    tmp_action->setIconText(i18n("Checkout"));
    tmp_action =
        add_action("make_svn_export_current",i18n("Export current repository path"),KShortcut(),KIcon("kdesvnexport"), m_Data->m_Model->svnWrapper(),SLOT(slotExportCurrent()));
    add_action("switch_browse_revision",i18n("Select browse revision"),KShortcut(),KIcon(),this,SLOT(slotSelectBrowsingRevision()));

    /* independe actions */
    tmp_action =
        add_action("make_svn_checkout",i18n("Checkout a repository"),KShortcut(),KIcon("kdesvncheckout"),m_Data->m_Model->svnWrapper(),SLOT(slotCheckout()));
    tmp_action->setIconText(i18n("Checkout"));
    tmp_action = add_action("make_svn_export",i18n("Export a repository"),KShortcut(),KIcon("kdesvnexport"),m_Data->m_Model->svnWrapper(),SLOT(slotExport()));
    tmp_action->setIconText(i18n("Export"));
    tmp_action = add_action("make_view_refresh",i18n("Refresh view"),KShortcut(Qt::Key_F5),KIcon("kdesvnrightreload"),this,SLOT(refreshCurrentTree()));
    tmp_action->setIconText(i18n("Refresh"));

    add_action("make_revisions_diff",i18n("Diff revisions"),KShortcut(),KIcon("kdesvndiff"),this,SLOT(slotDiffRevisions()));

    /* folding options */
    tmp_action = add_action("view_unfold_tree",i18n("Unfold File Tree"),KShortcut(),KIcon(),this,SLOT(slotUnfoldTree()));
    tmp_action->setToolTip(i18n("Opens all branches of the file tree"));
    tmp_action = add_action("view_fold_tree",i18n("Fold File Tree"),KShortcut(),KIcon(),this ,SLOT(slotFoldTree()));
    tmp_action->setToolTip(i18n("Closes all branches of the file tree"));

    /* caching */
    tmp_action = add_action("update_log_cache",i18n("Update log cache"),KShortcut(),KIcon(),this,SLOT(slotUpdateLogCache()));
    tmp_action->setToolTip(i18n("Update the log cache for current repository"));

    tmp_action = add_action("make_dir_commit",i18n("Commit"),KShortcut(),KIcon("kdesvncommit"),this,SLOT(slotDirCommit()));
    tmp_action = add_action("make_dir_update",i18n("Update to head"),KShortcut(),KIcon("kdesvnupdate"),this,SLOT(slotDirUpdate()));
    tmp_action = add_action("set_rec_property_dir",i18n("Set property recursive"),KShortcut(),KIcon(),this,SLOT(slotDirRecProperty()));

    tmp_action = add_action("show_repository_settings",i18n("Settings for current repository"),KShortcut(),KIcon(),this,SLOT(slotRepositorySettings()));

    enableActions();
}

bool MainTreeWidget::uniqueTypeSelected()
{
    QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    if (_mi.count()<1) {
        return false;
    }
    bool dir = static_cast<SvnItemModelNode*>(m_Data->srcInd(_mi[0]).internalPointer())->isDir();
    for (int i = 1; i<_mi.count();++i) {
        if (static_cast<SvnItemModelNode*>(m_Data->srcInd(_mi[i]).internalPointer())->isDir()!=dir) {
            return false;
        }
    }
    return true;
}

void MainTreeWidget::enableAction(const QString&name,bool how)
{
    QAction * temp = filesActions()->action(name);
    if (temp) {
        temp->setEnabled(how);
    }
}

void MainTreeWidget::enableActions()
{
    bool isopen = baseUri().length()>0;
    int c = m_TreeView->selectionModel()->selectedRows(0).count();
    int d = m_DirTreeView->selectionModel()->selectedRows(0).count();
    SvnItemModelNode*si = SelectedNode();
    bool single = c==1&&isopen;
    bool multi = c>1&&isopen;
    bool none = c==0&&isopen;
    bool dir = false;
    bool unique = uniqueTypeSelected();
    bool remote_enabled=/*isopen&&*/m_Data->m_Model->svnWrapper()->doNetworking();

    if (single && si && si->isDir()) {
        dir = true;
    }

    bool conflicted = single && si && si->isConflicted();
    QAction * temp = 0;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    enableAction("make_svn_log_nofollow",single||none);
    enableAction("make_svn_dir_log_nofollow",d==1&&isopen);
    enableAction("make_last_change",isopen);
    enableAction("make_svn_log_full",single||none);
    enableAction("make_svn_tree",single||none);
    enableAction("make_svn_partialtree",single||none);

    enableAction("make_svn_property",single);
    enableAction("make_left_svn_property",d==1);
    enableAction("set_rec_property_dir",d==1);
    enableAction("get_svn_property",single);
    enableAction("make_svn_remove",(multi||single));
    enableAction("make_svn_remove_left",d>0);
    enableAction("make_svn_lock",(multi||single));
    enableAction("make_svn_unlock",(multi||single));

    enableAction("make_svn_ignore",(single)&&si->parent()!=0&&!si->isRealVersioned());
    enableAction("make_left_add_ignore_pattern",(d==1)&& isWorkingCopy());
    enableAction("make_right_add_ignore_pattern",dir && isWorkingCopy());

    enableAction("make_svn_rename",single && (!isWorkingCopy()||si!=m_Data->m_Model->firstRootChild()));
    enableAction("make_svn_copy",single && (!isWorkingCopy()||si!=m_Data->m_Model->firstRootChild()));

    /* 2. only on files */
    enableAction("make_svn_blame",single&&!dir&&remote_enabled);
    enableAction("make_svn_range_blame",single&&!dir&&remote_enabled);
    enableAction("make_svn_cat",single&&!dir);

    /* 3. actions only on dirs */
    enableAction("make_svn_mkdir",dir||(none && isopen));
    enableAction("make_svn_switch",isWorkingCopy()&& (single||none));
    enableAction("make_switch_to_repo",isWorkingCopy());
    enableAction("make_import_dirs_into_current",dir||d==1);
    enableAction("make_svn_relocate",isWorkingCopy()&& (single||none));

    enableAction("make_svn_export_current",((single&&dir)||none));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    enableAction("make_svn_add",(multi||single) && isWorkingCopy());
    enableAction("make_svn_revert",(multi||single) && isWorkingCopy());
    enableAction("make_resolved",(multi||single) && isWorkingCopy());
    enableAction("make_try_resolve",conflicted && !dir);

    enableAction("make_svn_info",isopen);
    enableAction("make_svn_merge_revisions",(single||d==1)&&isWorkingCopy());
    enableAction("make_svn_merge",single||d==1||none);
    enableAction("make_svn_addrec",(multi||single||d>0) && isWorkingCopy());
    enableAction("make_svn_headupdate",isWorkingCopy()&&isopen&&remote_enabled);
    enableAction("make_dir_update",isWorkingCopy()&&isopen&&remote_enabled);

    enableAction("make_svn_revupdate",isWorkingCopy()&&isopen&&remote_enabled);
    enableAction("make_svn_commit",isWorkingCopy()&&isopen&&remote_enabled);
    enableAction("make_dir_commit",isWorkingCopy()&&isopen&&remote_enabled);

    enableAction("make_svn_basediff",isWorkingCopy()&&(single||none));
    enableAction("make_svn_dirbasediff",isWorkingCopy()&&(d<2));
    enableAction("make_svn_headdiff",isWorkingCopy()&&(single||none)&&remote_enabled);

    /// @todo check if all items have same type
    enableAction("make_svn_itemsdiff",multi && c==2 && unique && remote_enabled);
    enableAction("make_svn_diritemsdiff",d==2 && isopen && remote_enabled);

    /* 2. on dirs only */
    enableAction("make_cleanup",isWorkingCopy()&& (dir||none));
    enableAction("make_check_unversioned",isWorkingCopy()&& ((dir&&single) || none));

    /* remote actions only */
    enableAction("make_svn_checkout_current",((single&&dir)||none) && !isWorkingCopy() && remote_enabled);
    /* independ actions */
    enableAction("make_svn_checkout",remote_enabled);
    enableAction("make_svn_export",true);
    enableAction("make_view_refresh",isopen);

    enableAction("make_revisions_diff",isopen);
    enableAction("make_revisions_cat",isopen && !dir && single);
    enableAction("switch_browse_revision",!isWorkingCopy()&&isopen);
    enableAction("make_check_updates",isWorkingCopy()&&isopen && remote_enabled);
    enableAction("openwith",KAuthorized::authorizeKAction("openwith")&&single&&!dir);
    enableAction("show_repository_settings",isopen);

    enableAction("repo_statistic",isopen);

    temp = filesActions()->action("update_log_cache");
    if (temp) {
        temp->setEnabled(remote_enabled);
        if (!m_Data->m_Model->svnWrapper()->threadRunning(SvnActions::fillcachethread)) {
            temp->setText(i18n("Update log cache"));
        } else {
            temp->setText(i18n("Stop updating the logcache"));
        }
    }
}

KAction*MainTreeWidget::add_action(const QString&actionname,
        const QString&text,
        const KShortcut&sequ,
        const KIcon&icon,
        QObject*target,
        const char*slot)
{
    KAction*tmp_action = 0;
    tmp_action=m_Data->m_Collection->addAction(actionname);
    tmp_action->setText(text);
    tmp_action->setShortcut(sequ);
    tmp_action->setIcon(icon);
    connect(tmp_action, SIGNAL(triggered()), target,slot);
    return tmp_action;
}

KActionCollection*MainTreeWidget::filesActions()
{
    return m_Data->m_Collection;
}

void MainTreeWidget::closeMe()
{
    m_Data->m_Model->svnWrapper()->killallThreads();

    clear();
    setWorkingCopy("");
    setNetworked(false);
    setWorkingCopy(false);
    setBaseUri("");

    emit changeCaption("");
    emit sigUrlOpend(false);
    emit sigUrlChanged("");

    enableActions();
    m_Data->m_Model->svnWrapper()->reInitClient();
}

void MainTreeWidget::refreshCurrentTree()
{
    QTime t;
    t.start();
    m_Data->m_Model->refreshCurrentTree();
    if (isWorkingCopy()) {
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
    }
    m_Data->m_SortModel->invalidate();
    setUpdatesEnabled(true);
    //viewport()->repaint();
    QTimer::singleShot(1,this,SLOT(readSupportData()));
}

void MainTreeWidget::slotSettingsChanged()
{
    m_Data->m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort()?Qt::CaseSensitive:Qt::CaseInsensitive);
    m_Data->m_SortModel->invalidate();
    enableActions();
    if (m_Data->m_Model->svnWrapper() && !m_Data->m_Model->svnWrapper()->doNetworking()) {
        m_Data->m_Model->svnWrapper()->stopFillCache();
    }
    checkUseNavigation();
}

KService::List MainTreeWidget::offersList(SvnItem*item,bool execOnly)const
{
    KService::List offers;
    if (!item) {
        return offers;
    }
    QString constraint;
    constraint = "(DesktopEntryName != 'kdesvn') and (Type == 'Application')";
    if (execOnly) {
        constraint += QString (" and (exist Exec)");
    }
    if (!item->mimeType()) {
        return offers;
    }
    offers = KMimeTypeTrader::self()->query(item->mimeType()->name(), QString::fromLatin1("Application"),constraint);
    return offers;
}

void MainTreeWidget::slotItemActivated(const QModelIndex&_index)
{
    QModelIndex index = m_Data->m_SortModel->mapToSource(_index);
    itemActivated(index);
}

void MainTreeWidget::itemActivated(const QModelIndex&index,bool keypress)
{
    Q_UNUSED(keypress);
    SvnItemModelNode*item;
    if (index.isValid() && (item=static_cast<SvnItemModelNode*>(index.internalPointer()))) {
        if (!item->isDir()) {
            svn::Revision rev;
            KUrl::List lst;
            lst.append(item->kdeName(rev));
            KService::List li = offersList(item,true);
            if (li.count()==0||li.first()->exec().isEmpty()) {
                li = offersList(item);
            }
            if (li.count()>0&&!li.first()->exec().isEmpty()) {
                KService::Ptr ptr = li.first();
                KRun::run(*ptr, lst, KApplication::activeWindow());
            } else {
                KRun::displayOpenWithDialog(lst,KApplication::activeWindow());
            }
        } else if (Kdesvnsettings::show_navigation_panel()) {
            m_DirTreeView->selectionModel()->select(m_Data->m_DirSortModel->mapFromSource(index),QItemSelectionModel::ClearAndSelect);
            QModelIndex _ind = m_Data->m_Model->parent(index);
            if (_ind.isValid()) {
                m_DirTreeView->expand(m_Data->m_DirSortModel->mapFromSource(_ind));
            }
        } else {

        }
    }
}

void MainTreeWidget::slotCheckUpdates()
{
    if (isWorkingCopy() && m_Data->m_Model->svnWrapper()->doNetworking()) {
        m_Data->m_TimeUpdates.stop();
        m_Data->m_Model->svnWrapper()->createUpdateCache(baseUri());
    }
}

void MainTreeWidget::slotCheckModified()
{
    if (isWorkingCopy()) {
        m_Data->m_TimeModified.stop();
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
    }
}

void MainTreeWidget::slotNotifyMessage(const QString&what)
{
    emit sigLogMessage(what);
    kapp->processEvents();
}

void MainTreeWidget::readSupportData()
{
    /// this moment empty cause no usagedata explicit used by MainTreeWidget
}

void MainTreeWidget::slotClientException(const QString&what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(KApplication::activeModalWidget(),what,i18n("SVN Error"));
}

void MainTreeWidget::slotCacheDataChanged()
{
    m_Data->m_SortModel->invalidate();
    if (isWorkingCopy()) {
        if (!m_Data->m_TimeModified.isActive() && Kdesvnsettings::poll_modified()) {
            m_Data->m_TimeModified.setInterval(MinutesToMsec(Kdesvnsettings::poll_modified_minutes()));
            m_Data->m_TimeModified.start();
        }
        if (!m_Data->m_TimeUpdates.isActive() && Kdesvnsettings::poll_updates()) {
            m_Data->m_TimeUpdates.setInterval(MinutesToMsec(Kdesvnsettings::poll_updates_minutes()));
            m_Data->m_TimeUpdates.start();
        }
    }
}

void MainTreeWidget::slotIgnore()
{
    m_Data->m_Model->makeIgnore(SelectedIndex());
    m_Data->m_SortModel->invalidate();
}

void MainTreeWidget::slotLeftRecAddIgnore()
{
	SvnItem* item = DirSelected();
	if (!item||!item->isDir()) {
		return;
	}
	recAddIgnore(item);
}

void MainTreeWidget::slotRightRecAddIgnore()
{
	SvnItem* item = Selected();
	if (!item||!item->isDir()) {
		return;
	}
	recAddIgnore(item);
}

void MainTreeWidget::recAddIgnore(SvnItem*item)
{
	EditIgnorePattern * ptr = 0;
	KDialog * dlg = createOkDialog(&ptr,QString(i18n("Edit pattern to ignore for \"%1\"").arg(item->shortName())),
			true,"ignore_pattern_dlg");
    KConfigGroup _k(Kdesvnsettings::self()->config(),"ignore_pattern_dlg");
    DialogStack _s(dlg,_k);
    if (dlg->exec()!=QDialog::Accepted) {
        return;
    }
    svn::Depth _d = ptr->depth();
    QStringList _pattern = ptr->items();
    bool unignore = ptr->unignore();
    svn::Revision start(svn::Revision::WORKING);
    if (!isWorkingCopy()) {
        start=baseRevision();
    }

    svn::StatusEntries res;
    if (!m_Data->m_Model->svnWrapper()->makeStatus(item->fullName(),res,start,_d,true /* all entries */,false,false)) {
    	return;
    }
    for (int i=0; i<res.count();++i) {
    	if (!res[i]->isRealVersioned() || res[i]->entry().kind()!=svn_node_dir) {
    		continue;
    	}
    	m_Data->m_Model->svnWrapper()->makeIgnoreEntry(res[i]->path(),_pattern,unignore);
    }
    refreshCurrentTree();
}

void MainTreeWidget::slotMakeLogNoFollow()const
{
    doLog(false,false);
}

void MainTreeWidget::slotMakeLog()const
{
    doLog(true,false);
}

void MainTreeWidget::slotDirMakeLogNoFollow()const
{
    doLog(false,true);
}

void MainTreeWidget::doLog(bool use_follow_settings,bool left)const
{
    SvnItem*k=left?DirSelectedOrMain():SelectedOrMain();
    QString what;
    if (k) {
        what=k->fullName();
    } else if (!isWorkingCopy() && selectionCount()==0){
        what = baseUri();
    } else {
        return;
    }
    svn::Revision start(svn::Revision::HEAD);
    if (!isWorkingCopy()) {
        start=baseRevision();
    }
    svn::Revision end(svn::Revision::START);
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    bool follow = use_follow_settings?Kdesvnsettings::log_follows_nodes():false;
    Kdesvnsettings::setLast_node_follow(follow);
    int l = 50;
    m_Data->m_Model->svnWrapper()->makeLog(start,end,(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision()),what,follow,list,l);
}


void MainTreeWidget::slotContextMenu(const QPoint&)
{
    SvnItemList l;
    SelectionList(l);
    execContextMenu(l);
}

void MainTreeWidget::slotDirContextMenu(const QPoint&vp)
{
    SvnItemList l;
    DirSelectionList(l);
    KMenu popup;
    QAction * temp = 0;
    int count = 0;
    if ( (temp=filesActions()->action("make_dir_commit")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_dir_update")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_svn_dirbasediff")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_svn_diritemsdiff")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_svn_dir_log_nofollow")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_left_svn_property")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_svn_remove_left")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("make_left_add_ignore_pattern")) && temp->isEnabled() && ++count) popup.addAction(temp);
    if ( (temp=filesActions()->action("set_rec_property_dir")) && temp->isEnabled() && ++count) popup.addAction(temp);

    KService::List offers;
    OpenContextmenu*me=0;
    QAction*menuAction = 0;

    if (l.count()==1 && l.at(0)) {
        offers = offersList(l.at(0),l.at(0)->isDir());
        if (offers.count()>0) {
            svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision());
            me= new OpenContextmenu(l.at(0)->kdeName(rev),offers,0,0);
            me->setTitle(i18n("Open With..."));
            menuAction=popup.addMenu(me);
            ++count;
        }
    }
    if (count) {
        popup.exec(m_DirTreeView->viewport()->mapToGlobal(vp));
    }
    if (menuAction) {
        popup.removeAction(menuAction);
    }
    delete me;

}

void MainTreeWidget::execContextMenu(const SvnItemList&l)
{
    bool isopen = baseUri().length()>0;
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
    QMenu* popup = static_cast<QMenu*>(target);
    if (!popup) {
        return;
    }

    KService::List offers;
    OpenContextmenu*me=0;
    QAction*temp = 0;
    QAction*menuAction = 0;

    if (l.count()==1) offers = offersList(l.at(0),l.at(0)->isDir());

    if (l.count()==1/*&&!l.at(0)->isDir()*/) {
        if (offers.count()>0) {
            svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision());
            me= new OpenContextmenu(l.at(0)->kdeName(rev),offers,0,0);
            me->setTitle(i18n("Open With..."));
            menuAction=popup->addMenu(me);
        } else {
            temp = filesActions()->action("openwith");
            if (temp) {
                popup->addAction(temp);
            }
        }
    }
    popup->exec(QCursor::pos());
    if (menuAction) {
        popup->removeAction(menuAction);
    }
    delete me;
    if (temp) {
        popup->removeAction(temp);
    }
}

void MainTreeWidget::slotUnfoldTree()
{
    m_TreeView->expandAll();
}

void MainTreeWidget::slotFoldTree()
{
    m_TreeView->collapseAll();
}

void MainTreeWidget::slotOpenWith()
{
    SvnItem* which = Selected();
    if (!which||which->isDir()) {
        return;
    }
    svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision());
    KUrl::List lst;
    lst.append(which->kdeName(rev));
    KRun::displayOpenWithDialog(lst,KApplication::activeWindow());
}

void MainTreeWidget::slotSelectBrowsingRevision()
{
    if (isWorkingCopy()) return;
    Rangeinput_impl*rdlg = 0;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    rdlg->setNoWorking(true);
    rdlg->setStartOnly(true);
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_Data->m_remoteRevision= r.first;
        clear();
        m_Data->m_Model->checkDirs(baseUri(),0);
        emit changeCaption(baseUri()+"@"+r.first.toString());
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"revisions_dlg");
    dlg->saveDialogSize(_k);
}

void MainTreeWidget::slotMakeTree()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && selectionCount()==0){
        what = baseUri();
    } else {
        return;
    }
    svn::Revision rev(isWorkingCopy()?svn::Revision::WORKING:baseRevision());

    m_Data->m_Model->svnWrapper()->makeTree(what,rev);
}

void MainTreeWidget::slotMakePartTree()
{
    QString what;
    SvnItem*k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && selectionCount()==0){
        what = baseUri();
    } else {
        return;
    }
    Rangeinput_impl*rdlg = 0;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    int i = dlg->exec();
    Rangeinput_impl::revision_range r;
    if (i==QDialog::Accepted) {
        r = rdlg->getRange();
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"revisions_dlg");
    dlg->saveDialogSize(_k);

    if (i==QDialog::Accepted) {
        svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision());
        m_Data->m_Model->svnWrapper()->makeTree(what,rev,r.first,r.second);
    }
}

void MainTreeWidget::slotLock()
{
    SvnItemList lst;
    SelectionList(lst);
    if (lst.count()==0) {
        KMessageBox::error(this,i18n("Nothing selected for unlock"));
        return;
    }
    Commitmsg_impl*ptr = 0;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&ptr,QString(i18n("Lock message")),true,"locking_log_msg");
    if (!dlg) return;
    ptr->initHistory();
    ptr->hideDepth(true);
    ptr->keepsLocks(false);

    QCheckBox*_stealLock = new QCheckBox();//"",0,"create_dir_checkbox");
    _stealLock->setObjectName("create_dir_checkbox");
    _stealLock->setText(i18n("Steal lock?"));
    ptr->addItemWidget(_stealLock);

    if (dlg->exec()!=QDialog::Accepted) {
        ptr->saveHistory(true);
        return;
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"locking_log_msg");
    dlg->saveDialogSize(_k);

    QString logMessage = ptr->getMessage();
    bool steal = _stealLock->isChecked();
    ptr->saveHistory(false);

    QStringList displist;
    for (int i=0;i<lst.count();++i){
        displist.append(lst[i]->fullName());
    }
    m_Data->m_Model->svnWrapper()->makeLock(displist,logMessage,steal);
    refreshCurrentTree();
}


/*!
    \fn MainTreeWidget::slotUnlock()
 */
void MainTreeWidget::slotUnlock()
{
    SvnItemList lst;
    SelectionList(lst);
    if (lst.count()==0) {
        KMessageBox::error(this,i18n("Nothing selected for unlock"));
        return;
    }
    int res = KMessageBox::questionYesNoCancel(this,i18n("Break lock or ignore missing locks?"),i18n("Unlocking items"));
    if (res == KMessageBox::Cancel) {
        return;
    }
    bool breakit = res==KMessageBox::Yes;

    QStringList displist;
    for (int i=0;i<lst.count();++i){
        displist.append(lst[i]->fullName());
    }
    m_Data->m_Model->svnWrapper()->makeUnlock(displist,breakit);
    refreshCurrentTree();
}

void MainTreeWidget::slotDisplayLastDiff()
{
    SvnItem*kitem = Selected();
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().toLocal8Bit());
    }
    svn::Revision end = svn::Revision::PREV;
    if (!kitem) {
        if (isWorkingCopy()) {
            kitem = m_Data->m_Model->firstRootChild();
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
        if (!m_Data->m_Model->svnWrapper()->singleInfo(what,baseRevision(),inf)) {
            return;
        }
        start = inf.cmtRev();
    } else {
        start = kitem->cmtRev();
    }
    if (!isWorkingCopy()) {
        if (!m_Data->m_Model->svnWrapper()->singleInfo(what,start.revnum()-1,inf)) {
            return;
        }
        end = inf.cmtRev();
    }
    m_Data->m_Model->svnWrapper()->makeDiff(what,end,what,start,realWidget());
}

void MainTreeWidget::slotSimpleBaseDiff()
{
    simpleWcDiff(Selected(),svn::Revision::BASE,svn::Revision::WORKING);
}

void MainTreeWidget::slotDirSimpleBaseDiff()
{
    simpleWcDiff(DirSelected(),svn::Revision::BASE,svn::Revision::WORKING);
}

void MainTreeWidget::slotSimpleHeadDiff()
{
    simpleWcDiff(Selected(),svn::Revision::WORKING,svn::Revision::HEAD);
}

void MainTreeWidget::simpleWcDiff(SvnItem*kitem,const svn::Revision&first,const svn::Revision&second)
{
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().toLocal8Bit());
    }

    if (!kitem) {
        what=".";
    }else{
        what = relativePath(kitem);
    }
    // only possible on working copies - so we may say this values
    m_Data->m_Model->svnWrapper()->makeDiff(what,first,second,svn::Revision::UNDEFINED,kitem?kitem->isDir():true);
}

void MainTreeWidget::slotDiffRevisions()
{
    SvnItem*k = Selected();
    QString what;
    if (isWorkingCopy())
    {
        chdir(baseUri().toLocal8Bit());
    }

    if (!k) {
        what=(isWorkingCopy()?".":baseUri());
    }else{
        what = relativePath(k);
    }
    Rangeinput_impl*rdlg = 0;
    KDialog*dlg = createOkDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        svn::Revision _peg=(isWorkingCopy()?svn::Revision::WORKING:baseRevision());
        m_Data->m_Model->svnWrapper()->makeDiff(what,r.first,r.second,_peg,k?k->isDir():true);
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"revisions_dlg");
    dlg->saveDialogSize(_k);
    delete dlg;

}

void MainTreeWidget::slotDiffPathes()
{
    SvnItemList lst;

    QObject * tr = sender();
    bool unique=false;

    if (tr==filesActions()->action("make_svn_diritemsdiff")) {
        unique=true;
        DirSelectionList(lst);
    } else {
        SelectionList(lst);
    }

    if (lst.count()!=2 || (!unique && !uniqueTypeSelected()) ) {
        return;
    }

    SvnItem*k1,*k2;
    k1 = lst[0];
    k2 = lst[1];
    QString w1,w2;
    svn::Revision r1;

    if (isWorkingCopy()) {
        chdir(baseUri().toLocal8Bit());
        w1 = relativePath(k1);
        w2 = relativePath(k2);
        r1 = svn::Revision::WORKING;
    } else {
        w1 = k1->fullName();
        w2 = k2->fullName();
        r1 = baseRevision();
    }
    m_Data->m_Model->svnWrapper()->makeDiff(w1,r1,w2,r1);
}

void MainTreeWidget::slotInfo()
{
    SvnItemList lst;
    SelectionList(lst);
    svn::Revision rev(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision());
    if (!isWorkingCopy()) {
        rev = baseRevision();
    }
    if (lst.count()==0) {
        if (!isWorkingCopy()) {
            QStringList _sl(baseUri());
            m_Data->m_Model->svnWrapper()->makeInfo(_sl,rev,svn::Revision::UNDEFINED,Kdesvnsettings::info_recursive());
        } else {
            lst.append(SelectedOrMain());
        }
    }
    if (lst.count()>0) {
        m_Data->m_Model->svnWrapper()->makeInfo(lst,rev,rev,Kdesvnsettings::info_recursive());
    }
}

void MainTreeWidget::slotBlame()
{
    SvnItem*k = Selected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    m_Data->m_Model->svnWrapper()->makeBlame(start,end,k);
}

void MainTreeWidget::slotRangeBlame()
{
    SvnItem*k = Selected();
    if (!k) return;
    Rangeinput_impl*rdlg = 0;
    svn::SharedPointer<KDialog> dlg = createOkDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_Data->m_Model->svnWrapper()->makeBlame(r.first,r.second,k);
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"revisions_dlg");
    dlg->saveDialogSize(_k);
}

void MainTreeWidget::_propListTimeout()
{
    dispProperties(false);
}

void MainTreeWidget::slotDisplayProperties()
{
    dispProperties(true);
}

void MainTreeWidget::refreshItem(SvnItemModelNode*node)
{
    if (node) {
        m_Data->m_Model->refreshItem(node);
    }
}

void MainTreeWidget::slotChangeProperties(const svn::PropertiesMap&pm,const QStringList&dellist,const QString&path)
{
    m_Data->m_Model->svnWrapper()->changeProperties(pm,dellist,path);
    SvnItemModelNode* which = SelectedNode();
    if (which && which->fullName()==path) {
        m_Data->m_Model->refreshItem(which);
        dispProperties(true);
    }
}

void MainTreeWidget::dispProperties(bool force)
{
    CursorStack a(Qt::BusyCursor);
    bool cache_Only = (!force && isNetworked() && !Kdesvnsettings::properties_on_remote_items());
    svn::PathPropertiesMapListPtr pm;
    SvnItem*k = Selected();
    if (!k || !k->isRealVersioned()) {
        emit sigProplist(svn::PathPropertiesMapListPtr(),false,false,QString(""));
        return;
    }
    svn::Revision rev(isWorkingCopy()?svn::Revision::WORKING:baseRevision());
    pm =m_Data->m_Model->svnWrapper()->propList(k->fullName(),rev,cache_Only);
    emit sigProplist(pm,isWorkingCopy(),k->isDir(),k->fullName());
}

void MainTreeWidget::slotCat()
{
    SvnItem*k = Selected();
    if (!k) return;
    m_Data->m_Model->svnWrapper()->slotMakeCat(isWorkingCopy()?svn::Revision::HEAD:baseRevision(), k->fullName(),k->shortName(),
        isWorkingCopy()?svn::Revision::HEAD:baseRevision(),0);
}

void MainTreeWidget::slotRevisionCat()
{
    SvnItem*k = Selected();
    if (!k) return;
    Rangeinput_impl*rdlg = 0;
    KDialog*dlg = createOkDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    rdlg->setStartOnly(true);
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        m_Data->m_Model->svnWrapper()->slotMakeCat(r.first, k->fullName(),k->shortName(),isWorkingCopy()?svn::Revision::WORKING:baseRevision(),0);
    }
    KConfigGroup _k(Kdesvnsettings::self()->config(),"revisions_dlg");
    dlg->saveDialogSize(_k);
    delete dlg;
}

void MainTreeWidget::slotResolved()
{
    if (!isWorkingCopy()) return;
    SvnItem*which= SelectedOrMain();
    if (!which) return;
    m_Data->m_Model->svnWrapper()->slotResolved(which->fullName());
    which->refreshStatus(true);
    //slotRescanIcons(false);
}

void MainTreeWidget::slotTryResolve()
{
    if (!isWorkingCopy()) return;
    SvnItem*which= Selected();
    if (!which || which->isDir()) {
        return;
    }
    m_Data->m_Model->svnWrapper()->slotResolve(which->fullName());
}

void MainTreeWidget::slotLeftDelete()
{
    SvnItemList lst;
    DirSelectionList(lst);
    makeDelete(lst);
}

void MainTreeWidget::slotDelete()
{
    SvnItemList lst;
    SelectionList(lst);
    makeDelete(lst);
}

void MainTreeWidget::makeDelete(const SvnItemList&lst)
{
    if (lst.size()==0) {
        KMessageBox::error(this,i18n("Nothing selected for delete"));
        return;
    }
    svn::Pathes items;
    QStringList displist;
    KUrl::List kioList;
    SvnItemList::const_iterator liter;
    for (liter=lst.begin();liter!=lst.end();++liter){
        if (!(*liter)->isRealVersioned()) {
            KUrl _uri; _uri.setPath((*liter)->fullName());
            kioList.append(_uri);
        } else {
            items.push_back((*liter)->fullName());
        }
        displist.append((*liter)->fullName());
    }

    DeleteForm_impl*ptr = 0;
    KDialog*dlg = createYesDialog(&ptr,QString(i18n("Really delete these entries?")),true,"delete_items_dialog",true);
    if (!dlg) {
        return;
    }
    ptr->setStringList(displist);
    ptr->showExtraButtons(isWorkingCopy() && items.size()>0);
    int _ans = dlg->exec();
    KConfigGroup _kc(Kdesvnsettings::self()->config(),"delete_items_dialog");
    dlg->saveDialogSize(_kc);
    bool force = ptr->force_delete();
    bool keep = ptr->keep_local();
    delete dlg;

    if (_ans == KDialog::Yes) {
        WidgetBlockStack st(this);
        if (kioList.count()>0) {
            KIO::Job*aJob = KIO::del(kioList);
            if (!aJob->exec()) {
                aJob->showErrorDialog(this);
                return;
            }
        }
        if (items.size()>0) {
            m_Data->m_Model->svnWrapper()->makeDelete(items,keep,force);
        }
        refreshCurrentTree();
    }
}

void MainTreeWidget::internalDrop(const KUrl::List&_lst,Qt::DropAction action,const QModelIndex&index)
{
    if (_lst.size()==0) {
        return;
    }
    KUrl::List lst = _lst;
    QString target;
    QString nProto;

    if (isWorkingCopy()) {
        nProto="";
    } else {
        nProto = svn::Url::transformProtokoll(lst[0].protocol());
    }
    KUrl::List::iterator it = lst.begin();
    QStringList l;
    for (;it!=lst.end();++it) {
        l = (*it).prettyUrl().split('?');
        if (l.size()>1) {
            (*it) = l[0];
        } else if (isWorkingCopy())
        {
            (*it) = KUrl::fromPathOrUrl( (*it).path());
        }
        (*it).setProtocol(nProto);
        kDebug()<<"Dropped: "<<(*it)<<endl;
    }

    if (index.isValid()) {
        SvnItemModelNode*node=static_cast<SvnItemModelNode*>(index.internalPointer());
        target=node->fullName();
    } else {
        target=baseUri();
    }
    if (action==Qt::MoveAction) {
        m_Data->m_Model->svnWrapper()->makeMove(lst,target,false);
    } else if (action==Qt::CopyAction) {
        m_Data->m_Model->svnWrapper()->makeCopy(lst,target,(isWorkingCopy()?svn::Revision::UNDEFINED:baseRevision()));
    }
    refreshCurrentTree();
}

void MainTreeWidget::slotUrlDropped(const KUrl::List&_lst,Qt::DropAction action,const QModelIndex&index,bool intern)
{
    if (_lst.size()==0) {
        return;
    }
    if (intern) {
        internalDrop(_lst,action,index);
        return;
    }
    QString target;
    if (index.isValid()) {
        SvnItemModelNode*node=static_cast<SvnItemModelNode*>(index.internalPointer());
        target=node->fullName();
    } else {
        target=baseUri();
    }

    if (baseUri().length()==0) {
        openUrl(_lst[0]);
        return;
    }
    QString path = _lst[0].path();
    QFileInfo fi(path);
    if  (!isWorkingCopy()) {
        if (!fi.isDir()) {
            target+= '/'+_lst[0].fileName();
        }
        slotImportIntoDir(_lst[0],target,fi.isDir());
    } else {
        WidgetBlockStack(this);
        //m_pList->stopScan();
        KIO::Job * job = KIO::copy(_lst,target);
        connect(job, SIGNAL(result(KJob*)),SLOT(slotCopyFinished( KJob*)));
        job->exec();
    }
}

void MainTreeWidget::slotCopyFinished(KJob*_job)
{
    if (!_job) {
        return;
    }
    KIO::Job * job = static_cast<KIO::Job*>(_job);
    bool ok = true;
    if (job->error()) {
        job->showErrorDialog(this);
        ok = false;
    }
    if (ok) {
        KUrl::List lst = static_cast<KIO::CopyJob*>(job)->srcUrls();
        KUrl turl = static_cast<KIO::CopyJob*>(job)->destUrl();
        QString base = turl.path(KUrl::AddTrailingSlash);
        KUrl::List::iterator iter;
        svn::Pathes tmp;
        for (iter=lst.begin();iter!=lst.end();++iter) {
            QString _ne = base+(*iter).fileName(KUrl::IgnoreTrailingSlash);
            tmp.push_back(svn::Path(_ne));
        }
        m_Data->m_Model->svnWrapper()->addItems(tmp,svn::DepthInfinity);
    }
    refreshCurrentTree();
}

void MainTreeWidget::stopLogCache()
{
    QAction*temp = filesActions()->action("update_log_cache");
    m_Data->m_Model->svnWrapper()->stopFillCache();
    if (temp) {
        temp->setText(i18n("Update log cache"));
    }
}

void MainTreeWidget::slotUpdateLogCache()
{
    if (baseUri().length()>0 && m_Data->m_Model->svnWrapper()->doNetworking()) {
        QAction*temp = filesActions()->action("update_log_cache");
        if (!m_Data->m_Model->svnWrapper()->threadRunning(SvnActions::fillcachethread)) {
            m_Data->m_Model->svnWrapper()->startFillCache(baseUri());
            if (temp) {
                temp->setText(i18n("Stop updating the logcache"));
            }
        } else {
            m_Data->m_Model->svnWrapper()->stopFillCache();
            if (temp) {
                temp->setText(i18n("Update log cache"));
            }
        }
    }
}

void MainTreeWidget::slotMkBaseDirs()
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
    isopen = m_Data->m_Model->svnWrapper()->makeMkdir(targets,msg);
    if (isopen) {
        refreshCurrentTree();
    }
}

void MainTreeWidget::slotMkdir()
{
    SvnItemModelNode*k = SelectedNode();
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
    QString ex = m_Data->m_Model->svnWrapper()->makeMkdir(parentDir);
    if (!ex.isEmpty()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir*>(k),true,true);
    }
}

void MainTreeWidget::slotRename()
{
    copy_move(true);
}
void MainTreeWidget::slotCopy()
{
    copy_move(false);
}

void MainTreeWidget::copy_move(bool move)
{
    if (isWorkingCopy()&&SelectedNode()==m_Data->m_Model->firstRootChild()) {
        return;
    }
    bool ok, force;
    SvnItemModelNode*which = SelectedNode();
    if (!which) return;
    QString nName =  CopyMoveView_impl::getMoveCopyTo(&ok,&force,move,which->fullName(),baseUri(),this,"move_name");
    if (!ok) {
        return;
    }
    if (move) {
        m_Data->m_Model->svnWrapper()->makeMove(which->fullName(),nName,force);
    } else {
        m_Data->m_Model->svnWrapper()->makeCopy(which->fullName(),nName, isWorkingCopy()?svn::Revision::HEAD:baseRevision());
    }
}

void MainTreeWidget::slotCleanupAction()
{
    if (!isWorkingCopy()) return;
    SvnItemModelNode*which= SelectedNode();
    if (!which) which = m_Data->m_Model->firstRootChild();
    if (!which||!which->isDir()) return;
    if (m_Data->m_Model->svnWrapper()->makeCleanup(which->fullName())) {
        which->refreshStatus(true);
    }
}

void MainTreeWidget::slotMergeRevisions()
{
    if (!isWorkingCopy()) return;
    SvnItemModelNode*which=SelectedNode();
    if (!which) {
        return;
    }
    bool force,dry,rec,irelated,useExternal;
    Rangeinput_impl::revision_range range;
    if (!MergeDlg_impl::getMergeRange(range,&force,&rec,&irelated,&dry,&useExternal,this,"merge_range")) {
        return;
    }
    if (!useExternal) {
        m_Data->m_Model->svnWrapper()->slotMergeWcRevisions(which->fullName(),range.first,range.second,rec,!irelated,force,dry);
    } else {
        m_Data->m_Model->svnWrapper()->slotMergeExternal(which->fullName(),which->fullName(),which->fullName(),
                                                          range.first,range.second,
                                                          isWorkingCopy()?svn::Revision::UNDEFINED:m_Data->m_remoteRevision,
                                                          rec);
    }
    refreshItem(which);
    if (which->isDir()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir*>(which),true,false);
    }
}

void MainTreeWidget::slotMerge()
{
    SvnItemModelNode*which=SelectedNode();
    QString src1,src2,target;
    if (isWorkingCopy()) {
        if (m_Data->merge_Target.isEmpty()) {
            target = which?which->fullName():baseUri();
        } else {
            target = m_Data->merge_Target;
        }
        src1 = m_Data->merge_Src1;
    } else {
        if (m_Data->merge_Src1.isEmpty()){
            src1 = which?which->fullName():baseUri();
        } else {
            src1 = m_Data->merge_Src1;
        }
        target = m_Data->merge_Target;
    }
    src2 = m_Data->merge_Src2;
    bool force,dry,rec,irelated,useExternal,recordOnly,reintegrate;
    Rangeinput_impl::revision_range range;
    MergeDlg_impl*ptr = 0;
    KDialog*dlg = createOkDialog(&ptr,QString(i18n("Merge")),true,"merge_dialog",true);
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
        m_Data->merge_Src2 = src2;
        m_Data->merge_Src1 = src1;
        m_Data->merge_Target = target;
        force = ptr->force();
        dry = ptr->dryrun();
        rec = ptr->recursive();
        irelated = ptr->ignorerelated();
        useExternal = ptr->useExtern();
        recordOnly = ptr->recordOnly();
        range = ptr->getRange();
        reintegrate=ptr->reintegrate();
        if (!useExternal) {
            m_Data->m_Model->svnWrapper()->slotMerge(src1,src2,target,range.first,range.second,
                                                      isWorkingCopy()?svn::Revision::UNDEFINED:m_Data->m_remoteRevision,
                                                      rec,!irelated,force,dry,recordOnly,reintegrate);
        } else {
            m_Data->m_Model->svnWrapper()->slotMergeExternal(src1,src2,target,range.first,range.second,
                                                              isWorkingCopy()?svn::Revision::UNDEFINED:m_Data->m_remoteRevision,
                                                              rec);
        }
        if (isWorkingCopy()) {
            //            refreshItem(which);
            //            refreshRecursive(which);
            refreshCurrentTree();
        }
    }

    KConfigGroup _k(Kdesvnsettings::self()->config(),"merge_dialog");
    dlg->saveDialogSize(_k);

    delete dlg;
    enableActions();
}

void MainTreeWidget::slotRelocate()
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
    CheckoutInfo_impl*ptr = 0;
    KDialog * dlg = createOkDialog(&ptr,i18n("Relocate path %1",path),true,"relocate_dlg");
    if (dlg) {
        ptr->setStartUrl(fromUrl);
        ptr->disableAppend(true);
        ptr->disableTargetDir(true);
        ptr->disableRange(true);
        ptr->disableOpen(true);
        ptr->disableExternals(true);
        ptr->hideDepth(true,true);
        bool done = false;
        KConfigGroup _k(Kdesvnsettings::self()->config(),"relocate_dlg");
        dlg->restoreDialogSize(_k);
        if (dlg->exec()==QDialog::Accepted) {
            done = m_Data->m_Model->svnWrapper()->makeRelocate(fromUrl,ptr->reposURL(),path,ptr->overwrite());
        }
        dlg->saveDialogSize(_k);
        delete dlg;
        if (!done) return;
    }
    refreshItem(k->sItem());
}

void MainTreeWidget::slotImportDirsIntoCurrent()
{
    slotImportIntoCurrent(true);
}

/*!
\fn MainTreeWidget::slotImportIntoCurrent()
*/
void MainTreeWidget::slotImportIntoCurrent(bool dirs)
{
    if (selectionCount()>1) {
        KMessageBox::error(this,i18n("Cannot import into multiple targets!"));
        return;
    }
    QString targetUri;
    if (selectionCount()==0) {
        targetUri=baseUri();
    } else {
        targetUri = SelectedNode()->Url();
    }
    KUrl uri;
    if (dirs) uri = KFileDialog::getExistingDirectory(KUrl(),this,"Import files from folder");
    else uri = KFileDialog::getImageOpenUrl(KUrl(),this,"Import file");

    if (uri.url().isEmpty()) return;

    slotImportIntoDir(uri,targetUri,dirs);
}

void MainTreeWidget::slotImportIntoDir(const KUrl&importUrl,const QString&target,bool dirs)
{
    Commitmsg_impl*ptr = 0;
    Importdir_logmsg*ptr2 = 0;

    KDialog*dlg;
    KUrl uri = importUrl;
    if ( !uri.protocol().isEmpty() && uri.protocol()!="file") {
        KMessageBox::error(this,i18n("Cannot import remote urls!"));
        return;
    }
    QString targetUri = target;
    while (targetUri.endsWith('/')) {
        targetUri.truncate(targetUri.length()-1);
    }

    if (dirs) {
        dlg = createOkDialog(&ptr2,QString(i18n("Import log")),true,"import_log_msg");
        ptr = ptr2;
        ptr2->createDirboxDir("\""+uri.fileName()+"\"");
    } else {
        dlg = createOkDialog(&ptr,QString(i18n("Import log")),true,"import_log_msg");
    }

    if (!dlg) return;

    ptr->initHistory();
    KConfigGroup _k(Kdesvnsettings::self()->config(),"import_log_msg");
    if (dlg->exec()!=QDialog::Accepted) {
        ptr->saveHistory(true);
        dlg->saveDialogSize(_k);
        delete dlg;
        return;
    }
    dlg->saveDialogSize(_k);

    QString logMessage = ptr->getMessage();
    svn::Depth rec = ptr->getDepth();
    ptr->saveHistory(false);
    uri.setProtocol("");
    QString iurl = uri.path();
    while (iurl.endsWith('/')) {
        iurl.truncate(iurl.length()-1);
    }
    if (dirs && ptr2 && ptr2->createDir()) {
        targetUri+= '/'+uri.fileName();
    }
    if (ptr2) {
        m_Data->m_Model->svnWrapper()->slotImport(iurl,targetUri,logMessage,rec,ptr2->noIgnore(),ptr2->ignoreUnknownNodes());
    } else {
        m_Data->m_Model->svnWrapper()->slotImport(iurl,targetUri,logMessage,rec,false,false);
    }
    if (!isWorkingCopy()) {
        if (selectionCount()==0) {
            refreshCurrentTree();
        } else {
            m_Data->m_Model->refreshItem(SelectedNode());
        }
    }
    delete dlg;
}

void MainTreeWidget::slotChangeToRepository()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItemModelNode*k = m_Data->m_Model->firstRootChild();
    /* huh... */
    if (!k) return;
    svn::InfoEntry i;
    if (!m_Data->m_Model->svnWrapper()->singleInfo(k->Url(),svn::Revision::UNDEFINED,i)) {
        return;
    }
    if (i.reposRoot().isEmpty()) {
        KMessageBox::sorry(KApplication::activeModalWidget(),i18n("Could not retrieve repository of working copy."),i18n("SVN Error"));
    } else {
        sigSwitchUrl(i.reposRoot());
    }
}

void MainTreeWidget::slotCheckNewItems()
{
    if (!isWorkingCopy()) {
        KMessageBox::sorry(0,i18n("Only in working copy possible."),i18n("Error"));
        return;
    }
    if (selectionCount()>1) {
        KMessageBox::sorry(0,i18n("Only on single folder possible"),i18n("Error"));
        return;
    }
    SvnItem*w = SelectedOrMain();
    if (!w) {
        KMessageBox::sorry(0,i18n("Sorry - internal error!"),i18n("Error"));
        return;
    }
    m_Data->m_Model->svnWrapper()->checkAddItems(w->fullName(),true);
}

void MainTreeWidget::refreshCurrent(SvnItem*cur)
{
    if (!cur||!cur->sItem()) {
        refreshCurrentTree();
        return;
    }
    kapp->processEvents();
    setUpdatesEnabled(false);
    if (cur->isDir()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir*>(cur->sItem()));
    } else {
        m_Data->m_Model->refreshItem(cur->sItem());
    }
    setUpdatesEnabled(true);
    m_TreeView->viewport()->repaint();
}

void MainTreeWidget::slotReinitItem(SvnItem*item)
{
    if (!item) {
        return;
    }
    SvnItemModelNode*k = item->sItem();
    if (!k) {
        return;
    }
    m_Data->m_Model->refreshItem(k);
    if (k->isDir()) {
        m_Data->m_Model->clearNodeDir(static_cast<SvnItemModelNodeDir*>(k));
    }
}

void MainTreeWidget::keyPressEvent(QKeyEvent*event)
{
    if ((event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) && !event->isAutoRepeat()) {
        QModelIndex index = SelectedIndex();
        if (index.isValid()) {
            itemActivated(index,true);
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

void MainTreeWidget::slotItemExpanded(const QModelIndex&)
{
}

void MainTreeWidget::slotItemsInserted(const QModelIndex&)
{
    resizeAllColumns();
}

void MainTreeWidget::slotDirSelectionChanged(const QItemSelection&_item,const QItemSelection&)
{
    QModelIndexList _indexes = _item.indexes();
    switch (DirselectionCount()) {
    case 1:
        m_DirTreeView->setStatusTip(i18n("Hold CTRL key while click on selected item for unselect"));
        break;
    case 2:
        m_DirTreeView->setStatusTip(i18n("See context menu for more actions"));
        break;
    case 0:
        m_DirTreeView->setStatusTip(i18n("Click for navigate"));
        break;
    default:
        m_DirTreeView->setStatusTip(i18n("Navigation"));
        break;
    }
    if (_indexes.size()>=1) {
        QModelIndex ind = _indexes[0];
        QModelIndex _t =m_Data->srcDirInd(ind);
        if (m_Data->m_Model->canFetchMore(_t)) {
            WidgetBlockStack st(m_TreeView);
            WidgetBlockStack st2(m_DirTreeView);
            m_Data->m_Model->fetchMore(_t);
        }
        _t = m_Data->m_SortModel->mapFromSource(_t);
        if (Kdesvnsettings::show_navigation_panel()) m_TreeView->setRootIndex(_t);
    } else {
        m_TreeView->setRootIndex(QModelIndex());
    }
    if (m_TreeView->selectionModel()->hasSelection()) {
        m_TreeView->selectionModel()->clearSelection();
    } else {
        enableActions();
    }
    resizeAllColumns();
}

void MainTreeWidget::slotCommit()
{
    SvnItemList which;
    SelectionList(which);
    m_Data->m_Model->svnWrapper()->doCommit(which);
}

void MainTreeWidget::slotDirCommit()
{
    SvnItemList which;
    DirSelectionList(which);
    m_Data->m_Model->svnWrapper()->doCommit(which);
}

void MainTreeWidget::slotDirUpdate()
{
    SvnItemList which;
    DirSelectionList(which);
    QStringList what;
    if (which.count()==0) {
        what.append(baseUri());
    } else {
        SvnItemListConstIterator liter=which.begin();
        for(;liter!=which.end();++liter){
            what.append((*liter)->fullName());
        }
    }
    m_Data->m_Model->svnWrapper()->makeUpdate(what,svn::Revision::HEAD,svn::DepthUnknown);
}

void MainTreeWidget::slotRescanIcons()
{
    m_Data->m_Model->refreshIndex(m_Data->m_Model->firstRootIndex());
}

void MainTreeWidget::checkUseNavigation(bool startup)
{
    bool use = Kdesvnsettings::show_navigation_panel();
    if (use) {
        m_TreeView->collapseAll();
    }
    m_TreeView->setExpandsOnDoubleClick(!use);
    m_TreeView->setRootIsDecorated(!use);
    m_TreeView->setItemsExpandable(!use);
    QList<int> si;
    if (use) {
        if (!startup) {
            si = m_ViewSplitter->sizes();
            if (si.size()==2 && si[0]<5) {
                si[0]=200;
                m_ViewSplitter->setSizes(si);
            }
            m_DirTreeView->selectionModel()->clearSelection();
        }
    } else {
        si << 0 << 300;
        m_ViewSplitter->setSizes(si);

    }
    m_TreeView->setRootIndex(QModelIndex());
}

void MainTreeWidget::slotRepositorySettings()
{
    if (baseUri().length()==0) {
        return;
    }
    svn::InfoEntry inf;
    if (!m_Data->m_Model->svnWrapper()->singleInfo(baseUri(),baseRevision(),inf)) {
        return;
    }
    if (inf.reposRoot().isEmpty()) {
        KMessageBox::sorry(KApplication::activeModalWidget(),i18n("Could not retrieve repository."),i18n("SVN Error"));
    } else {
        DbSettings::showSettings(inf.reposRoot());
    }
}

void MainTreeWidget::slotRightProperties()
{
    SvnItem*k = Selected();
    if (!k) return;
    m_Data->m_Model->svnWrapper()->editProperties(k,isWorkingCopy()?svn::Revision::WORKING:svn::Revision::HEAD);
}

void MainTreeWidget::slotLeftProperties()
{
    SvnItem*k = DirSelected();
    if (!k) return;
    m_Data->m_Model->svnWrapper()->editProperties(k,isWorkingCopy()?svn::Revision::WORKING:svn::Revision::HEAD);
}

void MainTreeWidget::slotDirRecProperty()
{
    SvnItem*k = DirSelected();
    if (!k) return;
    SetPropertyWidget*ptr = 0;
    KDialog*dlg = createOkDialog(&ptr,QString(i18n("Set/add property recursive")),true,"property_dlg");
    if (!dlg) return;

    KConfigGroup _k(Kdesvnsettings::self()->config(),"property_dlg");
    DialogStack _s(dlg,_k);
    if (dlg->exec()!=QDialog::Accepted) {
        return;
    }


}

#include "maintreewidget.moc"
