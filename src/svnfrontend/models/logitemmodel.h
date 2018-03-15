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
#include <QSortFilterProxyModel>

#include "svnqt/svnqttypes.h"

class SvnLogModelNode;
class QTreeWidget;

typedef QSharedPointer<SvnLogModelNode> SvnLogModelNodePtr;

class SvnLogModel: public QAbstractListModel
{
    Q_OBJECT
public:
    SvnLogModel(const svn::LogEntriesMapPtr &_log, const QString &_name, QObject *parent);
    ~SvnLogModel();
    void setLogData(const svn::LogEntriesMapPtr &log, const QString &name);

    qlonglong toRevision(const QModelIndex &)const;
    const QString &fullMessage(const QModelIndex &index)const;
    void fillChangedPaths(const QModelIndex &index, QTreeWidget *target);
    const QString &realName(const QModelIndex &index);

    enum Columns {
        Author = 0,
        Revision,
        Date,
        Message,
        Count
    };

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &) const Q_DECL_OVERRIDE;

    SvnLogModelNodePtr indexNode(const QModelIndex &)const;
    int leftRow()const;
    int rightRow()const;
    void setLeftRow(int);
    void setRightRow(int);

    qlonglong min() const;
    qlonglong max() const;

private:
    QVector<SvnLogModelNodePtr> m_data;
    QString m_emptyString;
    qlonglong m_min, m_max;
    QString m_name;
    int m_left, m_right;

    friend class SvnLogSortModel;
};

class SvnLogSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SvnLogSortModel(QObject *parent = nullptr);
    ~SvnLogSortModel();

    void setSourceModel(QAbstractItemModel *sourceModel) final;
protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const final;
private:
    SvnLogModel *m_sourceModel;
};

#endif
