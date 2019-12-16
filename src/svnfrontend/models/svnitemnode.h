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
#ifndef SVNITEMNODE_H
#define SVNITEMNODE_H

#include "svnitemmodelfwd.h"

#include "svnfrontend/svnitem.h"

#include <QColor>

class SvnActions;
class MainTreeWidget;
class SvnItemModel;

class SvnItemModelNode : public SvnItem
{
public:
    SvnItemModelNode(SvnItemModelNodeDir *_parent, SvnActions *, MainTreeWidget *);

    int rowNumber() const;

    SvnItemModelNodeDir *parent()const;

    /************************
     * Methods from SvnItem *
     ************************/
    QString getParentDir() const override;
    SvnItem *getParentItem() const override;
    svn::Revision correctPeg() const override;
    void refreshStatus(bool children = false) override;
    SvnActions *getWrapper() const override;
    SvnItemModelNode *sItem() override
    {
        return this;
    }

    // own
    virtual bool NodeIsDir() const;
    virtual bool NodeHasChilds() const;
    virtual char sortChar() const;

    QColor backgroundColor() const;

protected:
    SvnItemModelNodeDir *_parentNode;
    SvnActions *_actions;
    MainTreeWidget *_display;
};

class SvnItemModelNodeDir final : public SvnItemModelNode
{
    friend class SvnItemModel;
public:
    SvnItemModelNodeDir(SvnActions *, MainTreeWidget *);
    SvnItemModelNodeDir(SvnItemModelNodeDir *_parent, SvnActions *, MainTreeWidget *);
    virtual ~SvnItemModelNodeDir();

    // SvnItemModelNode
    bool NodeIsDir() const override;
    bool NodeHasChilds() const override;
    char sortChar() const override;

    const QVector<SvnItemModelNode *> &childList()const;
    SvnItemModelNode *child(int row)const;

    SvnItemModelNode *findPath(const QVector<QStringRef> &parts);
    int indexOf(const QString &fullPath) const;

    bool contains(const QString &fullName) const;
    void clear();

    // SvnItem
    void refreshStatus(bool children = false) override;

protected:
    QVector<SvnItemModelNode *> m_Children;
};

#endif
