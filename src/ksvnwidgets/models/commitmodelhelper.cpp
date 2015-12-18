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
#include "commitmodelhelper.h"
#include "src/svnqt/commititem.h"

#include <klocale.h>

CommitActionEntry::CommitActionEntry(const QString &name, const QString &actiondesc, ACTION_TYPE kind)
    : _name(name), _actionDesc(actiondesc), _kind(kind)
{
}

CommitActionEntry::CommitActionEntry()
    : _name(), _actionDesc(), _kind(COMMIT)
{
}

CommitActionEntry::CommitActionEntry(const CommitActionEntry &src)
    : _name(src._name), _actionDesc(src._actionDesc), _kind(src._kind)
{
}

CommitActionEntry::~CommitActionEntry()
{
}

const QString &CommitActionEntry::action()const
{
    return _actionDesc;
}

const QString &CommitActionEntry::name()const
{
    return _name;
}

CommitActionEntry::ACTION_TYPE CommitActionEntry::type()const
{
    return _kind;
}

CommitModelNode::CommitModelNode(const svn::CommitItem &aItem)
    : m_Content(), m_Checkable(false), m_Checked(false)
{
    QString what;
    QString action;
    switch (aItem.actionType()) {
    case 'A':
    case 'a':
        action = i18n("Add");
        break;
    case 'C':
    case 'c':
        action = i18n("Copy");
        break;
    case 'D':
    case 'd':
        action = i18n("Delete");
        break;
    case 'M':
    case 'm':
        action = i18n("Modify (content or property)");
        break;
    case 'R':
    case 'r':
        action = i18n("Replace");
        break;
    case 'L':
    case 'l':
        action = i18n("(Un)Lock");
        break;
    }
    if (aItem.path().isEmpty()) {
        what = aItem.url();
    } else {
        what = aItem.path();
    }
    m_Content = CommitActionEntry(what, action);
}

CommitModelNode::CommitModelNode(const CommitActionEntry &aContent, bool checked)
    : m_Content(aContent), m_Checkable(true), m_Checked(checked)
{
}

CommitModelNode::~CommitModelNode()
{
}
