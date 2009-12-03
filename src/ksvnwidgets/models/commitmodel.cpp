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

#include "src/svnqt/commititem.h"

#include <klocale.h>

class CommitModelData
{
public:
    CommitModelData(){}
    virtual ~CommitModelData(){}

    int completeCount()const{return m_List.size()+m_hiddenList.size();}
    CommitModelNodeList m_List;
    CommitModelNodeList m_hiddenList;
};

CommitModel::CommitModel(const svn::CommitItemList&aList,QObject*parent)
    :QAbstractItemModel(parent),m_Content(new CommitModelData)
{
    setCommitData(aList);
}

/*********************
 * Begin CommitModel *
 *********************/
CommitModel::CommitModel(const QMap<QString,QString>&aList,QObject*parent)
    :QAbstractItemModel(parent),m_Content(new CommitModelData)
{
    setCommitData(aList);
}

CommitModel::CommitModel(const CommitActionEntries&_checked,const CommitActionEntries&_notchecked,QObject*parent)
    :QAbstractItemModel(parent),m_Content(new CommitModelData)
{
    setCommitData(_checked,_notchecked);
}

CommitModel::~CommitModel()
{
}

void CommitModel::setCommitData(const svn::CommitItemList&aList)
{
    beginRemoveRows(QModelIndex(),0,m_Content->m_List.count());
    m_Content->m_List.clear();
    endRemoveRows();
    beginInsertRows(QModelIndex(),0,aList.size());
    int j;
    for (j=0;j<aList.size();++j) {
        m_Content->m_List.append(new CommitModelNode(aList[j]));
    }
    endInsertRows();
}

void CommitModel::setCommitData(const QMap<QString,QString>&aList)
{
    beginRemoveRows(QModelIndex(),0,m_Content->m_List.count());
    m_Content->m_List.clear();
    endRemoveRows();
    beginInsertRows(QModelIndex(),0,aList.size());
    QMap<QString,QString>::ConstIterator it = aList.begin();
    for (;it!=aList.end();++it) {
        m_Content->m_List.append(new CommitModelNode(it.key(),it.value()));
    }
    endInsertRows();
}

