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
#ifndef SVNITEMNODE_H
#define SVNITEMNODE_H

#include "svnitemmodelfwd.h"

#include "svnfrontend/svnitem.h"

#include <QAbstractItemModel>
#include <QColor>

class SvnActions;
class MainTreeWidget;
class SvnItemModel;

class SvnItemModelNode: public SvnItem
{
public:
    SvnItemModelNode(SvnItemModelNodeDir *_parent, SvnActions *, MainTreeWidget *);
    virtual ~SvnItemModelNode();

    int rowNumber() const;

    SvnItemModelNodeDir *parent()const;

    /************************
     * Methods from SvnItem *
     ************************/
    virtual QString getParentDir()const;
    virtual SvnItem *getParentItem()const;
    virtual svn::Revision correctPeg()const;
    virtual void refreshStatus(bool children = false);
    virtual SvnActions *getWrapper() const;
    virtual bool NodeIsDir() const;
    virtual bool NodeHasChilds() const;

    virtual char sortChar() const;
    virtual SvnItemModelNode *sItem()
    {
        return this;
    }

    QColor backgroundColor() const;

protected:
    SvnItemModelNodeDir *_parentNode;
    SvnActions *_actions;
    MainTreeWidget *_display;
};

class SvnItemModelNodeDir: public SvnItemModelNode
{
    friend class SvnItemModel;
public:
    SvnItemModelNodeDir(SvnActions *, MainTreeWidget *);
    SvnItemModelNodeDir(SvnItemModelNodeDir *_parent, SvnActions *, MainTreeWidget *);
    virtual ~SvnItemModelNodeDir();
    virtual bool NodeIsDir() const;

    const QVector<SvnItemModelNode *> &childList()const;
    SvnItemModelNode *child(int row)const;

    SvnItemModelNode *findPath(const QStringList &parts);
    int indexOf(const QString &fullPath) const;
    virtual char sortChar() const;

    bool contains(const QString &fullName) const;
    virtual void refreshStatus(bool children = false);

    void clear();
    virtual bool NodeHasChilds() const;

protected:
    QVector<SvnItemModelNode *> m_Children;
};

#endif
