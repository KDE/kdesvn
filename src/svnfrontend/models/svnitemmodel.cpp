/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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

#include "svnitemmodel.h"
#include "getinfothread.h"
#include "helpers/kdesvn_debug.h"
#include "settings/kdesvnsettings.h"
#include "svnactions.h"
#include "svnfrontend/maintreewidget.h"
#include "svnitemnode.h"

#include "svnqt/client.h"
#include "svnqt/path.h"
#include "svnqt/status.h"
#include "svnqt/svnqt_defines.h"

#include <KDirWatch>
#include <KLocalizedString>
#include <KUrlMimeData>

#include <QBrush>
#include <QDir>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMimeData>
#include <QUuid>

/*****************************
 * Internal data class begin *
 *****************************/
class SvnItemModelData
{
    SvnItemModelData(const SvnItemModelData &);
    SvnItemModelData &operator=(const SvnItemModelData &);

public:
    SvnItemModelData(SvnItemModel *aCb, MainTreeWidget *display)
        : m_rootNode(nullptr)
        , m_SvnActions(nullptr)
        , m_Cb(aCb)
        , m_Display(display)
        , m_DirWatch(nullptr)
    {
        m_Uid = QUuid::createUuid().toString();
        m_InfoThread = new GetInfoThread(aCb);
    }

    ~SvnItemModelData()
    {
        m_InfoThread->cancelMe();
        if (!m_InfoThread->wait(500)) {
            m_InfoThread->terminate();
        }
        delete m_InfoThread;

        delete m_rootNode;
        delete m_DirWatch;
        m_rootNode = nullptr;
    }

    void clear()
    {
        delete m_rootNode;
        delete m_DirWatch;
        m_DirWatch = nullptr;
        m_rootNode = new SvnItemModelNodeDir(m_SvnActions, m_Display);
    }

    SvnItemModelNode *nodeForIndex(const QModelIndex &index) const
    {
        return index.isValid() ? static_cast<SvnItemModelNode *>(index.internalPointer()) : m_rootNode;
    }

    QModelIndex indexForNode(SvnItemModelNode *node, int rowNumber = -1) const
    {
        if (!node || node == m_rootNode) {
            return QModelIndex();
        }
        return m_Cb->createIndex(rowNumber == -1 ? node->rowNumber() : rowNumber, 0, node);
    }

    bool isRemoteAdded(const svn::Status &_Stat) const
    {
        return m_SvnActions->isUpdated(_Stat.path()) && _Stat.validReposStatus() && !_Stat.validLocalStatus();
    }

    bool MustCreateDir(const svn::Status &_Stat) const
    {
        // keep in sync with SvnItem::isDir()
        if (_Stat.entry().isValid() || isRemoteAdded(_Stat)) {
            if (_Stat.entry().kind() != svn_node_unknown) {
                return _Stat.entry().kind() == svn_node_dir;
            }
        }
        /* must be a local file */
        QFileInfo f(_Stat.path());
        return f.isDir();
    }

    void addWatchFile(const QString &aFile)
    {
        if (m_DirWatch) {
            m_DirWatch->addFile(aFile);
        }
    }
    void addWatchDir(const QString &aDir)
    {
        if (m_DirWatch) {
            m_DirWatch->addDir(aDir);
        }
    }

    SvnItemModelNodeDir *m_rootNode;

    SvnActions *m_SvnActions;
    SvnItemModel *m_Cb;
    MainTreeWidget *m_Display;
    KDirWatch *m_DirWatch;
    QString m_Uid;
    mutable GetInfoThread *m_InfoThread;
};
/*****************************
 * Internal data class end   *
 *****************************/

SvnItemModel::SvnItemModel(MainTreeWidget *display, QObject *parent)
    : QAbstractItemModel(parent)
    , m_Data(new SvnItemModelData(this, display))
{
    m_Data->m_SvnActions = new SvnActions(display);
    m_Data->m_rootNode = new SvnItemModelNodeDir(m_Data->m_SvnActions, display);
}

