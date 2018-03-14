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
#include "settings/kdesvnsettings.h"
#include "helpers/sshagent.h"
#include "svnqt/targets.h"
#include "svnqt/url.h"
#include "fronthelpers/rangeinput_impl.h"
#include "fronthelpers/widgetblockstack.h"
#include "fronthelpers/fronthelpers.h"
#include "ksvnwidgets/commitmsg_impl.h"
#include "ksvnwidgets/deleteform.h"
#include "helpers/kdesvn_debug.h"
#include "opencontextmenu.h"
#include "EditIgnorePattern.h"

#include <kjobwidgets.h>
#include <kjobuidelegate.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <kauthorized.h>
#include <kmimetypetrader.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <unistd.h>

#include <QApplication>
#include <QCheckBox>
#include <QKeyEvent>
#include <QMap>
#include <QUrlQuery>
#include <QTimer>

class MainTreeWidgetData
{
public:
    MainTreeWidgetData()
    {
        m_Collection = nullptr;
        m_Model = nullptr;
        m_SortModel = nullptr;
        m_DirSortModel = nullptr;
        m_remoteRevision = svn::Revision::UNDEFINED;
    }

    ~MainTreeWidgetData()
    {
        delete m_Model;
        delete m_SortModel;
        delete m_DirSortModel;
    }

    QModelIndex srcInd(const QModelIndex &ind)
    {
        return m_SortModel->mapToSource(ind);
    }

    QModelIndex srcDirInd(const QModelIndex &ind)
    {
        return m_DirSortModel->mapToSource(ind);
    }

    SvnItemModelNode *sourceNode(const QModelIndex &index, bool left)
    {
        if (!index.isValid()) {
            return nullptr;
        }
        QModelIndex ind = left ? m_DirSortModel->mapToSource(index) : m_SortModel->mapToSource(index);
        if (ind.isValid()) {
            return static_cast<SvnItemModelNode *>(ind.internalPointer());
        }
        return nullptr;
    }

    KActionCollection *m_Collection;
    SvnItemModel *m_Model;
    SvnSortFilterProxy *m_SortModel;
    SvnDirSortFilterProxy *m_DirSortModel;
    svn::Revision m_remoteRevision;
    QString merge_Target, merge_Src2, merge_Src1;

    QTimer m_TimeModified, m_TimeUpdates, m_resizeColumnsTimer;
};

MainTreeWidget::MainTreeWidget(KActionCollection *aCollection, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), m_Data(new MainTreeWidgetData)
{
    setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    m_TreeView->setFocusPolicy(Qt::StrongFocus);
    m_Data->m_Collection = aCollection;
    m_Data->m_SortModel = new SvnSortFilterProxy();
    m_Data->m_SortModel->setDynamicSortFilter(true);
    m_Data->m_SortModel->setSortRole(SORT_ROLE);
    m_Data->m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort() ? Qt::CaseSensitive : Qt::CaseInsensitive);
    m_Data->m_SortModel->sort(0);
    m_TreeView->setModel(m_Data->m_SortModel);
    m_TreeView->sortByColumn(0, Qt::AscendingOrder);
    m_Data->m_Model = new SvnItemModel(this);
    m_Data->m_SortModel->setSourceModel(m_Data->m_Model);

    m_Data->m_DirSortModel = new SvnDirSortFilterProxy();
    m_Data->m_DirSortModel->setDynamicSortFilter(true);
    m_Data->m_DirSortModel->setSortRole(SORT_ROLE);
    m_Data->m_DirSortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort() ? Qt::CaseSensitive : Qt::CaseInsensitive);

    m_DirTreeView->setModel(m_Data->m_DirSortModel);
    m_Data->m_DirSortModel->setSourceModel(m_Data->m_Model);

    connect(m_TreeView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));

    connect(m_DirTreeView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotDirSelectionChanged(QItemSelection,QItemSelection)));

    connect(m_Data->m_Model->svnWrapper(), SIGNAL(clientException(QString)), this, SLOT(slotClientException(QString)));
    connect(m_Data->m_Model, SIGNAL(clientException(QString)), this, SLOT(slotClientException(QString)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sendNotify(QString)), this, SLOT(slotNotifyMessage(QString)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(reinitItem(SvnItem*)), this, SLOT(slotReinitItem(SvnItem*)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigRefreshAll()), this, SLOT(refreshCurrentTree()));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigRefreshCurrent(SvnItem*)), this, SLOT(refreshCurrent(SvnItem*)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigRefreshItem(QString)), this, SLOT(slotRefreshItem(QString)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigGotourl(QUrl)), this, SLOT(_openUrl(QUrl)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigCacheStatus(qlonglong,qlonglong)), this, SIGNAL(sigCacheStatus(qlonglong,qlonglong)));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigThreadsChanged()), this, SLOT(enableActions()));
    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigCacheDataChanged()), this, SLOT(slotCacheDataChanged()));

    connect(m_Data->m_Model->svnWrapper(), SIGNAL(sigExtraStatusMessage(QString)), this, SIGNAL(sigExtraStatusMessage(QString)));

    connect(m_Data->m_Model, SIGNAL(urlDropped(QList<QUrl>,Qt::DropAction,QModelIndex,bool)),
            this, SLOT(slotUrlDropped(QList<QUrl>,Qt::DropAction,QModelIndex,bool)));

    connect(m_Data->m_Model, SIGNAL(itemsFetched(QModelIndex)), this, SLOT(slotItemsInserted(QModelIndex)));

    m_TreeView->sortByColumn(0, Qt::AscendingOrder);
    m_DirTreeView->sortByColumn(0, Qt::AscendingOrder);

    checkUseNavigation(true);
    setupActions();

    m_Data->m_TimeModified.setParent(this);
    connect(&(m_Data->m_TimeModified), SIGNAL(timeout()), this, SLOT(slotCheckModified()));
    m_Data->m_TimeUpdates.setParent(this);
    connect(&(m_Data->m_TimeUpdates), SIGNAL(timeout()), this, SLOT(slotCheckUpdates()));
    m_Data->m_resizeColumnsTimer.setSingleShot(true);
    m_Data->m_resizeColumnsTimer.setParent(this);
    connect(&(m_Data->m_resizeColumnsTimer), SIGNAL(timeout()), this, SLOT(resizeAllColumns()));
}

MainTreeWidget::~MainTreeWidget()
{
    delete m_Data;
}

