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
#ifndef SVNSORTFILTER_H
#define SVNSORTFILTER_H

#include <QSortFilterProxyModel>

#include "svnitemmodelfwd.h"

class SvnSortFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    enum ShowType {
        None = 0x0,
        Dir  = 1,
        File = 2,
        All = Dir | File
    };

    Q_DECLARE_FLAGS(TypeFlag, ShowType)

    void setShowFilter(svnmodel::ItemTypeFlag);
    svnmodel::ItemTypeFlag showFilter() const
    {
        return m_ShowFilter;
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    SvnItemModel *m_sourceModel = nullptr;
    svnmodel::ItemTypeFlag m_ShowFilter = svnmodel::All;
};

#endif

