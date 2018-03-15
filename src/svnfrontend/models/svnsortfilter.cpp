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

#include "svnsortfilter.h"
#include "svnitemmodel.h"
#include "svnitemnode.h"
#include "settings/kdesvnsettings.h"

SvnSortFilterProxy::SvnSortFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent), m_sourceModel(nullptr), m_ShowFilter(svnmodel::All)
{
}

void SvnSortFilterProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<SvnItemModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool SvnSortFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right)const
{
    if (!(left.isValid() && right.isValid())) {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    SvnItemModelNode *n1 = static_cast<SvnItemModelNode *>(left.internalPointer());
    SvnItemModelNode *n2 = static_cast<SvnItemModelNode *>(right.internalPointer());
    /*
     * when having valid model indexes the internal pointer MUST be valid, too.
     * so we may skip if for this.
     */
    Q_ASSERT(n1 && n2);
    if (n1->sortChar() == n2->sortChar()) {
        if (sortColumn() == SvnItemModel::LastRevision) {
            return n1->cmtRev() < n2->cmtRev();
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }
    // we want folders always @first
    if (sortOrder() == Qt::AscendingOrder) {
        return n1->sortChar() < n2->sortChar();
    } else {
        return n1->sortChar() > n2->sortChar();
    }
}

bool SvnSortFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
    if (m_sourceModel->filterIndex(source_parent, source_row, m_ShowFilter)) {
        return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void SvnSortFilterProxy::setShowFilter(svnmodel::ItemTypeFlag fl)
{
    m_ShowFilter = fl;
    invalidateFilter();
}