SvnItemModel::~SvnItemModel()
{
}

SvnItemModelNode *SvnItemModel::firstRootChild()
{
    if (!m_Data->m_rootNode) {
        return nullptr;
    }
    return m_Data->m_rootNode->child(0);
}

QModelIndex SvnItemModel::firstRootIndex()
{
    return m_Data->indexForNode(firstRootChild());
}

SvnItemModelNode *SvnItemModel::nodeForIndex(const QModelIndex &index)
{
    return m_Data->nodeForIndex(index);
}

void SvnItemModel::setRootNodeStat(const svn::StatusPtr &stat)
{
    m_Data->m_rootNode->setStat(stat);
}

void SvnItemModel::clear()
{
    int numRows = m_Data->m_rootNode->childList().count();
    if (numRows > 0)
        beginRemoveRows(QModelIndex(), 0, numRows - 1);
    m_Data->clear();
    if (numRows > 0)
        endRemoveRows();
}

void SvnItemModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    m_Data->m_InfoThread->clearNodes();
    m_Data->m_InfoThread->cancelMe();
    if (!m_Data->m_InfoThread->wait(1000)) { }
    QAbstractItemModel::beginRemoveRows(parent, first, last);
}

void SvnItemModel::clearNodeDir(SvnItemModelNodeDir *node)
{
    QModelIndex ind = m_Data->indexForNode(node);
    if (!node) {
        node = m_Data->m_rootNode;
    }
    int numRows = node->childList().size();
    beginRemoveRows(ind, 0, numRows);
    node->clear();
    endRemoveRows();
}

bool SvnItemModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return true;
    }
    return static_cast<SvnItemModelNode *>(parent.internalPointer())->NodeHasChilds();
}

bool SvnItemModel::filterIndex(const QModelIndex &parent, int childRow, svnmodel::ItemTypeFlag showOnly) const
{
    SvnItemModelNode *node = m_Data->nodeForIndex(parent);
    if (childRow < 0) {
        return false;
    }
    if (!node->NodeIsDir()) {
        qCDebug(KDESVN_LOG) << "Parent ist kein Dir" << Qt::endl;
        return false;
    }
    SvnItemModelNode *child = static_cast<SvnItemModelNodeDir *>(node)->child(childRow);
    if (child) {
        if ((child->isDir() && !showOnly.testFlag(svnmodel::Dir)) || (!child->isDir() && !showOnly.testFlag(svnmodel::File))) {
            return true;
        }
        return ItemDisplay::filterOut(child);
    }
    return false;
}

QVariant SvnItemModel::data(const QModelIndex &index, int role) const
{
    SvnItemModelNode *node = m_Data->nodeForIndex(index);
    switch (role) {
    case Qt::DisplayRole:
    case SORT_ROLE:
        switch (index.column()) {
        case Name:
            return node->shortName();
        case Status:
            return node->infoText();
        case LastRevision:
            return QString::number(node->cmtRev());
        case LastAuthor:
            return node->cmtAuthor();
        case LastDate:
            return node->fullDate();
        case Locked:
            return node->lockOwner();
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            int size = Kdesvnsettings::listview_icon_size();
            bool overlay = Kdesvnsettings::display_overlays();
            return node->getPixmap(size, overlay);
        }
        break;
    case Qt::EditRole:
        switch (index.column()) {
        case Name:
            return node->shortName();
        }
        break;
    case Qt::BackgroundRole: {
        QColor cl = node->backgroundColor();
        if (cl.isValid()) {
            return QBrush(cl);
        }
        break;
    }
    case Qt::ToolTipRole: {
        switch (index.column()) {
        case Name:
            if (node->hasToolTipText()) {
                return node->getToolTipText();
            } else {
                m_Data->m_InfoThread->appendNode(node);
                return QVariant();
            }
        }
        break;
    }
    }
    return QVariant();
}

