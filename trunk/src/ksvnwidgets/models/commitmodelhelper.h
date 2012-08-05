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

#include "src/svnqt/svnqttypes.h"

#include <QString>

class CommitActionEntry
{
public:
    enum ACTION_TYPE{
        COMMIT         = 1,
        ADD_COMMIT     = 2,
        DELETE         = 4,
        MISSING_DELETE = 8,

        ALL            = COMMIT|ADD_COMMIT|DELETE|MISSING_DELETE,
    };

    CommitActionEntry(const QString&,const QString&,ACTION_TYPE kind = COMMIT);
    CommitActionEntry(const CommitActionEntry&);
    CommitActionEntry();

    virtual ~CommitActionEntry();

    const QString&action()const;
    const QString&name()const;
    ACTION_TYPE type()const;

protected:
    QString _name;
    QString _actionDesc;
    ACTION_TYPE _kind;

};

class CommitModelNode
{
public:
    CommitModelNode(const svn::CommitItem&);
    CommitModelNode(const QString&,const QString&);
    CommitModelNode(const CommitActionEntry&,bool checked);

    virtual ~CommitModelNode();

    void setCheckable(bool how){m_Checkable=how;}
    bool checkable()const{return m_Checkable;}

    void setChecked(bool how){m_Checked=how;}
    bool checked()const{return m_Checked;}

    const CommitActionEntry&actionEntry()const{return m_Content;}

protected:
    CommitActionEntry m_Content;
    bool m_Checkable;
    bool m_Checked;
};

#endif