void MainTreeWidget::_openUrl(const QUrl &url)
{
    openUrl(url, true);
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

bool MainTreeWidget::openUrl(const QUrl &url, bool noReinit)
{

#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif

    CursorStack a;
    m_Data->m_Model->svnWrapper()->killallThreads();
    clear();
    emit sigProplist(svn::PathPropertiesMapListPtr(new svn::PathPropertiesMapList()), false, false, QString());

    if (!noReinit) {
        m_Data->m_Model->svnWrapper()->reInitClient();
    }

    QUrl _url(url);
    const QString proto = svn::Url::transformProtokoll(url.scheme());
    _url = _url.adjusted(QUrl::StripTrailingSlash|QUrl::NormalizePathSegments);
    _url.setScheme(proto);

    const QString baseUriString = _url.url(QUrl::StripTrailingSlash);
    const QVector<QStringRef> s = baseUriString.splitRef(QLatin1Char('?'));
    if (s.size() > 1) {
        setBaseUri(s.first().toString());
    } else {
        setBaseUri(baseUriString);
    }
    setWorkingCopy(false);
    setNetworked(false);
    m_Data->m_remoteRevision = svn::Revision::HEAD;

    if (QLatin1String("svn+file") == url.scheme()) {
        setBaseUri(url.path());
    } else {
        if (url.isLocalFile()) {
            QFileInfo fi(url.path());
            if (fi.exists() && fi.isSymLink()) {
                const QString sl = fi.readLink();
                if (sl.startsWith(QLatin1Char('/'))) {
                    setBaseUri(sl);
                } else {
                    fi.setFile(fi.path() + QLatin1Char('/') + sl);
                    setBaseUri(fi.absoluteFilePath());
                }
            } else {
                setBaseUri(url.path());
            }
            QUrl _dummy;
            qCDebug(KDESVN_LOG) << "check if " << baseUri() << " is a local wc ...";
            if (m_Data->m_Model->svnWrapper()->isLocalWorkingCopy(baseUri(), _dummy)) {
                setWorkingCopy(true);
                // make sure a valid path is stored as baseuri
                setBaseUri(url.toLocalFile());
                qCDebug(KDESVN_LOG) << "... yes -> " << baseUri();
            } else {
                setWorkingCopy(false);
                // make sure a valid url is stored as baseuri
                setBaseUri(url.toString());
                qCDebug(KDESVN_LOG) << "... no -> " << baseUri();
            }
        } else {
            setNetworked(true);
            if (!Kdesvnsettings::network_on()) {
                setBaseUri(QString());
                setNetworked(false);
                clear();
                KMessageBox::error(this, i18n("Networked URL to open but networking is disabled."));
                emit changeCaption(QString());
                emit sigUrlOpend(false);
                return false;
            }
        }
    }
    const QList<QPair<QString, QString>> q = QUrlQuery(url).queryItems();
    typedef QPair<QString, QString> queryPair;
    Q_FOREACH(const queryPair &p, q) {
        if (p.first == QLatin1String("rev")) {
            const QString v = p.second;
            svn::Revision tmp;
            m_Data->m_Model->svnWrapper()->svnclient()->url2Revision(v, m_Data->m_remoteRevision, tmp);
            if (m_Data->m_remoteRevision == svn::Revision::UNDEFINED) {
                m_Data->m_remoteRevision = svn::Revision::HEAD;
            }
        }
    }
    if (url.scheme() == QLatin1String("svn+ssh") ||
            url.scheme() == QLatin1String("ksvn+ssh")) {
        SshAgent ssh;
        ssh.addSshIdentities();
    }
    m_Data->m_Model->svnWrapper()->clearUpdateCache();
    if (isWorkingCopy()) {
        m_Data->m_Model->initDirWatch();
    }
    bool result = m_Data->m_Model->checkDirs(baseUri(), nullptr) > -1;
    if (result && isWorkingCopy()) {
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
        m_DirTreeView->expandToDepth(0);
        m_DirTreeView->selectionModel()->select(m_Data->m_DirSortModel->mapFromSource(m_Data->m_Model->firstRootIndex()), QItemSelectionModel::Select);
    }
    resizeAllColumns();

    if (!result) {
        setBaseUri(QString());
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
        m_Data->m_Model->svnWrapper()->startFillCache(baseUri(), true);
    }
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Starting cache " << _counttime.elapsed();
    _counttime.restart();
#endif
    emit changeCaption(baseUri());
    emit sigUrlOpend(result);
    emit sigUrlChanged(baseUriAsUrl());
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Fired signals " << _counttime.elapsed();
    _counttime.restart();
#endif

    QTimer::singleShot(1, this, SLOT(readSupportData()));
    enableActions();
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Enabled actions " << _counttime.elapsed();
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

svn::Revision MainTreeWidget::baseRevision()const
{
    return m_Data->m_remoteRevision;
}

QWidget *MainTreeWidget::realWidget()
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

SvnItemList MainTreeWidget::SelectionList()const
{
    SvnItemList ret;
    const QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    ret.reserve(_mi.size());
    if (_mi.isEmpty()) {
        QModelIndex ind = m_TreeView->rootIndex();
        if (ind.isValid()) {
            // really! it will remapped to this before setRootIndex! (see below)
            ret.push_back(m_Data->sourceNode(ind, false));
        }
        return ret;
    }
    for (int i = 0; i < _mi.count(); ++i) {
        ret.push_back(m_Data->sourceNode(_mi[i], false));
    }
    return ret;
}

SvnItemList MainTreeWidget::DirSelectionList()const
{
    SvnItemList ret;
    const QModelIndexList _mi = m_DirTreeView->selectionModel()->selectedRows(0);
    ret.reserve(_mi.size());
    for (int i = 0; i < _mi.count(); ++i) {
        ret.push_back(m_Data->sourceNode(_mi[i], true));
    }
    return ret;
}

QModelIndex MainTreeWidget::SelectedIndex()const
{
    const QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    if (_mi.count() != 1) {
        if (_mi.isEmpty()) {
            const QModelIndex ind = m_TreeView->rootIndex();
            if (ind.isValid()) {
                return m_Data->m_SortModel->mapToSource(ind);
            }
        }
        return QModelIndex();
    }
    return m_Data->m_SortModel->mapToSource(_mi[0]);
}

QModelIndex MainTreeWidget::DirSelectedIndex()const
{
    const QModelIndexList _mi = m_DirTreeView->selectionModel()->selectedRows(0);
    if (_mi.count() != 1) {
        return QModelIndex();
    }
    return m_Data->m_DirSortModel->mapToSource(_mi[0]);
}

SvnItemModelNode *MainTreeWidget::SelectedNode()const
{
    const QModelIndex index = SelectedIndex();
    if (index.isValid()) {
        SvnItemModelNode *item = static_cast<SvnItemModelNode *>(index.internalPointer());
        return item;
    }
    return nullptr;
}

SvnItemModelNode *MainTreeWidget::DirSelectedNode()const
{
    const QModelIndex index = DirSelectedIndex();
    if (index.isValid()) {
        SvnItemModelNode *item = static_cast<SvnItemModelNode *>(index.internalPointer());
        return item;
    }
    return nullptr;
}

void MainTreeWidget::slotSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    enableActions();
    QTimer::singleShot(100, this, SLOT(_propListTimeout()));
}

SvnItem *MainTreeWidget::Selected()const
{
    return SelectedNode();
}

SvnItem *MainTreeWidget::DirSelected()const
{
    return DirSelectedNode();
}

SvnItem *MainTreeWidget::DirSelectedOrMain()const
{
    SvnItem *_item = DirSelected();
    if (_item == nullptr && isWorkingCopy()) {
        _item = m_Data->m_Model->firstRootChild();
    }
    return _item;
}

SvnItem *MainTreeWidget::SelectedOrMain()const
{
    SvnItem *_item = Selected();
    if (_item == nullptr && isWorkingCopy()) {
        _item = m_Data->m_Model->firstRootChild();
    }
    return _item;
}

void MainTreeWidget::setupActions()
{
    if (!m_Data->m_Collection) {
        return;
    }
    QAction *tmp_action;
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    tmp_action = add_action(QStringLiteral("make_svn_log_full"), i18n("History of item"), QKeySequence(Qt::CTRL | Qt::Key_L), QIcon::fromTheme(QStringLiteral("kdesvnlog")), this, SLOT(slotMakeLog()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item"));
    tmp_action = add_action(QStringLiteral("make_svn_log_nofollow"), i18n("History of item ignoring copies"), QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_L), QIcon::fromTheme(QStringLiteral("kdesvnlog")), this, SLOT(slotMakeLogNoFollow()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item without following copies"));

    tmp_action = add_action(QStringLiteral("make_svn_dir_log_nofollow"), i18n("History of item ignoring copies"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnlog")), this, SLOT(slotDirMakeLogNoFollow()));
    tmp_action->setIconText(i18n("History"));
    tmp_action->setStatusTip(i18n("Displays the history log of selected item without following copies"));

    tmp_action = add_action(QStringLiteral("make_svn_tree"), i18n("Full revision tree"), QKeySequence(Qt::CTRL | Qt::Key_T), QIcon::fromTheme(QStringLiteral("kdesvntree")), this, SLOT(slotMakeTree()));
    tmp_action->setStatusTip(i18n("Shows history of item as linked tree"));
    tmp_action = add_action(QStringLiteral("make_svn_partialtree"), i18n("Partial revision tree"), QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_T), QIcon::fromTheme(QStringLiteral("kdesvntree")), this, SLOT(slotMakePartTree()));
    tmp_action->setStatusTip(i18n("Shows history of item as linked tree for a revision range"));

    tmp_action = add_action(QStringLiteral("make_svn_property"), i18n("Properties"), QKeySequence(Qt::CTRL | Qt::Key_P), QIcon(), this, SLOT(slotRightProperties()));
    tmp_action = add_action(QStringLiteral("make_left_svn_property"), i18n("Properties"), QKeySequence(), QIcon(), this, SLOT(slotLeftProperties()));
    add_action(QStringLiteral("get_svn_property"), i18n("Display Properties"), QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_P), QIcon(), this, SLOT(slotDisplayProperties()));
    tmp_action = add_action(QStringLiteral("make_last_change"), i18n("Display last changes"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotDisplayLastDiff()));
    tmp_action->setToolTip(i18n("Display last changes as difference to previous commit."));
    tmp_action = add_action(QStringLiteral("make_svn_info"), i18n("Details"), QKeySequence(Qt::CTRL | Qt::Key_I), QIcon::fromTheme(QStringLiteral("kdesvninfo")), this, SLOT(slotInfo()));
    tmp_action->setStatusTip(i18n("Show details about selected item"));
    tmp_action = add_action(QStringLiteral("make_svn_rename"), i18n("Move"), QKeySequence(Qt::Key_F2), QIcon::fromTheme(QStringLiteral("kdesvnmove")), this, SLOT(slotRename()));
    tmp_action->setStatusTip(i18n("Moves or renames current item"));
    tmp_action = add_action(QStringLiteral("make_svn_copy"), i18n("Copy"), QKeySequence(Qt::CTRL | Qt::Key_C), QIcon::fromTheme(QStringLiteral("kdesvncopy")), this, SLOT(slotCopy()));
    tmp_action->setStatusTip(i18n("Create a copy of current item"));
    tmp_action = add_action(QStringLiteral("make_check_updates"), i18n("Check for updates"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncheckupdates")), this, SLOT(slotCheckUpdates()));
    tmp_action->setToolTip(i18n("Check if current working copy has items with newer version in repository"));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action->setIconText(i18n("Check updates"));

    /* 2. actions only on files */
    tmp_action = add_action(QStringLiteral("make_svn_blame"), i18n("Blame"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnblame")), this, SLOT(slotBlame()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action(QStringLiteral("make_svn_range_blame"), i18n("Blame range"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnblame")), this, SLOT(slotRangeBlame()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs with revision and author information in-line."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action(QStringLiteral("make_svn_cat"), i18n("Cat head"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncat")), this, SLOT(slotCat()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs."));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action(QStringLiteral("make_revisions_cat"), i18n("Cat revision..."), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncat")), this, SLOT(slotRevisionCat()));
    tmp_action->setToolTip(i18n("Output the content of specified files or URLs at specific revision."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action(QStringLiteral("make_svn_lock"), i18n("Lock current items"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnlock")), this, SLOT(slotLock()));
    tmp_action->setToolTip(i18n("Try lock current item against changes from other users"));
    tmp_action->setStatusTip(tmp_action->toolTip());
    tmp_action = add_action(QStringLiteral("make_svn_unlock"), i18n("Unlock current items"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnunlock")), this, SLOT(slotUnlock()));
    tmp_action->setToolTip(i18n("Free existing lock on current item"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    /* 3. actions only on dirs */
    tmp_action = add_action(QStringLiteral("make_svn_mkdir"), i18n("New folder"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnnewfolder")), this, SLOT(slotMkdir()));
    tmp_action->setStatusTip(i18n("Create a new folder"));
    tmp_action = add_action(QStringLiteral("make_svn_switch"), i18n("Switch repository"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnswitch")), m_Data->m_Model->svnWrapper(), SLOT(slotSwitch()));
    tmp_action->setToolTip(i18n("Switch repository path of current working copy path (\"svn switch\")"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action(QStringLiteral("make_svn_relocate"), i18n("Relocate current working copy URL"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnrelocate")), this, SLOT(slotRelocate()));
    tmp_action->setToolTip(i18n("Relocate URL of current working copy path to other URL"));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action(QStringLiteral("make_check_unversioned"), i18n("Check for unversioned items"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnaddrecursive")), this, SLOT(slotCheckNewItems()));
    tmp_action->setIconText(i18n("Unversioned"));
    tmp_action->setToolTip(i18n("Browse folder for unversioned items and add them if wanted."));
    tmp_action->setStatusTip(tmp_action->toolTip());

    tmp_action = add_action(QStringLiteral("make_switch_to_repo"), i18n("Open repository of working copy"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnrepository")),
                            this, SLOT(slotChangeToRepository()));
    tmp_action->setToolTip(i18n("Opens the repository the current working copy was checked out from"));

    tmp_action = add_action(QStringLiteral("make_cleanup"), i18n("Cleanup"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncleanup")), this, SLOT(slotCleanupAction()));
    tmp_action->setToolTip(i18n("Recursively clean up the working copy, removing locks, resuming unfinished operations, etc."));
    tmp_action = add_action(QStringLiteral("make_import_dirs_into_current"), i18n("Import folders into current"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnimportfolder")),
                            this, SLOT(slotImportDirsIntoCurrent()));
    tmp_action->setToolTip(i18n("Import folder content into current URL"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    tmp_action = add_action(QStringLiteral("make_svn_add"), i18n("Add selected files/dirs"), QKeySequence(Qt::Key_Insert), QIcon::fromTheme(QStringLiteral("kdesvnadd")), m_Data->m_Model->svnWrapper(), SLOT(slotAdd()));
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository"));
    tmp_action->setIconText(i18n("Add"));
    tmp_action = add_action(QStringLiteral("make_svn_addrec"), i18n("Add selected files/dirs recursive"), QKeySequence(Qt::CTRL | Qt::Key_Insert), QIcon::fromTheme(QStringLiteral("kdesvnaddrecursive")),
                            m_Data->m_Model->svnWrapper(), SLOT(slotAddRec()));
    tmp_action->setToolTip(i18n("Adding selected files and/or directories to repository and all subitems of folders"));

    tmp_action = add_action(QStringLiteral("make_svn_remove"), i18n("Delete selected files/dirs"), QKeySequence(Qt::Key_Delete), QIcon::fromTheme(QStringLiteral("kdesvndelete")), this, SLOT(slotDelete()));
    tmp_action->setIconText(i18n("Delete"));
    tmp_action->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    tmp_action = add_action(QStringLiteral("make_svn_remove_left"), i18n("Delete folder"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndelete")), this, SLOT(slotLeftDelete()));
    tmp_action->setToolTip(i18n("Deleting selected directories from repository"));
    tmp_action->setIconText(i18n("Delete"));
    tmp_action  = add_action(QStringLiteral("make_svn_revert"), i18n("Revert current changes"), QKeySequence(Qt::CTRL | Qt::Key_R), QIcon::fromTheme(QStringLiteral("kdesvnreverse")), m_Data->m_Model->svnWrapper(), SLOT(slotRevert()));

    tmp_action = add_action(QStringLiteral("make_resolved"), i18n("Mark resolved"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnresolved")), this, SLOT(slotResolved()));
    tmp_action->setToolTip(i18n("Marking files or dirs resolved"));

    tmp_action = add_action(QStringLiteral("make_try_resolve"), i18n("Resolve conflicts"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnresolved")), this, SLOT(slotTryResolve()));

    tmp_action = add_action(QStringLiteral("make_svn_ignore"), i18n("Ignore/Unignore current item"), QKeySequence(), QIcon(), this, SLOT(slotIgnore()));
    tmp_action = add_action(QStringLiteral("make_left_add_ignore_pattern"), i18n("Add or Remove ignore pattern"), QKeySequence(), QIcon(), this, SLOT(slotLeftRecAddIgnore()));
    tmp_action = add_action(QStringLiteral("make_right_add_ignore_pattern"), i18n("Add or Remove ignore pattern"), QKeySequence(), QIcon(), this, SLOT(slotRightRecAddIgnore()));

    tmp_action = add_action(QStringLiteral("make_svn_headupdate"), i18n("Update to head"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnupdate")), m_Data->m_Model->svnWrapper(), SLOT(slotUpdateHeadRec()));
    tmp_action->setIconText(i18nc("Menu item", "Update"));
    tmp_action = add_action(QStringLiteral("make_svn_revupdate"), i18n("Update to revision..."), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnupdate")), m_Data->m_Model->svnWrapper(), SLOT(slotUpdateTo()));
    tmp_action = add_action(QStringLiteral("make_svn_commit"), i18n("Commit"), QKeySequence(QStringLiteral("CTRL+#")), QIcon::fromTheme(QStringLiteral("kdesvncommit")), this, SLOT(slotCommit()));
    tmp_action->setIconText(i18n("Commit"));

    tmp_action = add_action(QStringLiteral("make_svn_basediff"), i18n("Diff local changes"), QKeySequence(Qt::CTRL | Qt::Key_D), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotSimpleBaseDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against BASE (last checked out version) - does not require access to repository"));
    tmp_action = add_action(QStringLiteral("make_svn_dirbasediff"), i18n("Diff local changes"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotDirSimpleBaseDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against BASE (last checked out version) - does not require access to repository"));

    tmp_action =
        add_action(QStringLiteral("make_svn_headdiff"), i18n("Diff against HEAD"), QKeySequence(Qt::CTRL | Qt::Key_H), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotSimpleHeadDiff()));
    tmp_action->setToolTip(i18n("Diff working copy against HEAD (last checked in version)- requires access to repository"));

    tmp_action =
        add_action(QStringLiteral("make_svn_itemsdiff"), i18n("Diff items"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotDiffPathes()));
    tmp_action->setToolTip(i18n("Diff two items"));
    tmp_action =
        add_action(QStringLiteral("make_svn_diritemsdiff"), i18n("Diff items"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotDiffPathes()));
    tmp_action->setToolTip(i18n("Diff two items"));


    tmp_action =
        add_action(QStringLiteral("make_svn_merge_revisions"), i18n("Merge two revisions"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnmerge")), this, SLOT(slotMergeRevisions()));
    tmp_action->setIconText(i18n("Merge"));
    tmp_action->setToolTip(i18n("Merge two revisions of this entry into itself"));

    tmp_action =
        add_action(QStringLiteral("make_svn_merge"), i18n("Merge..."), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnmerge")), this, SLOT(slotMerge()));
    tmp_action->setToolTip(i18n("Merge repository path into current working copy path or current repository path into a target"));
    tmp_action = add_action(QStringLiteral("openwith"), i18n("Open With..."), QKeySequence(), QIcon(), this, SLOT(slotOpenWith()));

    /* remote actions only */
    tmp_action =
        add_action(QStringLiteral("make_svn_checkout_current"), i18n("Checkout current repository path"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncheckout")), m_Data->m_Model->svnWrapper(), SLOT(slotCheckoutCurrent()));
    tmp_action->setIconText(i18n("Checkout"));
    tmp_action =
        add_action(QStringLiteral("make_svn_export_current"), i18n("Export current repository path"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnexport")), m_Data->m_Model->svnWrapper(), SLOT(slotExportCurrent()));
    add_action(QStringLiteral("switch_browse_revision"), i18n("Select browse revision"), QKeySequence(), QIcon(), this, SLOT(slotSelectBrowsingRevision()));

    /* independe actions */
    tmp_action =
        add_action(QStringLiteral("make_svn_checkout"), i18n("Checkout a repository"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncheckout")), m_Data->m_Model->svnWrapper(), SLOT(slotCheckout()));
    tmp_action->setIconText(i18n("Checkout"));
    tmp_action = add_action(QStringLiteral("make_svn_export"), i18n("Export a repository"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnexport")), m_Data->m_Model->svnWrapper(), SLOT(slotExport()));
    tmp_action->setIconText(i18n("Export"));
    tmp_action = add_action(QStringLiteral("make_view_refresh"), i18n("Refresh view"), QKeySequence(Qt::Key_F5), QIcon::fromTheme(QStringLiteral("kdesvnrightreload")), this, SLOT(refreshCurrentTree()));
    tmp_action->setIconText(i18n("Refresh"));

    add_action(QStringLiteral("make_revisions_diff"), i18n("Diff revisions"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvndiff")), this, SLOT(slotDiffRevisions()));

    /* folding options */
    tmp_action = add_action(QStringLiteral("view_unfold_tree"), i18n("Unfold File Tree"), QKeySequence(), QIcon(), this, SLOT(slotUnfoldTree()));
    tmp_action->setToolTip(i18n("Opens all branches of the file tree"));
    tmp_action = add_action(QStringLiteral("view_fold_tree"), i18n("Fold File Tree"), QKeySequence(), QIcon(), this , SLOT(slotFoldTree()));
    tmp_action->setToolTip(i18n("Closes all branches of the file tree"));

    /* caching */
    tmp_action = add_action(QStringLiteral("update_log_cache"), i18n("Update log cache"), QKeySequence(), QIcon(), this, SLOT(slotUpdateLogCache()));
    tmp_action->setToolTip(i18n("Update the log cache for current repository"));

    tmp_action = add_action(QStringLiteral("make_dir_commit"), i18n("Commit"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvncommit")), this, SLOT(slotDirCommit()));
    tmp_action = add_action(QStringLiteral("make_dir_update"), i18n("Update to head"), QKeySequence(), QIcon::fromTheme(QStringLiteral("kdesvnupdate")), this, SLOT(slotDirUpdate()));
    tmp_action = add_action(QStringLiteral("set_rec_property_dir"), i18n("Set property recursive"), QKeySequence(), QIcon(), this, SLOT(slotDirRecProperty()));

    tmp_action = add_action(QStringLiteral("show_repository_settings"), i18n("Settings for current repository"), QKeySequence(), QIcon(), this, SLOT(slotRepositorySettings()));

    enableActions();
}

bool MainTreeWidget::uniqueTypeSelected()
{
    QModelIndexList _mi = m_TreeView->selectionModel()->selectedRows(0);
    if (_mi.count() < 1) {
        return false;
    }
    bool dir = static_cast<SvnItemModelNode *>(m_Data->srcInd(_mi[0]).internalPointer())->isDir();
    for (int i = 1; i < _mi.count(); ++i) {
        if (static_cast<SvnItemModelNode *>(m_Data->srcInd(_mi[i]).internalPointer())->isDir() != dir) {
            return false;
        }
    }
    return true;
}

void MainTreeWidget::enableAction(const QString &name, bool how)
{
    QAction *temp = filesActions()->action(name);
    if (temp) {
        temp->setEnabled(how);
        temp->setVisible(how);
    }
}

void MainTreeWidget::enableActions()
{
    const bool isopen = !baseUri().isEmpty();
    const SvnItemList fileList = SelectionList();
    const SvnItemList dirList = DirSelectionList();
    const SvnItemModelNode *si = SelectedNode();

    const bool single = isopen && fileList.size() == 1;
    const bool multi = isopen && fileList.size() > 1;
    const bool none = isopen && fileList.isEmpty();
    const bool single_dir = single && si && si->isDir();
    const bool unique = uniqueTypeSelected();
    const bool remote_enabled =/*isopen&&*/m_Data->m_Model->svnWrapper()->doNetworking();
    const bool conflicted = single && si && si->isConflicted();

    bool at_least_one_changed = false;
    bool at_least_one_conflicted = false;
    bool at_least_one_local_added = false;
    bool all_unversioned = true;
    bool all_versioned = true;
    bool at_least_one_directory = false;
    for(int i = 0; i < fileList.size(); ++i) {
      const SvnItem *item = fileList.at(i);
      if (!item) {
          // root item
          continue;
      }
      if (item->isChanged()) {
          at_least_one_changed = true;
      }
      if (item->isConflicted()) {
          at_least_one_conflicted = true;
      }
      if (item->isLocalAdded()) {
          at_least_one_local_added = true;
      }
      if (item->isRealVersioned()) {
          all_unversioned = false;
      } else {
          all_versioned = false;
      }
      if (item->isDir()) {
          at_least_one_directory = true;
      }
    }

    //qDebug("single: %d, multi: %d, none: %d, single_dir: %d, unique: %d, remove_enabled: %d, conflicted: %d, changed: %d, added: %d",
    //       single, multi, none, single_dir, unique, remote_enabled, conflicted, si && si->isChanged(), si && si->isLocalAdded());
    //qDebug("at_least_one_changed: %d, at_least_one_conflicted: %d, at_least_one_local_added: %d, all_unversioned: %d, all_versioned: %d, at_least_one_directory: %d",
    //       at_least_one_changed, at_least_one_conflicted, at_least_one_local_added, all_unversioned, all_versioned, at_least_one_directory);

    /* local and remote actions */
    /* 1. actions on dirs AND files */
    enableAction(QStringLiteral("make_svn_log_nofollow"), single || none);
    enableAction(QStringLiteral("make_svn_dir_log_nofollow"), dirList.size() == 1 && isopen);
    enableAction(QStringLiteral("make_last_change"), isopen);
    enableAction(QStringLiteral("make_svn_log_full"), single || none);
    enableAction(QStringLiteral("make_svn_tree"), single || none);
    enableAction(QStringLiteral("make_svn_partialtree"), single || none);

    enableAction(QStringLiteral("make_svn_property"), single);
    enableAction(QStringLiteral("make_left_svn_property"), dirList.size() == 1);
    enableAction(QStringLiteral("set_rec_property_dir"), dirList.size() == 1);
    enableAction(QStringLiteral("get_svn_property"), single);
    enableAction(QStringLiteral("make_svn_remove"), (multi || single));
    enableAction(QStringLiteral("make_svn_remove_left"), dirList.size() > 0);
    enableAction(QStringLiteral("make_svn_lock"), (multi || single));
    enableAction(QStringLiteral("make_svn_unlock"), (multi || single));

    enableAction(QStringLiteral("make_svn_ignore"), (single) && si && si->parent() != nullptr && !si->isRealVersioned());
    enableAction(QStringLiteral("make_left_add_ignore_pattern"), (dirList.size() == 1) && isWorkingCopy());
    enableAction(QStringLiteral("make_right_add_ignore_pattern"), single_dir && isWorkingCopy());

    enableAction(QStringLiteral("make_svn_rename"), single && (!isWorkingCopy() || si != m_Data->m_Model->firstRootChild()));
    enableAction(QStringLiteral("make_svn_copy"), single && (!isWorkingCopy() || si != m_Data->m_Model->firstRootChild()));

    /* 2. only on files */
    enableAction(QStringLiteral("make_svn_blame"), single && !single_dir && remote_enabled);
    enableAction(QStringLiteral("make_svn_range_blame"), single && !single_dir && remote_enabled);
    enableAction(QStringLiteral("make_svn_cat"), single && !single_dir);

    /* 3. actions only on dirs */
    enableAction(QStringLiteral("make_svn_mkdir"), single_dir || (none && isopen));
    enableAction(QStringLiteral("make_svn_switch"), isWorkingCopy() && (single || none));
    enableAction(QStringLiteral("make_switch_to_repo"), isWorkingCopy());
    enableAction(QStringLiteral("make_import_dirs_into_current"), single_dir || dirList.size() == 1);
    enableAction(QStringLiteral("make_svn_relocate"), isWorkingCopy() && (single || none));

    enableAction(QStringLiteral("make_svn_export_current"), ((single && single_dir) || none));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    enableAction(QStringLiteral("make_svn_add"), (multi || single) && isWorkingCopy() && all_unversioned);
    enableAction(QStringLiteral("make_svn_revert"), (multi || single) && isWorkingCopy() && (at_least_one_changed || at_least_one_conflicted || at_least_one_local_added));
    enableAction(QStringLiteral("make_resolved"), (multi || single) && isWorkingCopy());
    enableAction(QStringLiteral("make_try_resolve"), conflicted && !single_dir);

    enableAction(QStringLiteral("make_svn_info"), isopen);
    enableAction(QStringLiteral("make_svn_merge_revisions"), (single || dirList.size() == 1) && isWorkingCopy());
    enableAction(QStringLiteral("make_svn_merge"), single || dirList.size() == 1 || none);
    enableAction(QStringLiteral("make_svn_addrec"), (multi || single) && at_least_one_directory && isWorkingCopy() && all_unversioned);
    enableAction(QStringLiteral("make_svn_headupdate"), isWorkingCopy() && isopen && remote_enabled);
    enableAction(QStringLiteral("make_dir_update"), isWorkingCopy() && isopen && remote_enabled);

    enableAction(QStringLiteral("make_svn_revupdate"), isWorkingCopy() && isopen && remote_enabled);
    enableAction(QStringLiteral("make_svn_commit"), isWorkingCopy() && isopen && remote_enabled);
    enableAction(QStringLiteral("make_dir_commit"), isWorkingCopy() && isopen && remote_enabled);

    enableAction(QStringLiteral("make_svn_basediff"), isWorkingCopy() && (single || none));
    enableAction(QStringLiteral("make_svn_dirbasediff"), isWorkingCopy() && (dirList.size() < 2));
    enableAction(QStringLiteral("make_svn_headdiff"), isWorkingCopy() && (single || none) && remote_enabled);

    /// @todo check if all items have same type
    enableAction(QStringLiteral("make_svn_itemsdiff"), multi && fileList.size() == 2 && unique && remote_enabled && all_versioned);
    enableAction(QStringLiteral("make_svn_diritemsdiff"), dirList.size() == 2 && isopen && remote_enabled && all_versioned);

    /* 2. on dirs only */
    enableAction(QStringLiteral("make_cleanup"), isWorkingCopy() && (single_dir || none));
    enableAction(QStringLiteral("make_check_unversioned"), isWorkingCopy() && ((single_dir && single) || none));

    /* remote actions only */
    enableAction(QStringLiteral("make_svn_checkout_current"), ((single && single_dir) || none) && !isWorkingCopy() && remote_enabled);
    /* independ actions */
    enableAction(QStringLiteral("make_svn_checkout"), remote_enabled);
    enableAction(QStringLiteral("make_svn_export"), true);
    enableAction(QStringLiteral("make_view_refresh"), isopen);

    enableAction(QStringLiteral("make_revisions_diff"), isopen);
    enableAction(QStringLiteral("make_revisions_cat"), isopen && !single_dir && single);
    enableAction(QStringLiteral("switch_browse_revision"), !isWorkingCopy() && isopen);
    enableAction(QStringLiteral("make_check_updates"), isWorkingCopy() && isopen && remote_enabled);
    enableAction(QStringLiteral("openwith"), KAuthorized::authorizeAction("openwith") && single && !single_dir);
    enableAction(QStringLiteral("show_repository_settings"), isopen);

    enableAction(QStringLiteral("repo_statistic"), isopen);

    QAction *temp = filesActions()->action(QStringLiteral("update_log_cache"));
    if (temp) {
        temp->setEnabled(remote_enabled);
        if (!m_Data->m_Model->svnWrapper()->threadRunning(SvnActions::fillcachethread)) {
            temp->setText(i18n("Update log cache"));
        } else {
            temp->setText(i18n("Stop updating the log cache"));
        }
    }
}

QAction *MainTreeWidget::add_action(const QString &actionname,
                                    const QString &text,
                                    const QKeySequence &sequ,
                                    const QIcon &icon,
                                    QObject *target,
                                    const char *slot)
{
    QAction *tmp_action = nullptr;
    tmp_action = m_Data->m_Collection->addAction(actionname, target, slot);
    tmp_action->setText(text);
    m_Data->m_Collection->setDefaultShortcut(tmp_action, sequ);
    tmp_action->setIcon(icon);
    return tmp_action;
}

KActionCollection *MainTreeWidget::filesActions()
{
    return m_Data->m_Collection;
}

void MainTreeWidget::closeMe()
{
    m_Data->m_Model->svnWrapper()->killallThreads();

    clear();
    setWorkingCopy(true);
    setNetworked(false);
    setWorkingCopy(false);
    setBaseUri(QString());

    emit changeCaption(QString());
    emit sigUrlOpend(false);
    emit sigUrlChanged(QUrl());

    enableActions();
    m_Data->m_Model->svnWrapper()->reInitClient();
}

void MainTreeWidget::refreshCurrentTree()
{
    m_Data->m_Model->refreshCurrentTree();
    if (isWorkingCopy()) {
        m_Data->m_Model->svnWrapper()->createModifiedCache(baseUri());
    }
    m_Data->m_SortModel->invalidate();
    setUpdatesEnabled(true);
    //viewport()->repaint();
    QTimer::singleShot(1, this, SLOT(readSupportData()));
}

void MainTreeWidget::slotSettingsChanged()
{
    m_Data->m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort() ? Qt::CaseSensitive : Qt::CaseInsensitive);
    m_Data->m_SortModel->invalidate();
    m_Data->m_DirSortModel->invalidate();
    enableActions();
    if (m_Data->m_Model->svnWrapper() && !m_Data->m_Model->svnWrapper()->doNetworking()) {
        m_Data->m_Model->svnWrapper()->stopFillCache();
    }
    checkUseNavigation();
}

KService::List MainTreeWidget::offersList(SvnItem *item, bool execOnly) const
{
    KService::List offers;
    if (!item) {
        return offers;
    }
    if (!item->mimeType().isValid()) {
        return offers;
    }
    QString constraint(QLatin1String("(DesktopEntryName != 'kdesvn') and (Type == 'Application')"));
    if (execOnly) {
        constraint += QLatin1String(" and (exist Exec)");
    }
    offers = KMimeTypeTrader::self()->query(item->mimeType().name(), QString::fromLatin1("Application"), constraint);
    return offers;
}

void MainTreeWidget::slotItemActivated(const QModelIndex &_index)
{
    QModelIndex index = m_Data->m_SortModel->mapToSource(_index);
    itemActivated(index);
}

void MainTreeWidget::itemActivated(const QModelIndex &index, bool keypress)
{
    Q_UNUSED(keypress);
    SvnItemModelNode *item;
    if (index.isValid() && (item = static_cast<SvnItemModelNode *>(index.internalPointer()))) {
        if (!item->isDir()) {
            svn::Revision rev;
            QList<QUrl> lst;
            lst.append(item->kdeName(rev));
            KService::List li = offersList(item, true);
            if (li.isEmpty() || li.first()->exec().isEmpty()) {
                li = offersList(item);
            }
            if (!li.isEmpty() && !li.first()->exec().isEmpty()) {
                KService::Ptr ptr = li.first();
                KRun::runService(*ptr, lst, QApplication::activeWindow());
            } else {
                KRun::displayOpenWithDialog(lst, QApplication::activeWindow());
            }
        } else if (Kdesvnsettings::show_navigation_panel()) {
            m_DirTreeView->selectionModel()->select(m_Data->m_DirSortModel->mapFromSource(index), QItemSelectionModel::ClearAndSelect);
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

void MainTreeWidget::slotNotifyMessage(const QString &what)
{
    emit sigLogMessage(what);
    QCoreApplication::processEvents();
}

void MainTreeWidget::readSupportData()
{
    /// this moment empty cause no usagedata explicit used by MainTreeWidget
}

void MainTreeWidget::slotClientException(const QString &what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(QApplication::activeModalWidget(), what, i18n("SVN Error"));
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
    SvnItem *item = DirSelected();
    if (!item || !item->isDir()) {
        return;
    }
    recAddIgnore(item);
}

void MainTreeWidget::slotRightRecAddIgnore()
{
    SvnItem *item = Selected();
    if (!item || !item->isDir()) {
        return;
    }
    recAddIgnore(item);
}

void MainTreeWidget::recAddIgnore(SvnItem *item)
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("ignore_pattern_dlg")));
    dlg->setWindowTitle(i18nc("@title:window", "Edit Pattern to Ignore for \"%1\"", item->shortName()));
    dlg->setWithCancelButton();
    EditIgnorePattern *ptr(new EditIgnorePattern(dlg));
    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::Depth _d = ptr->depth();
    QStringList _pattern = ptr->items();
    bool unignore = ptr->unignore();
    svn::Revision start(svn::Revision::WORKING);
    if (!isWorkingCopy()) {
        start = baseRevision();
    }

    svn::StatusEntries res;
    if (!m_Data->m_Model->svnWrapper()->makeStatus(item->fullName(), res, start, _d, true /* all entries */, false, false)) {
        return;
    }
    for (int i = 0; i < res.count(); ++i) {
        if (!res[i]->isRealVersioned() || res[i]->entry().kind() != svn_node_dir) {
            continue;
        }
        m_Data->m_Model->svnWrapper()->makeIgnoreEntry(res[i]->path(), _pattern, unignore);
    }
    refreshCurrentTree();
    delete dlg;
}

void MainTreeWidget::slotMakeLogNoFollow()const
{
    doLog(false, false);
}

void MainTreeWidget::slotMakeLog()const
{
    doLog(true, false);
}

void MainTreeWidget::slotDirMakeLogNoFollow()const
{
    doLog(false, true);
}

void MainTreeWidget::doLog(bool use_follow_settings, bool left)const
{
    SvnItem *k = left ? DirSelectedOrMain() : SelectedOrMain();
    QString what;
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && selectionCount() == 0) {
        what = baseUri();
    } else {
        return;
    }
    svn::Revision start(svn::Revision::HEAD);
    if (!isWorkingCopy()) {
        start = baseRevision();
    }
    svn::Revision end(svn::Revision::START);
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    bool follow = use_follow_settings ? Kdesvnsettings::log_follows_nodes() : false;
    Kdesvnsettings::setLast_node_follow(follow);
    int l = 50;
    m_Data->m_Model->svnWrapper()->makeLog(start, end, (isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision()), what, follow, list, l);
}


void MainTreeWidget::slotContextMenu(const QPoint &)
{
    execContextMenu(SelectionList());
}

void MainTreeWidget::slotDirContextMenu(const QPoint &vp)
{
    QMenu popup;
    QAction *temp = nullptr;
    int count = 0;
    if ((temp = filesActions()->action(QStringLiteral("make_dir_commit"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_dir_update"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_svn_dirbasediff"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_svn_diritemsdiff"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_svn_dir_log_nofollow"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_left_svn_property"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_svn_remove_left"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("make_left_add_ignore_pattern"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }
    if ((temp = filesActions()->action(QStringLiteral("set_rec_property_dir"))) && temp->isEnabled() && ++count) {
        popup.addAction(temp);
    }

    OpenContextmenu *me = nullptr;
    QAction *menuAction = nullptr;
    const SvnItemList l = DirSelectionList();
    if (l.count() == 1 && l.at(0)) {
        const KService::List offers = offersList(l.at(0), l.at(0)->isDir());
        if (!offers.isEmpty()) {
            svn::Revision rev(isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision());
            me = new OpenContextmenu(l.at(0)->kdeName(rev), offers, nullptr);
            me->setTitle(i18n("Open With..."));
            menuAction = popup.addMenu(me);
            ++count;
        }
    }
    if (count) {
        popup.exec(m_DirTreeView->viewport()->mapToGlobal(vp));
    }
    if (menuAction) {
        popup.removeAction(menuAction);
        delete menuAction;
    }
    delete me;

}

void MainTreeWidget::execContextMenu(const SvnItemList &l)
{
    bool isopen = baseUri().length() > 0;
    QString menuname;

    if (!isopen) {
        menuname = "empty";
    } else if (isWorkingCopy()) {
        menuname = "local";
    } else {
        menuname = "remote";
    }
    if (l.isEmpty()) {
        menuname += "_general";
    } else if (l.count() > 1) {
        menuname += "_context_multi";
    } else {
        menuname += "_context_single";
        if (isWorkingCopy()) {
            if (l.at(0)->isRealVersioned()) {
                if (l.at(0)->isConflicted()) {
                    menuname += "_conflicted";
                } else {
                    menuname += "_versioned";
                    if (l.at(0)->isDir()) {
                        menuname += "_dir";
                    }
                }
            } else {
                menuname += "_unversioned";
            }
        } else if (l.at(0)->isDir()) {
            menuname += "_dir";
        }
    }

    //qDebug("menuname: %s", qPrintable(menuname));
    QWidget *target;
    emit sigShowPopup(menuname, &target);
    QMenu *popup = static_cast<QMenu *>(target);
    if (!popup) {
        return;
    }

    OpenContextmenu *me = nullptr;
    QAction *temp = nullptr;
    QAction *menuAction = nullptr;
    if (l.count() == 1/*&&!l.at(0)->isDir()*/) {
        KService::List offers = offersList(l.at(0), l.at(0)->isDir());
        if (!offers.isEmpty()) {
            svn::Revision rev(isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision());
            me = new OpenContextmenu(l.at(0)->kdeName(rev), offers, nullptr);
            me->setTitle(i18n("Open With..."));
            menuAction = popup->addMenu(me);
        } else {
            temp = filesActions()->action(QStringLiteral("openwith"));
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
        delete temp;
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
    SvnItem *which = Selected();
    if (!which || which->isDir()) {
        return;
    }
    svn::Revision rev(isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision());
    QList<QUrl> lst;
    lst.append(which->kdeName(rev));
    KRun::displayOpenWithDialog(lst, QApplication::activeWindow());
}

void MainTreeWidget::slotSelectBrowsingRevision()
{
    if (isWorkingCopy()) {
        return;
    }
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range, false)) {
        m_Data->m_remoteRevision = range.first;
        clear();
        m_Data->m_Model->checkDirs(baseUri(), nullptr);
        emit changeCaption(baseUri() + QLatin1Char('@') + range.first.toString());
    }
}

void MainTreeWidget::slotMakeTree()
{
    QString what;
    SvnItem *k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && selectionCount() == 0) {
        what = baseUri();
    } else {
        return;
    }
    svn::Revision rev(isWorkingCopy() ? svn::Revision::WORKING : baseRevision());

    m_Data->m_Model->svnWrapper()->makeTree(what, rev);
}

void MainTreeWidget::slotMakePartTree()
{
    QString what;
    SvnItem *k = SelectedOrMain();
    if (k) {
        what = k->fullName();
    } else if (!isWorkingCopy() && selectionCount() == 0) {
        what = baseUri();
    } else {
        return;
    }
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range)) {
        svn::Revision rev(isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision());
        m_Data->m_Model->svnWrapper()->makeTree(what, rev, range.first, range.second);
    }
}

void MainTreeWidget::slotLock()
{
    const SvnItemList lst = SelectionList();
    if (lst.isEmpty()) {
        KMessageBox::error(this, i18n("Nothing selected for unlock"));
        return;
    }
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("locking_log_msg")));
    dlg->setWindowTitle(i18nc("@title:window", "Lock Message"));
    dlg->setWithCancelButton();
    Commitmsg_impl *ptr(new Commitmsg_impl(dlg));
    ptr->initHistory();
    ptr->hideDepth(true);
    ptr->keepsLocks(false);

    QCheckBox *_stealLock = new QCheckBox(i18n("Steal lock?"));
    ptr->addItemWidget(_stealLock);

    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        if (dlg)
            ptr->saveHistory(true);
        delete dlg;
        return;
    }

    QString logMessage = ptr->getMessage();
    bool steal = _stealLock->isChecked();
    ptr->saveHistory(false);

    QStringList displist;
    for (int i = 0; i < lst.count(); ++i) {
        displist.append(lst[i]->fullName());
    }
    m_Data->m_Model->svnWrapper()->makeLock(displist, logMessage, steal);
    refreshCurrentTree();

    delete dlg;
}


/*!
    \fn MainTreeWidget::slotUnlock()
 */
void MainTreeWidget::slotUnlock()
{
    const SvnItemList lst = SelectionList();
    if (lst.isEmpty()) {
        KMessageBox::error(this, i18n("Nothing selected for unlock"));
        return;
    }
    KMessageBox::ButtonCode res = KMessageBox::questionYesNoCancel(this,
                                                                   i18n("Break lock or ignore missing locks?"),
                                                                   i18n("Unlocking items"));
    if (res == KMessageBox::Cancel) {
        return;
    }
    bool breakit = res == KMessageBox::Yes;

    QStringList displist;
    for (int i = 0; i < lst.count(); ++i) {
        displist.append(lst[i]->fullName());
    }
    m_Data->m_Model->svnWrapper()->makeUnlock(displist, breakit);
    refreshCurrentTree();
}

void MainTreeWidget::slotDisplayLastDiff()
{
    SvnItem *kitem = Selected();
    QString what;
    if (isWorkingCopy()) {
        QDir::setCurrent(baseUri());
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
            what = baseUri();
        }
    } else {
        what = relativePath(kitem);
    }
    svn::Revision start;
    svn::InfoEntry inf;
    if (!kitem) {
        // it has to have an item when in working copy, so we know we are in repository view.
        if (!m_Data->m_Model->svnWrapper()->singleInfo(what, baseRevision(), inf)) {
            return;
        }
        start = inf.cmtRev();
    } else {
        start = kitem->cmtRev();
    }
    if (!isWorkingCopy()) {
        if (!m_Data->m_Model->svnWrapper()->singleInfo(what, start.revnum() - 1, inf)) {
            return;
        }
        end = inf.cmtRev();
    }
    m_Data->m_Model->svnWrapper()->makeDiff(what, end, what, start, realWidget());
}

void MainTreeWidget::slotSimpleBaseDiff()
{
    simpleWcDiff(Selected(), svn::Revision::BASE, svn::Revision::WORKING);
}

void MainTreeWidget::slotDirSimpleBaseDiff()
{
    simpleWcDiff(DirSelected(), svn::Revision::BASE, svn::Revision::WORKING);
}

void MainTreeWidget::slotSimpleHeadDiff()
{
    simpleWcDiff(Selected(), svn::Revision::WORKING, svn::Revision::HEAD);
}

void MainTreeWidget::simpleWcDiff(SvnItem *kitem, const svn::Revision &first, const svn::Revision &second)
{
    QString what;
    if (isWorkingCopy()) {
        QDir::setCurrent(baseUri());
    }

    if (!kitem) {
        what = QLatin1Char('.');
    } else {
        what = relativePath(kitem);
    }
    // only possible on working copies - so we may say this values
    m_Data->m_Model->svnWrapper()->makeDiff(what, first, second, svn::Revision::UNDEFINED, kitem ? kitem->isDir() : true);
}

void MainTreeWidget::slotDiffRevisions()
{
    SvnItem *k = Selected();
    QString what;
    if (isWorkingCopy()) {
        QDir::setCurrent(baseUri());
    }

    if (!k) {
        what = (isWorkingCopy() ? "." : baseUri());
    } else {
        what = relativePath(k);
    }
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range)) {
        svn::Revision _peg = (isWorkingCopy() ? svn::Revision::WORKING : baseRevision());
        m_Data->m_Model->svnWrapper()->makeDiff(what, range.first, range.second, _peg, k ? k->isDir() : true);
    }
}

void MainTreeWidget::slotDiffPathes()
{
    SvnItemList lst;

    QObject *tr = sender();
    bool unique = false;

    if (tr == filesActions()->action(QStringLiteral("make_svn_diritemsdiff"))) {
        unique = true;
        lst = DirSelectionList();
    } else {
        lst = SelectionList();
    }

    if (lst.count() != 2 || (!unique && !uniqueTypeSelected())) {
        return;
    }

    SvnItem *k1 = lst.at(0);
    SvnItem *k2 = lst.at(1);
    QString w1, w2;
    svn::Revision r1;

    if (isWorkingCopy()) {
        QDir::setCurrent(baseUri());
        w1 = relativePath(k1);
        w2 = relativePath(k2);
        r1 = svn::Revision::WORKING;
    } else {
        w1 = k1->fullName();
        w2 = k2->fullName();
        r1 = baseRevision();
    }
    m_Data->m_Model->svnWrapper()->makeDiff(w1, r1, w2, r1);
}

void MainTreeWidget::slotInfo()
{
    svn::Revision rev(isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision());
    if (!isWorkingCopy()) {
        rev = baseRevision();
    }
    SvnItemList lst = SelectionList();
    if (lst.isEmpty()) {
        if (!isWorkingCopy()) {
            QStringList _sl(baseUri());
            m_Data->m_Model->svnWrapper()->makeInfo(_sl, rev, svn::Revision::UNDEFINED, Kdesvnsettings::info_recursive());
        } else {
            lst.append(SelectedOrMain());
        }
    }
    if (!lst.isEmpty()) {
        m_Data->m_Model->svnWrapper()->makeInfo(lst, rev, rev, Kdesvnsettings::info_recursive());
    }
}

void MainTreeWidget::slotBlame()
{
    SvnItem *k = Selected();
    if (!k) {
        return;
    }
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    m_Data->m_Model->svnWrapper()->makeBlame(start, end, k);
}

void MainTreeWidget::slotRangeBlame()
{
    SvnItem *k = Selected();
    if (!k) {
        return;
    }
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range)) {
        m_Data->m_Model->svnWrapper()->makeBlame(range.first, range.second, k);
    }
}

void MainTreeWidget::_propListTimeout()
{
    dispProperties(false);
}

void MainTreeWidget::slotDisplayProperties()
{
    dispProperties(true);
}

void MainTreeWidget::refreshItem(SvnItemModelNode *node)
{
    if (node) {
        m_Data->m_Model->refreshItem(node);
    }
}

void MainTreeWidget::slotChangeProperties(const svn::PropertiesMap &pm, const QStringList &dellist, const QString &path)
{
    m_Data->m_Model->svnWrapper()->changeProperties(pm, dellist, path);
    SvnItemModelNode *which = SelectedNode();
    if (which && which->fullName() == path) {
        m_Data->m_Model->refreshItem(which);
        dispProperties(true);
    }
}

void MainTreeWidget::dispProperties(bool force)
{
    CursorStack a(Qt::BusyCursor);
    bool cache_Only = (!force && isNetworked() && !Kdesvnsettings::properties_on_remote_items());
    svn::PathPropertiesMapListPtr pm;
    SvnItem *k = Selected();
    if (!k || !k->isRealVersioned()) {
        emit sigProplist(svn::PathPropertiesMapListPtr(), false, false, QString(""));
        return;
    }
    svn::Revision rev(isWorkingCopy() ? svn::Revision::WORKING : baseRevision());
    pm = m_Data->m_Model->svnWrapper()->propList(k->fullName(), rev, cache_Only);
    emit sigProplist(pm, isWorkingCopy(), k->isDir(), k->fullName());
}

void MainTreeWidget::slotCat()
{
    SvnItem *k = Selected();
    if (!k) {
        return;
    }
    m_Data->m_Model->svnWrapper()->slotMakeCat(isWorkingCopy() ? svn::Revision::HEAD : baseRevision(), k->fullName(), k->shortName(),
                                               isWorkingCopy() ? svn::Revision::HEAD : baseRevision(), nullptr);
}

void MainTreeWidget::slotRevisionCat()
{
    SvnItem *k = Selected();
    if (!k) {
        return;
    }
    Rangeinput_impl::revision_range range;
    if (Rangeinput_impl::getRevisionRange(range, true, true)) {
        m_Data->m_Model->svnWrapper()->slotMakeCat(range.first, k->fullName(), k->shortName(), isWorkingCopy() ? svn::Revision::WORKING : baseRevision(), nullptr);
    }
}

void MainTreeWidget::slotResolved()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItem *which = SelectedOrMain();
    if (!which) {
        return;
    }
    m_Data->m_Model->svnWrapper()->slotResolved(which->fullName());
    which->refreshStatus(true);
}

void MainTreeWidget::slotTryResolve()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItem *which = Selected();
    if (!which || which->isDir()) {
        return;
    }
    m_Data->m_Model->svnWrapper()->slotResolve(which->fullName());
}

void MainTreeWidget::slotLeftDelete()
{
    makeDelete(DirSelectionList());
}

void MainTreeWidget::slotDelete()
{
    makeDelete(SelectionList());
}

void MainTreeWidget::makeDelete(const SvnItemList &lst)
{
    if (lst.isEmpty()) {
        KMessageBox::error(this, i18n("Nothing selected for delete"));
        return;
    }
    svn::Paths items;
    QStringList displist;
    QList<QUrl> kioList;
    SvnItemList::const_iterator liter;
    for (liter = lst.begin(); liter != lst.end(); ++liter) {
        if (!(*liter)->isRealVersioned()) {
            QUrl _uri(QUrl::fromLocalFile((*liter)->fullName()));
            kioList.append(_uri);
        } else {
            items.push_back((*liter)->fullName());
        }
        displist.append((*liter)->fullName());
    }

    QPointer<DeleteForm> dlg(new DeleteForm(displist, QApplication::activeModalWidget()));
    dlg->showExtraButtons(isWorkingCopy() && !items.isEmpty());

    if (dlg->exec() == QDialog::Accepted) {
        bool force = dlg->force_delete();
        bool keep = dlg->keep_local();
        WidgetBlockStack st(this);
        if (!kioList.isEmpty()) {
            KIO::Job *aJob = KIO::del(kioList);
            if (!aJob->exec()) {
                KJobWidgets::setWindow(aJob, this);
                aJob->uiDelegate()->showErrorMessage();
                delete dlg;
                return;
            }
        }
        if (!items.isEmpty()) {
            m_Data->m_Model->svnWrapper()->makeDelete(svn::Targets(items), keep, force);
        }
        refreshCurrentTree();
    }
    delete dlg;
}

void MainTreeWidget::internalDrop(const QList<QUrl> &_lst, Qt::DropAction action, const QModelIndex &index)
{
    if (_lst.isEmpty()) {
        return;
    }
    QList<QUrl> lst = _lst;
    QString target;
    QString nProto;

    if (!isWorkingCopy()) {
        nProto = svn::Url::transformProtokoll(lst[0].scheme());
    }
    QList<QUrl>::iterator it = lst.begin();
    for (; it != lst.end(); ++it) {
        (*it).setQuery(QUrlQuery());
        if (!nProto.isEmpty())
            (*it).setScheme(nProto);
    }

    if (index.isValid()) {
        SvnItemModelNode *node = static_cast<SvnItemModelNode *>(index.internalPointer());
        target = node->fullName();
    } else {
        target = baseUri();
    }
    if (action == Qt::MoveAction) {
        m_Data->m_Model->svnWrapper()->makeMove(lst, target);
    } else if (action == Qt::CopyAction) {
        m_Data->m_Model->svnWrapper()->makeCopy(lst, target, (isWorkingCopy() ? svn::Revision::UNDEFINED : baseRevision()));
    }
    refreshCurrentTree();
}

void MainTreeWidget::slotUrlDropped(const QList<QUrl> &_lst, Qt::DropAction action, const QModelIndex &index, bool intern)
{
    if (_lst.isEmpty()) {
        return;
    }
    if (intern) {
        internalDrop(_lst, action, index);
        return;
    }
    QUrl target;
    if (index.isValid()) {
        SvnItemModelNode *node = static_cast<SvnItemModelNode *>(index.internalPointer());
        target = node->Url();
    } else {
        target = baseUriAsUrl();
    }

    if (baseUri().isEmpty()) {
        openUrl(_lst[0]);
        return;
    }
    QString path = _lst[0].path();
    QFileInfo fi(path);
    if (!isWorkingCopy()) {
        if (!fi.isDir()) {
            target.setPath(target.path() + QLatin1Char('/') + _lst[0].fileName());
        }
        slotImportIntoDir(_lst[0].toLocalFile(), target, fi.isDir());
    } else {
        WidgetBlockStack w(this);
        KIO::Job *job = KIO::copy(_lst, target);
        connect(job, SIGNAL(result(KJob*)), SLOT(slotCopyFinished(KJob*)));
        job->exec();
    }
}

void MainTreeWidget::slotCopyFinished(KJob *_job)
{
    KIO::CopyJob *job = dynamic_cast<KIO::CopyJob *>(_job);
    if (!job) {
        return;
    }
    bool ok = true;
    if (job->error()) {
        KJobWidgets::setWindow(job, this);
        job->uiDelegate()->showErrorMessage();
        ok = false;
    }
    if (ok) {
        const QList<QUrl> lst = job->srcUrls();
        const QString base = job->destUrl().toLocalFile() + QLatin1Char('/');
        svn::Paths tmp;
        tmp.reserve(lst.size());
        Q_FOREACH(const QUrl &url, lst) {
            tmp.push_back(svn::Path(base + url.fileName()));
        }
        m_Data->m_Model->svnWrapper()->addItems(tmp, svn::DepthInfinity);
    }
    refreshCurrentTree();
}

void MainTreeWidget::stopLogCache()
{
    QAction *temp = filesActions()->action(QStringLiteral("update_log_cache"));
    m_Data->m_Model->svnWrapper()->stopFillCache();
    if (temp) {
        temp->setText(i18n("Update log cache"));
    }
}

void MainTreeWidget::slotUpdateLogCache()
{
    if (baseUri().length() > 0 && m_Data->m_Model->svnWrapper()->doNetworking()) {
        QAction *temp = filesActions()->action(QStringLiteral("update_log_cache"));
        if (!m_Data->m_Model->svnWrapper()->threadRunning(SvnActions::fillcachethread)) {
            m_Data->m_Model->svnWrapper()->startFillCache(baseUri());
            if (temp) {
                temp->setText(i18n("Stop updating the log cache"));
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
    bool isopen = !baseUri().isEmpty();
    if (!isopen) {
        return;
    }
    QString parentDir = baseUri();
    svn::Paths targets;
    targets.append(svn::Path(parentDir + QLatin1String("/trunk")));
    targets.append(svn::Path(parentDir + QLatin1String("/branches")));
    targets.append(svn::Path(parentDir + QLatin1String("/tags")));
    QString msg = i18n("Automatic generated base layout by kdesvn");
    isopen = m_Data->m_Model->svnWrapper()->makeMkdir(svn::Targets(targets), msg);
    if (isopen) {
        refreshCurrentTree();
    }
}

void MainTreeWidget::slotMkdir()
{
    SvnItemModelNode *k = SelectedNode();
    QString parentDir;
    if (k) {
        if (!k->isDir()) {
            KMessageBox::sorry(nullptr, i18n("May not make subdirectories of a file"));
            return;
        }
        parentDir = k->fullName();
    } else {
        parentDir = baseUri();
    }
    QString ex = m_Data->m_Model->svnWrapper()->makeMkdir(parentDir);
    if (!ex.isEmpty()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir *>(k), true, true);
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
    if (isWorkingCopy() && SelectedNode() == m_Data->m_Model->firstRootChild()) {
        return;
    }
    bool ok;
    SvnItemModelNode *which = SelectedNode();
    if (!which) {
        return;
    }
    QString nName = CopyMoveView_impl::getMoveCopyTo(&ok, move, which->fullName(), baseUri(), this);
    if (!ok) {
        return;
    }
    if (move) {
        m_Data->m_Model->svnWrapper()->makeMove(which->fullName(), nName);
    } else {
        m_Data->m_Model->svnWrapper()->makeCopy(which->fullName(), nName, isWorkingCopy() ? svn::Revision::HEAD : baseRevision());
    }
}

void MainTreeWidget::slotCleanupAction()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItemModelNode *which = SelectedNode();
    if (!which) {
        which = m_Data->m_Model->firstRootChild();
    }
    if (!which || !which->isDir()) {
        return;
    }
    if (m_Data->m_Model->svnWrapper()->makeCleanup(which->fullName())) {
        which->refreshStatus(true);
    }
}

void MainTreeWidget::slotMergeRevisions()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItemModelNode *which = SelectedNode();
    if (!which) {
        return;
    }
    bool force, dry, rec, irelated, useExternal, allowmixedrevs;
    Rangeinput_impl::revision_range range;
    if (!MergeDlg_impl::getMergeRange(range, &force, &rec, &irelated, &dry, &useExternal, &allowmixedrevs, this)) {
        return;
    }
    if (!useExternal) {
        m_Data->m_Model->svnWrapper()->slotMergeWcRevisions(which->fullName(), range.first, range.second, rec, !irelated, force, dry, allowmixedrevs);
    } else {
        m_Data->m_Model->svnWrapper()->slotMergeExternal(which->fullName(), which->fullName(), which->fullName(),
                                                         range.first, range.second,
                                                         isWorkingCopy() ? svn::Revision::UNDEFINED : m_Data->m_remoteRevision,
                                                         rec);
    }
    refreshItem(which);
    if (which->isDir()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir *>(which), true, false);
    }
}

void MainTreeWidget::slotMerge()
{
    SvnItemModelNode *which = SelectedNode();
    QString src1, src2, target;
    if (isWorkingCopy()) {
        if (m_Data->merge_Target.isEmpty()) {
            target = which ? which->fullName() : baseUri();
        } else {
            target = m_Data->merge_Target;
        }
        src1 = m_Data->merge_Src1;
    } else {
        if (m_Data->merge_Src1.isEmpty()) {
            src1 = which ? which->fullName() : baseUri();
        } else {
            src1 = m_Data->merge_Src1;
        }
        target = m_Data->merge_Target;
    }
    src2 = m_Data->merge_Src2;
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("merge_dialog")));
    dlg->setWindowTitle(i18nc("@title:window", "Merge"));
    dlg->setWithCancelButton();
    dlg->setHelp(QLatin1String("merging-items"));
    MergeDlg_impl *ptr(new MergeDlg_impl(dlg));
    ptr->setDest(target);
    ptr->setSrc1(src1);
    ptr->setSrc2(src1);
    dlg->addWidget(ptr);
    if (dlg->exec() == QDialog::Accepted) {
        src1 = ptr->Src1();
        src2 = ptr->Src2();
        if (src2.isEmpty()) {
            src2 = src1;
        }
        target = ptr->Dest();
        m_Data->merge_Src2 = src2;
        m_Data->merge_Src1 = src1;
        m_Data->merge_Target = target;
        bool force = ptr->force();
        bool dry = ptr->dryrun();
        bool rec = ptr->recursive();
        bool irelated = ptr->ignorerelated();
        bool useExternal = ptr->useExtern();
        bool allowmixedrevs = ptr->allowmixedrevs();
        bool recordOnly = ptr->recordOnly();
        Rangeinput_impl::revision_range range = ptr->getRange();
        bool reintegrate = ptr->reintegrate();
        if (!useExternal) {
            m_Data->m_Model->svnWrapper()->slotMerge(src1, src2, target, range.first, range.second,
                                                     isWorkingCopy() ? svn::Revision::UNDEFINED : m_Data->m_remoteRevision,
                                                     rec, !irelated, force, dry, recordOnly, reintegrate, allowmixedrevs);
        } else {
            m_Data->m_Model->svnWrapper()->slotMergeExternal(src1, src2, target, range.first, range.second,
                                                             isWorkingCopy() ? svn::Revision::UNDEFINED : m_Data->m_remoteRevision,
                                                             rec);
        }
        if (isWorkingCopy()) {
            //            refreshItem(which);
            //            refreshRecursive(which);
            refreshCurrentTree();
        }
    }
    delete dlg;
    enableActions();
}

void MainTreeWidget::slotRelocate()
{
    if (!isWorkingCopy()) {
        return;
    }
    SvnItem *k = SelectedOrMain();
    if (!k) {
        KMessageBox::error(nullptr, i18n("Error getting entry to relocate"));
        return;
    }
    const QString path = k->fullName();
    const QUrl fromUrl = k->Url();
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("relocate_dlg")));
    dlg->setWindowTitle(i18nc("@title:window", "Relocate Path %1", path));
    dlg->setWithCancelButton();
    CheckoutInfo_impl *ptr(new CheckoutInfo_impl(dlg));
    ptr->setStartUrl(fromUrl);
    ptr->disableAppend(true);
    ptr->disableTargetDir(true);
    ptr->disableRange(true);
    ptr->disableOpen(true);
    ptr->hideDepth(true);
    ptr->hideOverwrite(true);
    dlg->addWidget(ptr);
    bool done = false;
    if (dlg->exec() == QDialog::Accepted) {
        if (!ptr->reposURL().isValid()) {
            KMessageBox::error(QApplication::activeModalWidget(), i18n("Invalid url given!"),
                               i18n("Relocate path %1", path));
            delete dlg;
            return;
        }
        done = m_Data->m_Model->svnWrapper()->makeRelocate(fromUrl, ptr->reposURL(), path, ptr->overwrite(), ptr->ignoreExternals());
    }
    delete dlg;
    if (done) {
        refreshItem(k->sItem());
    }
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
    if (selectionCount() > 1) {
        KMessageBox::error(this, i18n("Cannot import into multiple targets"));
        return;
    }
    QUrl targetDir;
    if (selectionCount() == 0) {
        if (isNetworked())
            targetDir = QUrl(baseUri());
        else
            targetDir = QUrl::fromLocalFile(baseUri());
    } else {
        targetDir = SelectedNode()->Url();
    }
    QString source;
    if (dirs) {
        source = QFileDialog::getExistingDirectory(this, i18n("Import files from folder"));
    } else {
        source = QFileDialog::getOpenFileName(this, i18n("Import file"), QString());
    }

    slotImportIntoDir(source, targetDir, dirs);
}

void MainTreeWidget::slotImportIntoDir(const QString &source, const QUrl &_targetUri, bool dirs)
{
    QString sourceUri = source;
    while (sourceUri.endsWith(QLatin1Char('/'))) {
        sourceUri.chop(1);
    }
    if (sourceUri.isEmpty()) {
        return;
    }

    if (_targetUri.isEmpty()) {
        return;
    }
    QUrl targetUri(_targetUri);

    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("import_log_msg")));
    dlg->setWindowTitle(i18nc("@title:window", "Import Log"));
    dlg->setWithCancelButton();
    Commitmsg_impl *ptr = nullptr;
    Importdir_logmsg *ptr2 = nullptr;
    if (dirs) {
        ptr2 = new Importdir_logmsg(dlg);
        ptr2->createDirboxDir(QLatin1Char('"') + QFileInfo(sourceUri).fileName() + QLatin1Char('"'));
        ptr = ptr2;
    } else {
        ptr = new Commitmsg_impl(dlg);
    }
    ptr->initHistory();
    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        if (dlg) {
            ptr->saveHistory(true);
            delete dlg;
        }
        return;
    }

    QString logMessage = ptr->getMessage();
    svn::Depth rec = ptr->getDepth();
    ptr->saveHistory(false);

    if (dirs && ptr2 && ptr2->createDir()) {
        targetUri.setPath(targetUri.path() + QLatin1Char('/') + QFileInfo(sourceUri).fileName());
    }
    if (ptr2) {
        m_Data->m_Model->svnWrapper()->slotImport(sourceUri, targetUri, logMessage, rec, ptr2->noIgnore(), ptr2->ignoreUnknownNodes());
    } else {
        m_Data->m_Model->svnWrapper()->slotImport(sourceUri, targetUri, logMessage, rec, false, false);
    }
    if (!isWorkingCopy()) {
        if (selectionCount() == 0) {
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
    SvnItemModelNode *k = m_Data->m_Model->firstRootChild();
    /* huh... */
    if (!k) {
        return;
    }
    svn::InfoEntry i;
    if (!m_Data->m_Model->svnWrapper()->singleInfo(k->Url().toString(), svn::Revision::UNDEFINED, i)) {
        return;
    }
    if (i.reposRoot().isEmpty()) {
        KMessageBox::sorry(QApplication::activeModalWidget(), i18n("Could not retrieve repository of working copy."), i18n("SVN Error"));
    } else {
        sigSwitchUrl(i.reposRoot());
    }
}

void MainTreeWidget::slotCheckNewItems()
{
    if (!isWorkingCopy()) {
        KMessageBox::sorry(nullptr, i18n("Only in working copy possible."), i18n("Error"));
        return;
    }
    if (selectionCount() > 1) {
        KMessageBox::sorry(nullptr, i18n("Only on single folder possible"), i18n("Error"));
        return;
    }
    SvnItem *w = SelectedOrMain();
    if (!w) {
        KMessageBox::sorry(nullptr, i18n("Sorry - internal error"), i18n("Error"));
        return;
    }
    m_Data->m_Model->svnWrapper()->checkAddItems(w->fullName(), true);
}

void MainTreeWidget::refreshCurrent(SvnItem *cur)
{
    if (!cur || !cur->sItem()) {
        refreshCurrentTree();
        return;
    }
    QCoreApplication::processEvents();
    setUpdatesEnabled(false);
    if (cur->isDir()) {
        m_Data->m_Model->refreshDirnode(static_cast<SvnItemModelNodeDir *>(cur->sItem()));
    } else {
        m_Data->m_Model->refreshItem(cur->sItem());
    }
    setUpdatesEnabled(true);
    m_TreeView->viewport()->repaint();
}

void MainTreeWidget::slotReinitItem(SvnItem *item)
{
    if (!item) {
        return;
    }
    SvnItemModelNode *k = item->sItem();
    if (!k) {
        return;
    }
    m_Data->m_Model->refreshItem(k);
    if (k->isDir()) {
        m_Data->m_Model->clearNodeDir(static_cast<SvnItemModelNodeDir *>(k));
    }
}

void MainTreeWidget::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !event->isAutoRepeat()) {
        QModelIndex index = SelectedIndex();
        if (index.isValid()) {
            itemActivated(index, true);
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

void MainTreeWidget::slotItemExpanded(const QModelIndex &)
{
}

void MainTreeWidget::slotItemsInserted(const QModelIndex &)
{
    m_Data->m_resizeColumnsTimer.start(50);
}

void MainTreeWidget::slotDirSelectionChanged(const QItemSelection &_item, const QItemSelection &)
{
    const QModelIndexList _indexes = _item.indexes();
    switch (DirselectionCount()) {
    case 1:
        m_DirTreeView->setStatusTip(i18n("Hold Ctrl key while click on selected item for unselect"));
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
    if (_indexes.size() >= 1) {
        const QModelIndex _t = m_Data->srcDirInd(_indexes.at(0));
        if (m_Data->m_Model->canFetchMore(_t)) {
            WidgetBlockStack st(m_TreeView);
            WidgetBlockStack st2(m_DirTreeView);
            m_Data->m_Model->fetchMore(_t);
        }
        if (Kdesvnsettings::show_navigation_panel()) {
            m_TreeView->setRootIndex(m_Data->m_SortModel->mapFromSource(_t));
        }
        // Display relative path (including name of the checkout) in the titlebar
        auto item = m_Data->m_Model->nodeForIndex(_t);
        if (item) {
            const QString repoBasePath = baseUri();
            const QString relativePath = item->fullName().mid(repoBasePath.lastIndexOf('/') + 1);
            changeCaption(relativePath);
        }

    } else {
        checkSyncTreeModel();
    }
    if (m_TreeView->selectionModel()->hasSelection()) {
        m_TreeView->selectionModel()->clearSelection();
    } else {
        enableActions();
    }
    resizeAllColumns();
}

void MainTreeWidget::checkSyncTreeModel()
{
    // make sure that the treeview shows the contents of the selected directory in the directory tree view
    // it can go out of sync when the dir tree model has no current index - then we use the first entry
    // or when the filter settings are changed
    QModelIndex curIdxDir = m_DirTreeView->currentIndex();
    if (!curIdxDir.isValid() && m_Data->m_DirSortModel->columnCount() > 0)
    {
        m_DirTreeView->setCurrentIndex(m_Data->m_DirSortModel->index(0, 0));
        curIdxDir = m_DirTreeView->currentIndex();
    }
    const QModelIndex curIdxBase = m_Data->srcDirInd(curIdxDir);
    m_TreeView->setRootIndex(m_Data->m_SortModel->mapFromSource(curIdxBase));
}

void MainTreeWidget::slotCommit()
{
    m_Data->m_Model->svnWrapper()->doCommit(SelectionList());
}

void MainTreeWidget::slotDirCommit()
{
    m_Data->m_Model->svnWrapper()->doCommit(DirSelectionList());
}

void MainTreeWidget::slotDirUpdate()
{
    const SvnItemList which = DirSelectionList();
    svn::Paths what;
    if (which.isEmpty()) {
        what.append(svn::Path(baseUri()));
    } else {
        what.reserve(which.size());
        Q_FOREACH(const SvnItem *item, which) {
            what.append(svn::Path(item->fullName()));
        }
    }
    m_Data->m_Model->svnWrapper()->makeUpdate(svn::Targets(what), svn::Revision::HEAD, svn::DepthUnknown);
}

void MainTreeWidget::slotRefreshItem(const QString &path)
{
    const QModelIndex idx = m_Data->m_Model->findIndex(path);
    if (!idx.isValid())
        return;
    m_Data->m_Model->emitDataChangedRow(idx);
}

void MainTreeWidget::checkUseNavigation(bool startup)
{
    bool use = Kdesvnsettings::show_navigation_panel();
    if (use)
    {
        checkSyncTreeModel();
    }
    else
    {
        // tree view is the only visible view, make sure to display all
        m_TreeView->setRootIndex(QModelIndex());
        m_TreeView->expand(QModelIndex());
    }
    m_TreeView->setExpandsOnDoubleClick(!use);
    m_TreeView->setRootIsDecorated(!use);
    m_TreeView->setItemsExpandable(!use);
    QList<int> si;
    if (use) {
        if (!startup) {
            si = m_ViewSplitter->sizes();
            if (si.size() == 2 && si[0] < 5) {
                si[0] = 200;
                m_ViewSplitter->setSizes(si);
            }
        }
    } else {
        si << 0 << 300;
        m_ViewSplitter->setSizes(si);

    }
}

void MainTreeWidget::slotRepositorySettings()
{
    if (baseUri().length() == 0) {
        return;
    }
    svn::InfoEntry inf;
    if (!m_Data->m_Model->svnWrapper()->singleInfo(baseUri(), baseRevision(), inf)) {
        return;
    }
    if (inf.reposRoot().isEmpty()) {
        KMessageBox::sorry(QApplication::activeModalWidget(), i18n("Could not retrieve repository."), i18n("SVN Error"));
    } else {
        DbSettings::showSettings(inf.reposRoot().toString(), this);
    }
}

void MainTreeWidget::slotRightProperties()
{
    SvnItem *k = Selected();
    if (!k) {
        return;
    }
    m_Data->m_Model->svnWrapper()->editProperties(k, isWorkingCopy() ? svn::Revision::WORKING : svn::Revision::HEAD);
}

void MainTreeWidget::slotLeftProperties()
{
    SvnItem *k = DirSelected();
    if (!k) {
        return;
    }
    m_Data->m_Model->svnWrapper()->editProperties(k, isWorkingCopy() ? svn::Revision::WORKING : svn::Revision::HEAD);
}

void MainTreeWidget::slotDirRecProperty()
{
    SvnItem *k = DirSelected();
    if (!k) {
        return;
    }
    KMessageBox::information(this, i18n("Not yet implemented"), i18n("Edit property recursively"));
}