QModelIndex SvnItemModel::index(int row, int column, const QModelIndex &parent) const
{
    SvnItemModelNode *node = m_Data->nodeForIndex(parent);
    if (row < 0) {
        return QModelIndex();
    }
    Q_ASSERT(node->NodeIsDir());
    SvnItemModelNode *child = static_cast<SvnItemModelNodeDir *>(node)->child(row);
    if (child) {
        return createIndex(row, column, child);
    } else {
        return QModelIndex();
    }
}

QVariant SvnItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case Name:
            return (i18n("Name"));
        case Status:
            return (i18n("Status"));
        case LastRevision:
            return (i18n("Last changed Revision"));
        case LastAuthor:
            return (i18n("Last author"));
        case LastDate:
            return (i18n("Last change date"));
        case Locked:
            return (i18n("Locked by"));
        }
    }
    return QVariant();
}

int SvnItemModel::columnCount(const QModelIndex & /*parent*/) const
{
    return ColumnCount;
}

int SvnItemModel::rowCount(const QModelIndex &parent) const
{
    if (!m_Data || !m_Data->m_rootNode) {
        return 0;
    }

    if (!parent.isValid()) {
        return m_Data->m_rootNode->childList().count();
    }
    SvnItemModelNodeDir *node = static_cast<SvnItemModelNodeDir *>(m_Data->nodeForIndex(parent));
    return node->childList().count();
}

QModelIndex SvnItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    SvnItemModelNode *child = static_cast<SvnItemModelNode *>(index.internalPointer());
    return m_Data->indexForNode(child->parent());
}

SvnActions *SvnItemModel::svnWrapper()
{
    return m_Data->m_SvnActions;
}

int SvnItemModel::checkDirs(const QString &_what, SvnItemModelNode *_parent)
{
    QString what = _what;
    svn::StatusEntries dlist;
    while (what.endsWith(QLatin1Char('/'))) {
        what.chop(1);
    }
    // prevent this from checking unversioned folder. FIXME: what happen when we do open url on a non-working-copy folder??
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif

    if (!m_Data->m_Display->isWorkingCopy() || (!_parent) || ((_parent) && (_parent->isVersioned()))) {
        if (!svnWrapper()->makeStatus(what, dlist, m_Data->m_Display->baseRevision(), false, true, true)) {
            return -1;
        }
    } else {
        return checkUnversionedDirs(_parent);
    }
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Time for getting entries: " << _counttime.elapsed();
    _counttime.restart();
#endif
    svn::StatusEntries neweritems;
    svnWrapper()->getaddedItems(what, neweritems);
    dlist += neweritems;
    SvnItemModelNode *node = nullptr;
    for (auto it = dlist.begin(); it != dlist.end(); ++it) {
        const svn::StatusPtr &sp = *it;
        if (sp->path() == what || sp->entry().url().toString() == what) {
            if (!_parent) {
                // toplevel item
                beginInsertRows(m_Data->indexForNode(m_Data->m_rootNode), 0, 0);
                if (sp->entry().kind() == svn_node_dir) {
                    node = new SvnItemModelNodeDir(m_Data->m_rootNode, svnWrapper(), m_Data->m_Display);
                } else {
                    node = new SvnItemModelNode(m_Data->m_rootNode, svnWrapper(), m_Data->m_Display);
                }
                node->setStat(sp);
                m_Data->m_rootNode->m_Children.prepend(node);
                endInsertRows();
            }
            dlist.erase(it);
            break;
        }
    }
    if (_parent) {
        node = _parent;
    }
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Time finding parent node: " << _counttime.elapsed();
#endif
    insertDirs(node, dlist);
    return dlist.size();
}

