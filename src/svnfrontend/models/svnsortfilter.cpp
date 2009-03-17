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
    if (!n1 || !n2 || n1->sortChar()==n2->sortChar()) {
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