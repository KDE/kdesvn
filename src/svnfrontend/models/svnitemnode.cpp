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

#include "svnitemnode.h"
#include "src/svnfrontend/maintreewidget.h"
#include "src/svnqt/revision.hpp"
#include "src/settings/kdesvnsettings.h"

#include <kdebug.h>

SvnItemModelNode::SvnItemModelNode(SvnItemModelNodeDir*aParentNode,SvnActions*bl,MainTreeWidget*id)
    :SvnItem(),_parentNode(aParentNode),_actions(bl),_display(id)
{
}

SvnItemModelNode::~SvnItemModelNode()
{
    _parentNode=0;
}

int SvnItemModelNode::rowNumber()const
{
    if (!_parentNode) {
        return 0;
    }
    return _parentNode->childList().indexOf(const_cast<SvnItemModelNode*>(this));
}

bool SvnItemModelNode::NodeIsDir()
{
    return false;
}

SvnItemModelNodeDir*SvnItemModelNode::parent()const
{
    return _parentNode;
}

QColor SvnItemModelNode::backgroundColor()
{
    if (Kdesvnsettings::colored_state()&&m_bgColor!=NONE) {
        switch(m_bgColor) {
            case UPDATES:
                return Kdesvnsettings::color_need_update();
                break;
            case  LOCKED:
                return Kdesvnsettings::color_locked_item();
                break;
            case  ADDED:
                return Kdesvnsettings::color_item_added();
                break;
            case  DELETED:
                return Kdesvnsettings::color_item_deleted();
                break;
            case  MODIFIED:
                return Kdesvnsettings::color_changed_item();
                break;
            case MISSING:
                return Kdesvnsettings::color_missed_item();
                break;
            case NOTVERSIONED:
                return Kdesvnsettings::color_notversioned_item();
                break;
            case CONFLICT:
                return Kdesvnsettings::color_conflicted_item();
                break;
            case NEEDLOCK:
                return Kdesvnsettings::color_need_lock();
                break;
            default:
                break;
        }
    }
    return QColor();
}

bool SvnItemModelNode::NodeHasChilds()
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

SvnItem*SvnItemModelNode::getParentItem()const
{
    return _parentNode;
}

const svn::Revision& SvnItemModelNode::correctPeg()const
{
    /// @todo backlink to remote revision storage
    return _display->baseRevision();
}

void SvnItemModelNode::refreshStatus(bool childs,const QList<SvnItem*>&exclude, bool depsonly)
{
    if (!depsonly) {
        _display->refreshItem(this);
    }
    if (!childs) {
        _parentNode->refreshStatus(false,exclude,depsonly);
    }
}

SvnActions* SvnItemModelNode::getWrapper() const
{
    return _actions;
}

char SvnItemModelNode::sortChar()
{
    return 3;
}

SvnItemModelNodeDir::SvnItemModelNodeDir(SvnActions*bl,MainTreeWidget*disp)
    :SvnItemModelNode(0,bl,disp),m_Childs()
{
}

SvnItemModelNodeDir::SvnItemModelNodeDir(SvnItemModelNodeDir*_parent,SvnActions*bl,MainTreeWidget*disp)
    :SvnItemModelNode(_parent,bl,disp),m_Childs()
{
}

SvnItemModelNodeDir::~SvnItemModelNodeDir()
{
    clear();
}

void SvnItemModelNodeDir::clear()
{
    qDeleteAll(m_Childs);
    m_Childs.clear();
}

const QList<SvnItemModelNode*>& SvnItemModelNodeDir::childList()const
{
    return m_Childs;
}

bool SvnItemModelNodeDir::NodeIsDir()
{
    if (isValid()) {
        return isDir();
    }
    return true;
}

SvnItemModelNode* SvnItemModelNodeDir::child(int row)const
{
    if (row<0) {
        return 0;
    }
    if (row>=m_Childs.size()) {
        return 0;
    }
    return m_Childs[row];
}

char SvnItemModelNodeDir::sortChar()
{
    return 1;
}

SvnItemModelNode* SvnItemModelNodeDir::findPath(const QStringList&parts)
{
    for (int i=0;i<m_Childs.size();++i) {
        if (m_Childs[i]->shortName()==parts[0]) {
            if (parts.size()==1) {
                return m_Childs[i];
            } else if (m_Childs[i]->isDir()){
                QStringList np = parts;
                np.removeFirst();
                return static_cast<SvnItemModelNodeDir*>(m_Childs[i])->findPath(np);
            }
        }
    }
    return 0;
}

bool SvnItemModelNodeDir::contains(const QString&fullName)
{
    return indexOf(fullName)!=-1;
}

int SvnItemModelNodeDir::indexOf(const QString&fullPath)
{
    for (int i=0;i<m_Childs.size();++i) {
        if (m_Childs[i]->fullName()==fullPath) {
            return i;
        }
    }
    return -1;
}

void SvnItemModelNodeDir::refreshStatus(bool childs,const QList<SvnItem*>&exclude, bool depsonly)
{
    SvnItemModelNode::refreshStatus(childs,exclude,depsonly);
    if (!isValid()) {
        return;
    }
    if (childs) {
        for (int i=0;i<m_Childs.size();++i) {
            m_Childs[i]->refreshStatus(childs,exclude,depsonly);
        }
    }
}

bool SvnItemModelNodeDir::NodeHasChilds()
{
    return !isIgnored();
}