void SvnItemModel::insertDirs(SvnItemModelNode *_parent, svn::StatusEntries &dlist)
{
    if (dlist.isEmpty()) {
        return;
    }
    QModelIndex ind = m_Data->indexForNode(_parent);
    SvnItemModelNodeDir *parent;
    if (!_parent) {
        parent = m_Data->m_rootNode;
    } else {
        parent = static_cast<SvnItemModelNodeDir *>(_parent);
    }
    SvnItemModelNode *node = nullptr;
    beginInsertRows(ind, parent->childList().count(), parent->childList().count() + dlist.count() - 1);
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif
    for (const svn::StatusPtr &sp : dlist) {
#ifdef DEBUG_TIMER
        _counttime.restart();
#endif
        if (m_Data->MustCreateDir(*sp)) {
            node = new SvnItemModelNodeDir(parent, svnWrapper(), m_Data->m_Display);
        } else {
            node = new SvnItemModelNode(parent, svnWrapper(), m_Data->m_Display);
        }
        node->setStat(sp);
#ifdef DEBUG_TIMER
        //        qCDebug(KDESVN_LOG)<<"Time creating item: "<<_counttime.elapsed();
        _counttime.restart();
#endif
        if (m_Data->m_Display->isWorkingCopy() && m_Data->m_DirWatch) {
            if (node->isDir()) {
                m_Data->addWatchDir(node->fullName());
            } else {
                m_Data->addWatchFile(node->fullName());
            }
        }
#ifdef DEBUG_TIMER
        //        qCDebug(KDESVN_LOG)<<"Time add watch: "<<_counttime.elapsed();
        _counttime.restart();
#endif
        parent->m_Children.append(node);
#ifdef DEBUG_TIMER
//        qCDebug(KDESVN_LOG)<<"Time append node: "<<_counttime.elapsed();
#endif
    }
#ifdef DEBUG_TIMER
    _counttime.restart();
#endif
    endInsertRows();
#ifdef DEBUG_TIMER
//    qCDebug(KDESVN_LOG)<<"Time append all node: "<<_counttime.elapsed();
#endif
}

bool SvnItemModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return false;
    }
    SvnItemModelNode *node = static_cast<SvnItemModelNode *>(parent.internalPointer());
    return node->NodeHasChilds() && static_cast<SvnItemModelNodeDir *>(node)->childList().isEmpty();
}

void SvnItemModel::fetchMore(const QModelIndex &parent)
{
    SvnItemModelNode *node = static_cast<SvnItemModelNode *>(parent.internalPointer());
    if (!node->isDir()) {
        return;
    }
    if (checkDirs(node->fullName(), node) > 0) {
        emit itemsFetched(parent);
    }
}

bool SvnItemModel::insertRows(int, int, const QModelIndex &)
{
    return false;
}

bool SvnItemModel::insertColumns(int, int, const QModelIndex &)
{
    return false;
}

bool SvnItemModel::removeRows(int, int, const QModelIndex &)
{
    return false;
}

bool SvnItemModel::removeColumns(int, int, const QModelIndex &)
{
    return false;
}
Qt::ItemFlags SvnItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == Name) {
        f |= /*Qt::ItemIsEditable |*/ Qt::ItemIsDragEnabled;
    }
    if (!index.isValid()) {
        f |= Qt::ItemIsDropEnabled;
    } else {
        SvnItemModelNode *node = m_Data->nodeForIndex(index);
        if (node && node->isDir()) {
            f |= Qt::ItemIsDropEnabled;
        }
    }
    return f;
}

Qt::DropActions SvnItemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList SvnItemModel::mimeTypes() const
{
    return QStringList() << QLatin1String("text/uri-list")
                         /*                         << QLatin1String( "application/x-kde-cutselection" ) */ // TODO
                         //<< QLatin1String( "text/plain" )
                         << QLatin1String("application/x-kde-urilist");
}

bool SvnItemModel::dropUrls(const QList<QUrl> &data, Qt::DropAction action, int row, int column, const QModelIndex &parent, bool intern)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (action == Qt::IgnoreAction) {
        return true;
    }
    if (action == Qt::LinkAction) {
        return false;
    }
    emit urlDropped(data, action, parent, intern);
    return true;
}