void CommitModel::setCommitData(const CommitActionEntries&checked,const CommitActionEntries&notchecked)
{
    beginRemoveRows(QModelIndex(),0,m_Content->m_List.count());
    m_Content->m_List.clear();
    endRemoveRows();

    beginInsertRows(QModelIndex(),0,checked.size()+notchecked.size());
    int j;
    for (j=0;j<checked.size();++j) {
        m_Content->m_List.append(new CommitModelNode(checked[j],true));
    }
    for (j=0;j<notchecked.size();++j) {
        m_Content->m_List.append(new CommitModelNode(notchecked[j],false));
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

CommitModelNodePtr CommitModel::node(const QModelIndex&index)
{
    if (!index.isValid()||index.row()>=m_Content->m_List.count()) {
        return CommitModelNodePtr();
    }
    return m_Content->m_List[index.row()];
}

CommitActionEntries CommitModel::checkedEntries()const
{
    int i;
    CommitActionEntries res;
    for (i=0;i<m_Content->m_List.count();++i) {
        if (m_Content->m_List[i]->checked()) {
            res.append(m_Content->m_List[i]->actionEntry());
        }
    }
    for (i=0;i<m_Content->m_hiddenList.count();++i) {
        if (m_Content->m_hiddenList[i]->checked()) {
            res.append(m_Content->m_hiddenList[i]->actionEntry());
        }
    }
    return res;
}

void CommitModel::markItems(bool mark,CommitActionEntry::ACTION_TYPE _type)
{
    QModelIndex _index;
    QVariant v = mark?int(2):int(0);
    for (int i=0;i<m_Content->m_List.count();++i) {
        if (m_Content->m_List[i]->actionEntry().type() & _type) {
            _index=index(i,0,QModelIndex());
            setData(_index,v,Qt::CheckStateRole);
        }
    }
}

void CommitModel::hideItems(bool hide,CommitActionEntry::ACTION_TYPE _type)
{
    int i;
    QModelIndex _index;
    if (hide) {
        QVariant v=0;
        for (i=0;i<m_Content->m_List.count();++i) {
            if (m_Content->m_List[i]->actionEntry().type()==_type) {
                _index=index(i,0,QModelIndex());
                setData(_index,v,Qt::CheckStateRole);
                m_Content->m_hiddenList.append(m_Content->m_List[i]);
                beginRemoveRows(QModelIndex(),i,i);
                m_Content->m_List.removeAt(i);
                endRemoveRows();
                // Important!
                i=0;
            }
        }
    } else {
        for (i=0;i<m_Content->m_hiddenList.count();++i) {
            if (m_Content->m_hiddenList[i]->actionEntry().type()==_type) {
                beginInsertRows(QModelIndex(),0,0);
                m_Content->m_List.prepend(m_Content->m_hiddenList[i]);
                m_Content->m_hiddenList.removeAt(i);
                endInsertRows();
                // Important!
                i=0;
            }
        }
    }
}


/*!
    \fn CommitModel::removeEntries(const QStringList&)
 */
void CommitModel::removeEntries(const QStringList&items)
{
    QModelIndex _index;
    for (int i = 0; i<items.size();++i) {
        for (int j=0;j<m_Content->m_List.count();++j) {
            if (m_Content->m_List[j]->actionEntry().name() == items[i]) {
                beginRemoveRows(QModelIndex(),j,j);
                m_Content->m_List.removeAt(j);
                endRemoveRows();
                // Important!
                j=0;
            }
        }
    }
}

/************************************
 * begin overload of Model methods  *
 ************************************/
QModelIndex CommitModel::index(int row,int column,const QModelIndex & /*parent*/)const
{
    if (row >= m_Content->m_List.count()) {
        return QModelIndex();
    }
    CommitModelNode*n=m_Content->m_List[row];
    return createIndex(row,column,n);
}

QModelIndex CommitModel::parent(const QModelIndex&)const
{
    // we have no tree...
    return QModelIndex();
}

QVariant CommitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()||index.row()>=m_Content->m_List.count()) {
        return QVariant();
    }
    const CommitModelNodePtr & _l = m_Content->m_List[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        if (index.column()==ActionColumn()) {
            return _l->actionEntry().action();
        } else if (index.column()==ItemColumn()) {
            return _l->actionEntry().name();
        }
    }
    return QVariant();
}

QVariant CommitModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    Q_UNUSED(orientation);
    switch (role) {
    case Qt::DisplayRole:
        if (section==ActionColumn()) {
            return i18n("Action");
        } else if (section==ItemColumn()) {
            return i18n("Entry");
        }
    }
    return QVariant();
}

int CommitModel::rowCount(const QModelIndex&) const
{
    return m_Content->m_List.count();
}

int CommitModel::columnCount(const QModelIndex&) const
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
CommitModelCheckitem::CommitModelCheckitem(const CommitActionEntries&_checked,const CommitActionEntries&_notchecked,QObject*parent)
    :CommitModel(_checked,_notchecked,parent)
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

Qt::ItemFlags CommitModelCheckitem::flags(const QModelIndex & index) const
{
    if (index.isValid()) {
        if (index.column()==0) {
            return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
        } else {
            return CommitModel::flags(index);
        }
    }
    return 0;
}

QVariant CommitModelCheckitem::data(const QModelIndex &index, int role) const
{
    if (index.column()!=0 || role!=Qt::CheckStateRole||!index.isValid()||index.row()>=m_Content->m_List.count()) {
        return CommitModel::data(index,role);
    }
    if (m_Content->m_List[index.row()]->checked()) return Qt::Checked;
    return Qt::Unchecked;
}

bool CommitModelCheckitem::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role!=Qt::CheckStateRole||!index.isValid()||index.row()>=m_Content->m_List.count()||index.column()!=0) {
        return CommitModel::setData(index,value,role);
    }
    if (value.type()==QVariant::Int) {
        CommitModelNodePtr _l = m_Content->m_List[index.row()];
        bool old = _l->checked();
        bool nv = value.toInt()>0;
        _l->setChecked(nv);
        if (old!=nv) {
            emit dataChanged(index,index);
        }
        return old!=nv;
    }
    return false;
}
/************************************
 * end CommitModelCheckitem         *
 ************************************/

#include "commitmodel.moc"
