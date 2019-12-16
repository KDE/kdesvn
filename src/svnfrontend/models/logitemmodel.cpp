/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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

#include "logitemmodel.h"
#include "logmodelhelper.h"

#include "svnqt/client.h"

#include <KLocalizedString>
#include <QTreeWidget>
#include <QMap>

SvnLogModel::SvnLogModel(const svn::LogEntriesMapPtr &_log, const QString &m_name, QObject *parent)
    : QAbstractListModel(parent)
    , m_emptyString()
    , m_min(-1)
    , m_max(-1)
    , m_name()
    , m_left(-1)
    , m_right(-1)
{
    setLogData(_log, m_name);
}

void SvnLogModel::setLogData(const svn::LogEntriesMapPtr &log, const QString &name)
{
    beginResetModel();
    m_data.clear();
    endResetModel();
    m_name = name;
    m_left = m_right = -1;

    QMap<long int, SvnLogModelNodePtr> itemMap;

    m_min = m_max = -1;

    if (log->isEmpty()) {
        return;
    }

    m_data.reserve(log->count());
    beginInsertRows(QModelIndex(), 0, log->count() - 1);
    for (const svn::LogEntry &entry : qAsConst(*log)) {
        SvnLogModelNodePtr np(new SvnLogModelNode(entry));
        m_data.append(np);
        if (entry.revision > m_max) {
            m_max = entry.revision;
        }
        if (entry.revision < m_min || m_min == -1) {
            m_min = entry.revision;
        }
        itemMap[entry.revision] = np;
    }
    endInsertRows();
    QString bef = m_name;
    qlonglong rev;
    // YES! I'd checked it: this is much faster than getting list of keys
    // and iterating over that list!
    for (long c = m_max; c > -1; --c) {
        if (!itemMap.contains(c)) {
            continue;
        }
        if (itemMap[c]->realName().isEmpty()) {
            itemMap[c]->setRealName(bef);
        }
        itemMap[c]->copiedFrom(bef, rev);
    }
}

qlonglong SvnLogModel::min() const
{
    return m_min;
}

qlonglong SvnLogModel::max() const
{
    return m_max;
}

int SvnLogModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_data.count();
}

QVariant SvnLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }
    const SvnLogModelNodePtr &_l = m_data.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case Revision:
            return _l->revision();
        case Author:
            return _l->author();
        case Date:
            return _l->date();
        case Message:
            return _l->shortMessage();
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (index.row() == m_left) {
                return QIcon::fromTheme(QStringLiteral("kdesvnleft"));
            }
            if (index.row() == m_right) {
                return QIcon::fromTheme(QStringLiteral("kdesvnright"));
            }
            return QStringLiteral("   ");
        }
        break;
    }
    return QVariant();
}

qlonglong SvnLogModel::toRevision(const QModelIndex &index)const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return -1;
    }
    return m_data[index.row()]->revision();
}

const QString &SvnLogModel::fullMessage(const QModelIndex &index)const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return m_emptyString;
    }
    return m_data[index.row()]->message();
}

const QString &SvnLogModel::realName(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return m_emptyString;
    }
    return m_data[index.row()]->realName();
}

int SvnLogModel::columnCount(const QModelIndex &idx) const
{
    return idx.isValid() ? 0 : Count;
}

QVariant SvnLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case Revision:
            return i18n("Revision");
        case Author:
            return i18n("Author");
        case Date:
            return i18n("Date");
        case Message:
            return i18n("Message");
        }
    }
    return QVariant();
}

SvnLogModelNodePtr SvnLogModel::indexNode(const QModelIndex &index)const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return SvnLogModelNodePtr();
    }
    return m_data.at(index.row());
}

void SvnLogModel::fillChangedPaths(const QModelIndex &index, QTreeWidget *where)
{
    if (!where || !index.isValid() || index.row() >= m_data.count()) {
        return;
    }
    where->clear();
    const SvnLogModelNodePtr &_l = m_data.at(index.row());
    if (_l->changedPaths().isEmpty()) {
        return;
    }
    QList<QTreeWidgetItem *> _list;
    for (const svn::LogChangePathEntry &entry : _l->changedPaths()) {
        _list.append(new LogChangePathItem(entry));
    }
    where->addTopLevelItems(_list);
    where->resizeColumnToContents(0);
    where->resizeColumnToContents(1);
    where->resizeColumnToContents(2);
    where->sortByColumn(1, Qt::AscendingOrder);
}

int SvnLogModel::leftRow()const
{
    return m_left;
}

int SvnLogModel::rightRow()const
{
    return m_right;
}

void SvnLogModel::setLeftRow(int v)
{
    if (m_right == v) {
        m_right = -1;
    }
    m_left = v;
}

void SvnLogModel::setRightRow(int v)
{
    if (m_left == v) {
        m_left = -1;
    }
    m_right = v;
}

//
// SvnLogSortModel
//
void SvnLogSortModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<SvnLogModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool SvnLogSortModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if(source_left.column() != source_right.column() || !m_sourceModel) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    const SvnLogModelNodePtr &dataLeft = m_sourceModel->m_data.at(source_left.row());
    const SvnLogModelNodePtr &dataRight = m_sourceModel->m_data.at(source_right.row());
    switch (source_left.column()) {
        case SvnLogModel::Author:
            return dataLeft->author() < dataRight->author();
        case SvnLogModel::Revision:
            return dataLeft->revision() < dataRight->revision();
        case SvnLogModel::Date:
            return dataLeft->dateMSec() < dataRight->dateMSec();
        case SvnLogModel::Message:
            return dataLeft->message() < dataRight->message();
        default:
            break;
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