QMimeData *SvnItemModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    for (const QModelIndex &index : indexes) {
        if (index.column() == 0) {
            urls << m_Data->nodeForIndex(index)->kdeName(m_Data->m_Display->baseRevision());
        }
    }
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(urls);

    KUrlMimeData::MetaDataMap metaMap;
    metaMap[QStringLiteral("kdesvn-source")] = QLatin1Char('t');
    metaMap[QStringLiteral("kdesvn-id")] = uniqueIdentifier();
    KUrlMimeData::setMetaData(metaMap, mimeData);

    return mimeData;
}

void SvnItemModel::makeIgnore(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    SvnItemModelNode *node = m_Data->nodeForIndex(index);
    if (!node || node == m_Data->m_rootNode || node->isRealVersioned()) {
        return;
    }
    SvnItemModelNodeDir *pa = node->parent();
    if (!pa) {
        return;
    }
    if (m_Data->m_SvnActions->makeIgnoreEntry(node, node->isIgnored())) {
        refreshIndex(index);
        refreshItem(pa);
    }
}

bool SvnItemModel::refreshItem(SvnItemModelNode *item)
{
    if (!item || item == m_Data->m_rootNode) {
        return false;
    }
    try {
        item->setStat(m_Data->m_SvnActions->svnclient()->singleStatus(item->fullName(), false, m_Data->m_Display->baseRevision()));
    } catch (const svn::ClientException &e) {
        item->setStat(svn::StatusPtr(new svn::Status));
        return false;
    }
    return true;
}

bool SvnItemModel::refreshIndex(const QModelIndex &idx)
{
    bool ret = refreshItem(m_Data->nodeForIndex(idx));
    emitDataChangedRow(idx);
    return ret;
}

void SvnItemModel::emitDataChangedRow(const QModelIndex &idx)
{
    const auto colS(index(idx.row(), 0, idx.parent()));
    const auto colE(index(idx.row(), columnCount() - 1, idx.parent()));
    emit dataChanged(colS, colE);
}

SvnItemModelNode *SvnItemModel::findPath(const svn::Path &_p)
{
    QString ip = _p.path();
    SvnItemModelNode *n1 = firstRootChild();
    if (n1) {
        if (n1->fullName().length() < ip.length()) {
            ip = ip.right(ip.length() - n1->fullName().length());
        } else if (n1->fullName() == ip) {
            return n1;
        }
        if (!n1->isDir()) {
            return nullptr;
        }
        const QVector<QString> lp = ip.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        SvnItemModelNodeDir *d1 = static_cast<SvnItemModelNodeDir *>(n1);
        return d1->findPath(lp);
    }
    return nullptr;
}

QModelIndex SvnItemModel::findIndex(const svn::Path &_p)
{
    return m_Data->indexForNode(findPath(_p));
}

void SvnItemModel::initDirWatch()
{
    delete m_Data->m_DirWatch;
    m_Data->m_DirWatch = nullptr;
    if (m_Data->m_Display->isWorkingCopy()) {
        m_Data->m_DirWatch = new KDirWatch(this);
        connect(m_Data->m_DirWatch, &KDirWatch::dirty, this, &SvnItemModel::slotDirty);
        connect(m_Data->m_DirWatch, &KDirWatch::created, this, &SvnItemModel::slotCreated);
        connect(m_Data->m_DirWatch, &KDirWatch::deleted, this, &SvnItemModel::slotDeleted);
        if (m_Data->m_DirWatch) {
            m_Data->m_DirWatch->addDir(m_Data->m_Display->baseUri() + QLatin1Char('/'), KDirWatch::WatchDirOnly);
            m_Data->m_DirWatch->startScan(true);
        }
    }
}

void SvnItemModel::slotCreated(const QString &what)
{
    QModelIndex ind = findIndex(what);
    if (!ind.isValid()) {
        return;
    }
    SvnItemModelNode *n = static_cast<SvnItemModelNode *>(ind.internalPointer());
    if (!n) {
        return;
    }
    if (n->isRealVersioned()) {
        refreshIndex(ind);
    }
}

