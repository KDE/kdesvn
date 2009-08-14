/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/

#include "svnsortfilter.h"
#include "svnitemmodel.h"
#include "svnitemnode.h"
#include "src/settings/kdesvnsettings.h"
#include <kdebug.h>

SvnSortFilterProxy::SvnSortFilterProxy(QObject *parent)
    :QSortFilterProxyModel(parent),m_sourceModel(0),m_order(Qt::AscendingOrder),m_ShowFilter(svnmodel::All)
{
}

SvnSortFilterProxy::~SvnSortFilterProxy()
{
}

void SvnSortFilterProxy::sort(int column,Qt::SortOrder order)
{
    m_order = order;
    QSortFilterProxyModel::sort(column,order);
}

bool SvnSortFilterProxy::hasChildren(const QModelIndex & parent)const
{
    if (!parent.isValid()) {
        return true;
    }
    if (m_sourceModel) {
        QModelIndex ind = mapToSource(parent);
        return m_sourceModel->hasChildren(ind);
    } else {
        return QSortFilterProxyModel::hasChildren(parent);
    }
}

void SvnSortFilterProxy::setSourceSvnModel(SvnItemModel*sourceModel)
{
    m_sourceModel=sourceModel;
    setSourceModel(sourceModel);
}

bool SvnSortFilterProxy::lessThan(const QModelIndex & left,const QModelIndex & right)const
{
    if (!(left.isValid() && right.isValid())) {
        return QSortFilterProxyModel::lessThan(left,right);
    }
    SvnItemModelNode *n1,*n2;
    n1 = static_cast<SvnItemModelNode*>(left.internalPointer());
    n2 = static_cast<SvnItemModelNode*>(right.internalPointer());
    /*
     * when having valid model indexes the internal pointer MUST be valid, too.
     * so we may skip if for this.
     */
    Q_ASSERT(n1 && n2);
    if (n1->sortChar()==n2->sortChar()) {
        if (sortColumn()==SvnItemModel::LastRevision) {
            return n1->cmtRev()<n2->cmtRev();
        }
        return QSortFilterProxyModel::lessThan(left,right);
    }
    // we want folders always @first
    if (m_order==Qt::AscendingOrder) {
        return n1->sortChar()<n2->sortChar();
    } else {
        return n1->sortChar()>n2->sortChar();
    }
}

bool SvnSortFilterProxy::filterAcceptsRow(int source_row, const QModelIndex & source_parent)const
{
    if (m_sourceModel->filterIndex(source_parent,source_row,m_ShowFilter)) {
        return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row,source_parent);
}

void SvnSortFilterProxy::setShowFilter(svnmodel::ItemTypeFlag fl)
{
    m_ShowFilter = fl;
}

#include "svnsortfilter.moc"
