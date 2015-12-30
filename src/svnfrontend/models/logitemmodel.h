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

#ifndef LOG_ITEM_MODEL_H
#define LOG_ITEM_MODEL_H

#include <QAbstractListModel>

#include "src/svnqt/svnqttypes.h"

class SvnLogModelData;
class SvnLogModelNode;
class QTreeWidget;

typedef QSharedPointer<SvnLogModelNode> SvnLogModelNodePtr;

class SvnLogModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    SvnLogModel(const svn::LogEntriesMapPtr &_log, const QString &_name, QObject *parent);
    virtual ~SvnLogModel();
    void setLogData(const svn::LogEntriesMapPtr &_log, const QString &_name);
    QVariant data(const QModelIndex &index, int role) const;

    virtual QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex())const;
    virtual QModelIndex parent(const QModelIndex &)const;

    qlonglong toRevision(const QModelIndex &)const;
    const QString &fullMessage(const QModelIndex &index)const;
    void fillChangedPaths(const QModelIndex &index, QTreeWidget *target);
    const QString &realName(const QModelIndex &index);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    enum Columns {
        Author = 0,
        Revision,
        Date,
        Message,
        Count
    };
    virtual int rowCount(const QModelIndex &parent)const;
    virtual SvnLogModelNodePtr indexNode(const QModelIndex &)const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex &)const;

    long leftRow()const;
    long rightRow()const;
    void setLeftRow(long);
    void setRightRow(long);

    long min() const;
    long max() const;

private:
    QSharedPointer<SvnLogModelData> m_data;
};

#endif
