/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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

#include "logitemmodel.h"
#include "logmodelhelper.h"

#include "src/svnqt/client.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

#include <QStringList>
#include <QTreeWidget>
#include <QMap>

class SvnLogModelData
{
public:
    SvnLogModelData()
        : m_List(), m_rowCount(-1), m_Empty(), _min(-1), _max(-1), _name(), _left(-1), _right(-1)
    {
    }
    QVector<SvnLogModelNodePtr> m_List;
    int m_rowCount;
    QString m_Empty;
    long _min, _max;
    QString _name;
    long _left, _right;
};

SvnLogModel::SvnLogModel(const svn::LogEntriesMapPtr &_log, const QString &_name, QObject *parent)
    : QAbstractItemModel(parent), m_data(new SvnLogModelData)
{
    setLogData(_log, _name);
}

SvnLogModel::~SvnLogModel()
{
}

QModelIndex SvnLogModel::index(int row, int column, const QModelIndex &parent)const
{
    Q_UNUSED(parent);
    if (row >= m_data->m_List.count() || row < 0) {
        return QModelIndex();
    }
    SvnLogModelNodePtr n = m_data->m_List.at(row);
    return createIndex(row, column, n.data());
}

Qt::ItemFlags SvnLogModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled/*|Qt::ItemIsUserCheckable*/;
    }
    return 0;
}

QModelIndex SvnLogModel::parent(const QModelIndex &)const
{
    // we have no tree...
    return QModelIndex();
}

void SvnLogModel::setLogData(const svn::LogEntriesMapPtr &_log, const QString &_name)
{
    beginRemoveRows(QModelIndex(), 0, m_data->m_List.count());
    m_data->m_List.clear();
    endRemoveRows();
    m_data->_name = _name;
    m_data->_left = m_data->_right = -1;

    QMap<long int, SvnLogModelNodePtr> itemMap;

    m_data->_min = m_data->_max = -1;

    m_data->m_List.reserve(_log->count());
    beginInsertRows(QModelIndex(), 0, _log->count());
    svn::LogEntriesMap::const_iterator it = _log->constBegin();
    for (; it != _log->constEnd(); ++it) {
        SvnLogModelNodePtr np(new SvnLogModelNode((*it)));
        m_data->m_List.append(np);
        if ((*it).revision > m_data->_max) {
            m_data->_max = (*it).revision;
        }
        if ((*it).revision < m_data->_min || m_data->_min == -1) {
            m_data->_min = (*it).revision;
        }
        itemMap[(*it).revision] = np;
    }
    endInsertRows();
    QString bef = m_data->_name;
    long rev;
    // YES! I'd checked it: this is much faster than getting list of keys
    // and iterating over that list!
    for (long c = m_data->_max; c > -1; --c) {
        if (!itemMap.contains(c)) {
            continue;
        }
        if (itemMap[c]->realName().isEmpty()) {
            itemMap[c]->setRealName(bef);
        }
        itemMap[c]->copiedFrom(bef, rev);
    }
}

long SvnLogModel::min() const
{
    return m_data->_min;
}

long SvnLogModel::max() const
{
    return m_data->_max;
}

int SvnLogModel::rowCount(const QModelIndex &parent)const
{
    Q_UNUSED(parent);
    return m_data->m_List.count();
}

QVariant SvnLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data->m_List.count()) {
        return QVariant();
    }
    const SvnLogModelNodePtr &_l = m_data->m_List.at(index.row());

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
            if (index.row() == m_data->_left) {
                return KIcon("kdesvnleft");
            } else if (index.row() == m_data->_right) {
                return KIcon("kdesvnright");
            } else {
                return QString("   ");
            }
        }
        break;
    }
    return QVariant();
}

qlonglong SvnLogModel::toRevision(const QModelIndex &index)const
{
    if (!index.isValid() || index.row() >= m_data->m_List.count()) {
        return -1;
    }
    return m_data->m_List[index.row()]->revision();
}

const QString &SvnLogModel::fullMessage(const QModelIndex &index)const
{
    if (!index.isValid() || index.row() >= m_data->m_List.count()) {
        return m_data->m_Empty;
    }
    return m_data->m_List[index.row()]->message();
}

const QString &SvnLogModel::realName(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_data->m_List.count()) {
        return m_data->m_Empty;
    }
    return m_data->m_List[index.row()]->realName();
}

int SvnLogModel::columnCount(const QModelIndex &)const
{
    return Count;
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
    if (!index.isValid() || index.row() >= m_data->m_List.count()) {
        return SvnLogModelNodePtr();
    }
    return m_data->m_List.at(index.row());
}

void SvnLogModel::fillChangedPaths(const QModelIndex &index, QTreeWidget *where)
{
    if (!where || !index.isValid() || index.row() >= m_data->m_List.count()) {
        return;
    }
    where->clear();
    const SvnLogModelNodePtr &_l = m_data->m_List.at(index.row());
    if (_l->changedPaths().count() == 0) {
        return;
    }
    QList<QTreeWidgetItem *> _list;
    for (int i = 0; i < _l->changedPaths().count(); ++i) {
        _list.append(new LogChangePathItem(_l->changedPaths()[i]));
    }
    where->addTopLevelItems(_list);
    where->resizeColumnToContents(0);
    where->resizeColumnToContents(1);
    where->resizeColumnToContents(2);

}

long SvnLogModel::leftRow()const
{
    return m_data->_left;
}

long SvnLogModel::rightRow()const
{
    return m_data->_right;
}

void SvnLogModel::setLeftRow(long v)
{
    if (m_data->_right == v) {
        m_data->_right = -1;
    }
    m_data->_left = v;
}

void SvnLogModel::setRightRow(long v)
{
    if (m_data->_left == v) {
        m_data->_left = -1;
    }
    m_data->_right = v;
}
