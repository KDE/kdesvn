/***************************************************************************
 *   Copyright (C) 2007 by Rajko Albrecht  ral@alwins-world.de             *
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

#include "logmodelhelper.h"
#include "svnqt/log_entry.h"

#include <KLocalizedString>

#define TREE_PATH_ITEM_TYPE QTreeWidgetItem::UserType+1

LogChangePathItem::LogChangePathItem(const svn::LogChangePathEntry &e, QTreeWidget *view)
    : QTreeWidgetItem(view, TREE_PATH_ITEM_TYPE)
{
    init(e);
}

void LogChangePathItem::init(const svn::LogChangePathEntry &e)
{
    _action = e.action;
    setText(0, QString(QLatin1Char(_action)));
    _path = e.path;
    setText(1, e.path);
    _revision = e.copyFromRevision;
    _source = e.copyFromPath;
    if (e.copyFromRevision > -1) {
        setText(2, i18n("%1 at revision %2", e.copyFromPath, e.copyFromRevision));
    }
}

SvnLogModelNode::SvnLogModelNode(const svn::LogEntry &_entry)
    : _data(_entry)
    , _realName(QString())
    , _date(svn::DateTime(_entry.date).toQDateTime())
{
    const QVector<QStringRef> sp = _entry.message.splitRef(QLatin1Char('\n'));
    if (sp.isEmpty()) {
        _shortMessage = _entry.message;
    } else {
        _shortMessage = sp.at(0).toString();
    }
}

const svn::LogChangePathEntries &SvnLogModelNode::changedPaths()const
{
    return _data.changedPaths;
}

bool SvnLogModelNode::copiedFrom(QString &_n, qlonglong &_rev)const
{
    for (const svn::LogChangePathEntry &entry : _data.changedPaths) {
        if (entry.action == 'A' &&
                !entry.copyFromPath.isEmpty() &&
                isParent(entry.path, _realName)) {
            QString r = _realName.mid(entry.path.length());
            _n = entry.copyFromPath;
            _n += r;
            _rev = entry.copyFromRevision;
            return true;
        }
    }
    return false;
}

bool SvnLogModelNode::isParent(const QString &_par, const QString &tar)
{
    if (_par == tar) {
        return true;
    }
    QString par = _par.endsWith(QLatin1Char('/')) ? _par : _par + QLatin1Char('/');
    return tar.startsWith(par);
}

void SvnLogModelNode::setChangedPaths(const svn::LogEntry &le)
{
    _data.changedPaths = le.changedPaths;
}
