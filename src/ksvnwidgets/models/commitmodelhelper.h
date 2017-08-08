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

#ifndef COMMITMODELHELPER_H
#define COMMITMODELHELPER_H

#include "svnqt/svnqttypes.h"

#include <QString>

class CommitActionEntry
{
public:
    enum ACTION_TYPE {
        COMMIT         = 1,
        ADD_COMMIT     = 2,
        DELETE         = 4,
        MISSING_DELETE = 8,

        ALL            = COMMIT | ADD_COMMIT | DELETE | MISSING_DELETE,
    };
    Q_FLAGS(ACTION_TYPE)
    Q_DECLARE_FLAGS(ActionTypes, ACTION_TYPE)

    CommitActionEntry() = default;
    CommitActionEntry(const QString &name, const QString &actiondesc, ACTION_TYPE kind = COMMIT)
        : _name(name), _actionDesc(actiondesc), _kind(kind)
    {}

    QString action() const { return _actionDesc; }
    QString name() const { return _name; }
    ACTION_TYPE type() const { return _kind; }

protected:
    QString _name;
    QString _actionDesc;
    ACTION_TYPE _kind = COMMIT;
};

class CommitModelNode
{
public:
    explicit CommitModelNode(const svn::CommitItem &);
    explicit CommitModelNode(const CommitActionEntry &, bool checked);

    ~CommitModelNode();

    void setCheckable(bool how)
    {
        m_Checkable = how;
    }
    bool checkable()const
    {
        return m_Checkable;
    }

    void setChecked(bool how)
    {
        m_Checked = how;
    }
    bool checked()const
    {
        return m_Checked;
    }

    const CommitActionEntry &actionEntry()const
    {
        return m_Content;
    }

protected:
    CommitActionEntry m_Content;
    bool m_Checkable;
    bool m_Checked;
};

#endif