void SvnItemModel::slotDeleted(const QString &what)
{
    QModelIndex ind = findIndex(what);
    if (!ind.isValid()) {
        m_Data->m_DirWatch->removeDir(what);
        m_Data->m_DirWatch->removeFile(what);
        return;
    }
    SvnItemModelNode *n = static_cast<SvnItemModelNode *>(ind.internalPointer());
    if (!n) {
        return;
    }
    if (!n->isRealVersioned()) {
        SvnItemModelNodeDir *p = n->parent();
        QModelIndex pi = m_Data->indexForNode(p);
        if (!pi.isValid()) {
            return;
        }
        if (ind.row() >= p->m_Children.count()) {
            return;
        }
        beginRemoveRows(pi, ind.row(), ind.row());
        p->m_Children.removeAt(ind.row());
        endRemoveRows();
        if (n->isDir()) {
            m_Data->m_DirWatch->removeDir(what);
        } else {
            m_Data->m_DirWatch->removeFile(what);
        }
    } else {
        refreshIndex(ind);
    }
}

void SvnItemModel::checkAddNewItems(const QModelIndex &ind)
{
    SvnItemModelNodeDir *n = static_cast<SvnItemModelNodeDir *>(ind.internalPointer());
    QString what = n->fullName();
    svn::StatusEntries dlist;
    while (what.endsWith(QLatin1Char('/'))) {
        what.chop(1);
    }
    if (!svnWrapper()->makeStatus(what, dlist, m_Data->m_Display->baseRevision(), false, true, true)) {
        return;
    }
    const auto pred = [&](const svn::StatusPtr &sp) -> bool {
        return n->contains(sp->path()) || sp->path() == what;
    };
    dlist.erase(std::remove_if(dlist.begin(), dlist.end(), pred), dlist.end());
    if (!dlist.isEmpty()) {
        insertDirs(n, dlist);
    }
}

void SvnItemModel::slotDirty(const QString &what)
{
    QModelIndex ind = findIndex(what);
    if (!ind.isValid()) {
        return;
    }
    SvnItemModelNode *n = static_cast<SvnItemModelNode *>(ind.internalPointer());
    if (!n) {
        return;
    }
    if (n->isRealVersioned()) {
        if (!n->isDir()) {
            refreshIndex(ind);
        } else {
            checkAddNewItems(ind);
        }
    } else if (n->isDir()) {
        checkUnversionedDirs(n);
    }
}

bool SvnItemModel::checkRootNode()
{
    if (!m_Data->m_rootNode) {
        return false;
    }
    try {
        m_Data->m_rootNode->setStat(m_Data->m_SvnActions->svnclient()->singleStatus(m_Data->m_Display->baseUri(), false, m_Data->m_Display->baseRevision()));
    } catch (const svn::ClientException &e) {
        m_Data->m_rootNode->setStat(svn::StatusPtr(new svn::Status));
        emit clientException(e.msg());
        return false;
    }
    return true;
}

bool SvnItemModel::refreshCurrentTree()
{
    bool check_created = false;
    if (!m_Data->m_rootNode) {
        return false;
    }
    SvnItemModelNodeDir *_start = m_Data->m_rootNode;
    if (m_Data->m_Display->isWorkingCopy()) {
        if (!m_Data->m_rootNode->m_Children.isEmpty() && m_Data->m_rootNode->m_Children.at(0)->NodeIsDir()) {
            _start = static_cast<SvnItemModelNodeDir *>(m_Data->m_rootNode->m_Children.at(0));
            refreshItem(_start);
        } else {
            return false;
        }
    } else {
        if (!checkRootNode()) {
            return false;
        }
        _start = m_Data->m_rootNode;
        check_created = true;
    }
    return refreshDirnode(_start, check_created);
}

