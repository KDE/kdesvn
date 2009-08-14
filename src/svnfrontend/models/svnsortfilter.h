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
#ifndef SVNSORTFILTER_H
#define SVNSORTFILTER_H

#include <QSortFilterProxyModel>

#include "svnitemmodelfwd.h"

class SvnSortFilterProxy:public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SvnSortFilterProxy(QObject *parent = 0);
    virtual ~SvnSortFilterProxy();

    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual void setSourceSvnModel(SvnItemModel* sourceModel);
    virtual void sort(int column,Qt::SortOrder order = Qt::AscendingOrder);

    enum ShowType {
        None = 0x0,
        Dir  = 1,
        File = 2,
        All = Dir|File
    };

    Q_DECLARE_FLAGS(TypeFlag, ShowType)

    void setShowFilter(svnmodel::ItemTypeFlag);
    svnmodel::ItemTypeFlag showFilter()const
    {
        return m_ShowFilter;
    }

protected:
    virtual bool lessThan(const QModelIndex & left,const QModelIndex & right)const;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent )const;

    SvnItemModel*m_sourceModel;
    Qt::SortOrder m_order;
    svnmodel::ItemTypeFlag m_ShowFilter;
};

#endif


