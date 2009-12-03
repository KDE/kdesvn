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

#include "logmodelhelper.h"
#include "src/svnqt/log_entry.h"

#include <klocale.h>
#include <kdebug.h>

#include <QList>

#define TREE_PATH_ITEM_TYPE QTreeWidgetItem::UserType+1

LogChangePathItem::LogChangePathItem(QTreeWidget*parent,const svn::LogChangePathEntry&e)
    :QTreeWidgetItem(parent,TREE_PATH_ITEM_TYPE)
{
    init(e);
}

LogChangePathItem::LogChangePathItem(const svn::LogChangePathEntry&e)
    :QTreeWidgetItem(TREE_PATH_ITEM_TYPE)
{
    init(e);
}

void LogChangePathItem::init(const svn::LogChangePathEntry&e)
{
    _action = QChar(e.action);
    setText(0,_action);
    _path = e.path;
    setText(1,e.path);
    _revision = e.copyFromRevision;
    _source = e.copyFromPath;
    if (e.copyFromRevision>-1) {
        setText(2,i18n("%1 at revision %2",e.copyFromPath,e.copyFromRevision));
    }
}

SvnLogModelNode::SvnLogModelNode(const svn::LogEntry&_entry)
    :_data(_entry),_realName(QString())
{
    _date = svn::DateTime(_entry.date);
    QStringList sp = _entry.message.split('\n');
    if (sp.count()==0) {
        _shortMessage=_entry.message;
    } else {
        _shortMessage=sp[0];
    }
}

const QList<svn::LogChangePathEntry>& SvnLogModelNode::changedPaths()const
{
    return _data.changedPaths;
}

bool SvnLogModelNode::copiedFrom(QString&_n,long&_rev)const
{
    for (int i = 0; i < changedPaths().count();++i) {
        if (changedPaths()[i].action=='A' &&
            !changedPaths()[i].copyFromPath.isEmpty() &&
            isParent(changedPaths()[i].path,_realName)) {
            QString tmpPath = _realName;
            QString r = _realName.mid(changedPaths()[i].path.length());
            _n=changedPaths()[i].copyFromPath;
            _n+=r;
            _rev = changedPaths()[i].copyFromRevision;
            return true;
        }
    }
    return false;
}

bool SvnLogModelNode::isParent(const QString&_par,const QString&tar)
{
    if (_par==tar) return true;
    QString par = _par.endsWith('/')?_par:_par+'/';
    return tar.startsWith(par);
}

void SvnLogModelNode::setChangedPaths(const svn::LogEntry&le)
{
    _data.changedPaths = le.changedPaths;
}
