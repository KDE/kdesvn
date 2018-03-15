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

#include "commitmodel.h"
#include "commitmodelhelper.h"

#include "svnqt/commititem.h"

#include <KLocalizedString>

CommitModel::CommitModel(const svn::CommitItemList &aList, QObject *parent)
    : QAbstractItemModel(parent)
{
    setCommitData(aList);
}

/*********************
 * Begin CommitModel *
 *********************/
CommitModel::CommitModel(const CommitActionEntries &_checked, const CommitActionEntries &_notchecked, QObject *parent)
    : QAbstractItemModel(parent)
{
    setCommitData(_checked, _notchecked);
}

CommitModel::~CommitModel()
{
}

void CommitModel::setCommitData(const svn::CommitItemList &aList)
{
    beginRemoveRows(QModelIndex(), 0, m_List.count());
    m_List.clear();
    endRemoveRows();

    m_List.reserve(aList.size());
    beginInsertRows(QModelIndex(), 0, aList.size() - 1);
    for (int j = 0; j < aList.size(); ++j) {
        m_List.append(CommitModelNodePtr(new CommitModelNode(aList[j])));
    }
    endInsertRows();
}

void CommitModel::setCommitData(const CommitActionEntries &checked, const CommitActionEntries &notchecked)
{
    beginRemoveRows(QModelIndex(), 0, m_List.count());
    m_List.clear();
    endRemoveRows();

    m_List.reserve(checked.size() + notchecked.size());
    beginInsertRows(QModelIndex(), 0, checked.size() + notchecked.size() - 1);
    for (int j = 0; j < checked.size(); ++j) {
        m_List.append(CommitModelNodePtr(new CommitModelNode(checked[j], true)));
    }
    for (int j = 0; j < notchecked.size(); ++j) {
        m_List.append(CommitModelNodePtr(new CommitModelNode(notchecked[j], false)));
    }
    endInsertRows();
}

int CommitModel::ActionColumn()const
{
    return 0;
}

int CommitModel::ItemColumn()const
{
    return 1;
}

CommitModelNodePtr CommitModel::node(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_List.count()) {
        return CommitModelNodePtr();
    }
    return m_List.at(index.row());
}

CommitActionEntries CommitModel::checkedEntries()const
{
    CommitActionEntries res;
    for (int i = 0; i < m_List.count(); ++i) {
        if (m_List.at(i)->checked()) {
            res.append(m_List.at(i)->actionEntry());
        }
    }
    return res;
}

void CommitModel::markItems(bool mark, CommitActionEntry::ACTION_TYPE _type)
{
    QVariant v = mark ? int(2) : int(0);
    for (int i = 0; i < m_List.count(); ++i) {
        if (m_List.at(i)->actionEntry().type() & _type) {
            QModelIndex _index = index(i, 0, QModelIndex());
            setData(_index, v, Qt::CheckStateRole);
            dataChanged(_index, _index);
        }
    }
}

/*!
    \fn CommitModel::removeEntries(const QStringList&)
 */
void CommitModel::removeEntries(const QStringList &_items)
{
    QStringList items = _items;
    // items is normally much smaller than m_List, therefore
    // iterate over the items in the inner loop
    for (int j = m_List.count() - 1; j >= 0; --j) {
        const QString aeName = m_List.at(j)->actionEntry().name();
        for (int i = items.size() - 1; i >= 0; --i) {
            if (aeName == items.at(i)) {
                beginRemoveRows(QModelIndex(), j, j);
                m_List.remove(j);
                endRemoveRows();
                items.removeAt(i);
                break;  // break inner loop
            }
        }
        if (items.isEmpty())
          break;
    }
}

const CommitModelNodePtr CommitModel::dataForRow(int row) const
{
    if (row < 0 || row >= m_List.size())
        return CommitModelNodePtr();
    return m_List.at(row);
}


/************************************
 * begin overload of Model methods  *
 ************************************/