bool SvnItemModel::refreshDirnode(SvnItemModelNodeDir *node, bool check_empty, bool notrec)
{
    if (!node) {
        if (m_Data->m_Display->isWorkingCopy()) {
            return false;
        } else {
            if (!checkRootNode()) {
                return false;
            }
            node = m_Data->m_rootNode;
        }
    }
    QString what = (node != m_Data->m_rootNode) ? node->fullName() : m_Data->m_Display->baseUri();

    if (node->m_Children.isEmpty() && !check_empty) {
        if (node->fullName() == m_Data->m_Display->baseUri()) {
            return refreshItem(node);
        }
        return true;
    }
    svn::StatusEntries dlist;

    if (!svnWrapper()->makeStatus(what, dlist, m_Data->m_Display->baseRevision())) {
        return false;
    }
    if (m_Data->m_Display->isWorkingCopy()) {
        svn::StatusEntries neweritems;
        svnWrapper()->getaddedItems(what, neweritems);
        dlist += neweritems;
    }

    for (auto it = dlist.begin(); it != dlist.end(); ++it) {
        if ((*it)->path() == what) {
            dlist.erase(it);
            break;
        }
    }
    QModelIndex ind = m_Data->indexForNode(node);
    for (int i = 0; i < node->m_Children.size(); ++i) {
        const SvnItemModelNode *n = node->m_Children[i];
        bool found = false;
        for (const auto &entry : std::as_const(dlist)) {
            if (entry->path() == n->fullName()) {
                found = true;
                break;
            }
        }
        if (!found) {
            beginRemoveRows(ind, i, i);
            node->m_Children.removeAt(i);
            delete n;
            endRemoveRows();
            --i;
        }
    }

    for (auto it = dlist.begin(); it != dlist.end();) {
        int index = node->indexOf((*it)->path());
        if (index != -1) {
            SvnItemModelNode *n = node->m_Children[index];
            n->setStat((*it));
            if (n->NodeIsDir() != n->isDir()) {
                beginRemoveRows(ind, index, index);
                node->m_Children.removeAt(index);
                delete n;
                endRemoveRows();
            } else {
                it = dlist.erase(it);
            }
        } else {
            ++it;
        }
    }

    // make sure that we do not read in the whole tree when just refreshing the current tree.
    if (!node->m_Children.isEmpty() && !notrec) {
        for (auto &child : node->m_Children) {
            if (child->NodeIsDir()) {
                // both other parameters makes no sense at this point - defaults
                refreshDirnode(static_cast<SvnItemModelNodeDir *>(child), false, false);
            }
        }
    }
    // after so we don't recurse about it.
    insertDirs(node, dlist);
    if (!dlist.isEmpty()) {
        emit itemsFetched(m_Data->indexForNode(node));
    }
    return true;
}

int SvnItemModel::checkUnversionedDirs(SvnItemModelNode *_parent)
{
    if (!_parent || !_parent->isDir()) {
        // no toplevel unversioned - kdesvn is not a filemanager
        return 0;
    }
    QDir d(_parent->fullName());
    d.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = d.entryInfoList();
    if (list.isEmpty()) {
        return 0;
    }
    svn::StatusEntries dlist;
    SvnItemModelNodeDir *n = static_cast<SvnItemModelNodeDir *>(_parent);
    for (const auto &fi : list) {
        if (!(n->contains(fi.absoluteFilePath()) || fi.absoluteFilePath() == n->fullName())) {
            svn::StatusPtr stat(new svn::Status(fi.absoluteFilePath()));
            dlist.append(stat);
        }
    }
    if (!dlist.isEmpty()) {
        insertDirs(_parent, dlist);
    }
    return dlist.size();
}

const QString &SvnItemModel::uniqueIdentifier() const
{
    return m_Data->m_Uid;
}

void SvnItemModel::slotNotifyMessage(const QString &msg)
{
    qCDebug(KDESVN_LOG) << msg;
}

#include "moc_svnitemmodel.cpp"
