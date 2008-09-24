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
#include <QSortFilterProxyModel>

#ifndef _SVNSORTFILTER_H
#define _SVNSORTFILTER_H

class SvnItemModel;

class SvnSortFilterProxy:public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SvnSortFilterProxy(QObject *parent = 0);
    virtual ~SvnSortFilterProxy();

    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual void setSourceSvnModel(SvnItemModel* sourceModel);
    virtual void sort(int column,Qt::SortOrder order = Qt::AscendingOrder);

protected:
    virtual bool lessThan(const QModelIndex & left,const QModelIndex & right)const;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent )const;

private:
    SvnItemModel*m_sourceModel;
    Qt::SortOrder m_order;
};

#endif


