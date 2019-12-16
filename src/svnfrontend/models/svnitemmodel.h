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
#ifndef SVNITEMMODEL_H
#define SVNITEMMODEL_H

#include <QAbstractItemModel>
#include <QScopedPointer>

#include "svnitemmodelfwd.h"
#include "svnqt/svnqttypes.h"


class SvnItemModelData;
class QItemSelectionModel;
class MainTreeWidget;
class SvnActions;
class QMimeData;

namespace svn
{
class Path;
}

#define SORT_ROLE Qt::UserRole+1
#define FILTER_ROLE Qt::UserRole+2
#define BG_ROLE Qt::UserRole+3

class SvnItemModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SvnItemModel(MainTreeWidget *display, QObject *parent = nullptr);
    ~SvnItemModel();

    void clear();

    enum Column {
        Name = 0,
        Status,
        LastRevision,
        LastAuthor,
        LastDate,
        Locked,
        ColumnCount
    };

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex())const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)const override;
    int columnCount(const QModelIndex &parent = QModelIndex())const override;
    int rowCount(const QModelIndex &parent = QModelIndex())const override;
    QModelIndex parent(const QModelIndex &index)const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex())const override;
    bool canFetchMore(const QModelIndex &parent)const override;
    void fetchMore(const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index)const override;

    //! Returns the very first item in list.
    /*!
     * This item marks the working copy itself when a working copy is opened.
     * When opened a repository it is just an entry.
     */
    SvnItemModelNode *firstRootChild();
    SvnItemModelNode *nodeForIndex(const QModelIndex &index);
    QModelIndex firstRootIndex();
    void setRootNodeStat(const svn::StatusPtr &);

    SvnActions *svnWrapper();

    int checkDirs(const QString &_what, SvnItemModelNode *parent);
    Qt::DropActions supportedDropActions()const override;
    QStringList mimeTypes()const override;
    QMimeData *mimeData(const QModelIndexList &indexes)const override;

    bool dropUrls(const QList<QUrl> &data, Qt::DropAction action, int row, int column, const QModelIndex &parent, bool intern);

    bool filterIndex(const QModelIndex &, int, svnmodel::ItemTypeFlag)const;

    /* svn actions starts here */
    void makeIgnore(const QModelIndex &);

    //! looks if \a aPath exists in tree
    /*! Looks always for perfect match,
     * \return node of matched item or 0
     */
    SvnItemModelNode *findPath(const svn::Path &aPath);
    //! looks if \a aPath exists in tree
    /*! Looks always for perfect match,
     * \return QModelIndex of matched item or invalid QModelIndex
     */
    QModelIndex findIndex(const svn::Path &aPath);
    void initDirWatch();
    bool refreshCurrentTree();
    bool refreshDirnode(SvnItemModelNodeDir *, bool check_empty = false, bool notrec = false);
    bool refreshItem(SvnItemModelNode *);
    bool refreshIndex(const QModelIndex &idx);
    void emitDataChangedRow(const QModelIndex &idx);

    void clearNodeDir(SvnItemModelNodeDir *);

    const QString &uniqueIdentifier()const;

Q_SIGNALS:
    void urlDropped(const QList<QUrl> &, Qt::DropAction, const QModelIndex &, bool);
    void clientException(const QString &);
    void itemsFetched(const QModelIndex &);

protected:
    /* the parent entry must removed from list before */
    void insertDirs(SvnItemModelNode *_parent, svn::StatusEntries &);
    //! \a ind must be a directory index
    void checkAddNewItems(const QModelIndex &ind);
    bool checkRootNode();
    int checkUnversionedDirs(SvnItemModelNode *_parent);
    void beginRemoveRows(const QModelIndex &parent, int first, int last);

public Q_SLOTS:
    virtual void slotNotifyMessage(const QString &);

protected Q_SLOTS:
    void slotCreated(const QString &);
    void slotDeleted(const QString &);
    void slotDirty(const QString &);

private:
    friend class SvnItemModelData;
    QScopedPointer<SvnItemModelData> m_Data;
    bool insertRows(int , int, const QModelIndex & = QModelIndex()) override;
    bool insertColumns(int, int, const QModelIndex & = QModelIndex()) override;
    bool removeRows(int, int, const QModelIndex & = QModelIndex()) override;
    bool removeColumns(int, int, const QModelIndex & = QModelIndex()) override;
};

#endif