QModelIndex CommitModel::index(int row, int column, const QModelIndex & /*parent*/)const
{
    if (row < 0 || row >= m_List.count()) {
        return QModelIndex();
    }
    const CommitModelNodePtr &n = m_List.at(row);
    return createIndex(row, column, n.data());
}

QModelIndex CommitModel::parent(const QModelIndex &)const
{
    // we have no tree...
    return QModelIndex();
}

QVariant CommitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_List.count() || role != Qt::DisplayRole) {
        return QVariant();
    }
    const CommitModelNodePtr &n = m_List.at(index.row());
    if (index.column() == ActionColumn()) {
        return n->actionEntry().action();
    }
    if (index.column() == ItemColumn()) {
        return n->actionEntry().name();
    }
    return QVariant();
}

QVariant CommitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == ActionColumn()) {
            return i18n("Action");
        }
        if (section == ItemColumn()) {
            return i18n("Entry");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

int CommitModel::rowCount(const QModelIndex &) const
{
    return m_List.count();
}

int CommitModel::columnCount(const QModelIndex &) const
{
    return 2;
}
/************************************
 * end overload of Model methods    *
 ************************************/
/*********************
 * end CommitModel   *
 *********************/

/************************************
 * begin CommitModelCheckitem       *
 ************************************/
CommitModelCheckitem::CommitModelCheckitem(const CommitActionEntries &_checked, const CommitActionEntries &_notchecked, QObject *parent)
    : CommitModel(_checked, _notchecked, parent)
{
}

CommitModelCheckitem::~CommitModelCheckitem()
{
}

int CommitModelCheckitem::ActionColumn()const
{
    return 1;
}

int CommitModelCheckitem::ItemColumn()const
{
    return 0;
}

Qt::ItemFlags CommitModelCheckitem::flags(const QModelIndex &index) const
{
    if (index.isValid() && index.column() == ItemColumn()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    return CommitModel::flags(index);
}

QVariant CommitModelCheckitem::data(const QModelIndex &index, int role) const
{
    if (index.column() != ItemColumn() || role != Qt::CheckStateRole || !index.isValid() || index.row() >= m_List.count()) {
        return CommitModel::data(index, role);
    }
    if (m_List.at(index.row())->checked()) {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

bool CommitModelCheckitem::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != ItemColumn() || role != Qt::CheckStateRole || !index.isValid() || index.row() >= m_List.count()) {
        return CommitModel::setData(index, value, role);
    }
    if (value.type() == QVariant::Int) {
        CommitModelNodePtr _l = m_List.at(index.row());
        bool old = _l->checked();
        bool nv = value.toInt() > 0;
        _l->setChecked(nv);
        if (old != nv) {
            emit dataChanged(index, index);
        }
        return old != nv;
    }
    return false;
}
/************************************
 * end CommitModelCheckitem         *
 ************************************/

/***************************
 * Begin CommitFilterModel *
 ***************************/
CommitFilterModel::CommitFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_sourceModel(nullptr)
    , m_visibleTypes(CommitActionEntry::ALL)
{}

CommitFilterModel::~CommitFilterModel()
{}

void CommitFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<CommitModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool CommitFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!m_sourceModel || source_parent.isValid())
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    const CommitModelNodePtr node = m_sourceModel->dataForRow(source_row);
    return ((node->actionEntry().type() & m_visibleTypes) != 0);
}

void CommitFilterModel::hideItems(bool bHide, CommitActionEntry::ACTION_TYPE aType)
{
    const CommitActionEntry::ActionTypes curVisibleTypes = m_visibleTypes;
    if (bHide) {
        m_visibleTypes &= ~aType;
    } else {
        m_visibleTypes |= aType;
    }
    if (m_visibleTypes != curVisibleTypes) {
        invalidateFilter();
    }
}

/*************************
 * end CommitFilterModel *
 *************************/
