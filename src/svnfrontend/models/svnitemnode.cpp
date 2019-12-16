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

#include "svnitemnode.h"
#include "svnfrontend/maintreewidget.h"
#include "svnqt/revision.h"
#include "settings/kdesvnsettings.h"

SvnItemModelNode::SvnItemModelNode(SvnItemModelNodeDir *aParentNode, SvnActions *bl, MainTreeWidget *id)
    : SvnItem(), _parentNode(aParentNode), _actions(bl), _display(id)
{
}

int SvnItemModelNode::rowNumber()const
{
    if (!_parentNode) {
        return -1;
    }
    return _parentNode->childList().indexOf(const_cast<SvnItemModelNode *>(this));
}

bool SvnItemModelNode::NodeIsDir() const
{
    return false;
}

SvnItemModelNodeDir *SvnItemModelNode::parent()const
{
    return _parentNode;
}

QColor SvnItemModelNode::backgroundColor() const
{
    if (Kdesvnsettings::colored_state()) {
        switch (m_bgColor) {
        case UPDATES:
            return Kdesvnsettings::color_need_update();
        case  LOCKED:
            return Kdesvnsettings::color_locked_item();
        case  ADDED:
            return Kdesvnsettings::color_item_added();
        case  DELETED:
            return Kdesvnsettings::color_item_deleted();
        case  MODIFIED:
            return Kdesvnsettings::color_changed_item();
        case MISSING:
            return Kdesvnsettings::color_missed_item();
        case NOTVERSIONED:
            return Kdesvnsettings::color_notversioned_item();
        case CONFLICT:
            return Kdesvnsettings::color_conflicted_item();
        case NEEDLOCK:
            return Kdesvnsettings::color_need_lock();
        case NONE:
            break;
        }
    }
    return QColor();
}

bool SvnItemModelNode::NodeHasChilds() const
{
    return false;
}

/************************
 * Methods from SvnItem *
 ************************/
QString SvnItemModelNode::getParentDir()const
{
    if (parent()) {
        return parent()->fullName();
    }
    return QString();
}

SvnItem *SvnItemModelNode::getParentItem()const
{
    return _parentNode;
}

svn::Revision SvnItemModelNode::correctPeg()const
{
    /// @todo backlink to remote revision storage
    return _display->baseRevision();
}

void SvnItemModelNode::refreshStatus(bool children)
{
    _display->refreshItem(this);
    if (!children && _parentNode) {
        _parentNode->refreshStatus(false);
    }
}

SvnActions *SvnItemModelNode::getWrapper() const
{
    return _actions;
}

char SvnItemModelNode::sortChar() const
{
    return 3;
}

SvnItemModelNodeDir::SvnItemModelNodeDir(SvnActions *bl, MainTreeWidget *disp)
    : SvnItemModelNode(nullptr, bl, disp), m_Children()
{
}

SvnItemModelNodeDir::SvnItemModelNodeDir(SvnItemModelNodeDir *_parent, SvnActions *bl, MainTreeWidget *disp)
    : SvnItemModelNode(_parent, bl, disp), m_Children()
{
}

SvnItemModelNodeDir::~SvnItemModelNodeDir()
{
    clear();
}

void SvnItemModelNodeDir::clear()
{
    qDeleteAll(m_Children);
    m_Children.clear();
}

const QVector<SvnItemModelNode *> &SvnItemModelNodeDir::childList()const
{
    return m_Children;
}

bool SvnItemModelNodeDir::NodeIsDir() const
{
    if (isValid()) {
        return isDir();
    }
    return true;
}

SvnItemModelNode *SvnItemModelNodeDir::child(int row)const
{
    if (row < 0 || row >= m_Children.size()) {
        return nullptr;
    }
    return m_Children[row];
}

char SvnItemModelNodeDir::sortChar() const
{
    return 1;
}

SvnItemModelNode *SvnItemModelNodeDir::findPath(const QVector<QStringRef> &parts)
{
    for (auto &child : m_Children) {
        if (child->shortName() == parts[0]) {
            if (parts.size() == 1) {
                return child;
            } else if (child->isDir()) {
                return static_cast<SvnItemModelNodeDir *>(child)->findPath(parts.mid(1));
            }
        }
    }
    return nullptr;
}

bool SvnItemModelNodeDir::contains(const QString &fullName) const
{
    return indexOf(fullName) != -1;
}

int SvnItemModelNodeDir::indexOf(const QString &fullPath) const
{
    for (int i = 0; i < m_Children.size(); ++i) {
        if (m_Children[i]->fullName() == fullPath) {
            return i;
        }
    }
    return -1;
}

void SvnItemModelNodeDir::refreshStatus(bool children)
{
    SvnItemModelNode::refreshStatus(children);
    if (!isValid()) {
        return;
    }
    if (children) {
        for (auto &child : m_Children) {
            child->refreshStatus(children);
        }
    }
}

bool SvnItemModelNodeDir::NodeHasChilds() const
{
    return !isIgnored();
}
