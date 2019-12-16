/***************************************************************************
 *   Copyright (C) 2008-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#include "conflictdescription.h"
#include "svnqt_defines.h"

#include <svn_wc.h>
#include "helper.h"

namespace svn
{

ConflictDescription::~ConflictDescription()
{
}

ConflictDescription::ConflictDescription(const svn_wc_conflict_description2_t *conflict)
{
    init();
    if (!conflict) {
        return;
    }
    m_baseFile = QString::fromUtf8(conflict->base_abspath);
    m_mergedFile = QString::fromUtf8(conflict->merged_file);
    m_mimeType = QString::fromUtf8(conflict->mime_type);
    m_myFile = QString::fromUtf8(conflict->my_abspath);
    m_Path = QString::fromUtf8(conflict->local_abspath);
    m_propertyName = QString::fromUtf8(conflict->property_name);
    m_theirFile = QString::fromUtf8(conflict->their_abspath);
    switch (conflict->action) {
    case svn_wc_conflict_action_edit:
        m_action = ConflictAction::Edit;
        break;
    case svn_wc_conflict_action_add:
        m_action = ConflictAction::Add;
        break;
    case svn_wc_conflict_action_delete:
        m_action = ConflictAction::Delete;
        break;
    case svn_wc_conflict_action_replace:
        m_action = ConflictAction::Replace;
        break;
    }
    switch (conflict->kind) {
    case svn_wc_conflict_kind_text:
        m_Type = ConflictType::Text;
        break;
    case svn_wc_conflict_kind_property:
        m_Type = ConflictType::Property;
        break;
    case svn_wc_conflict_kind_tree:
        m_Type = ConflictType::Tree;
        break;
    }
    m_nodeKind = conflict->node_kind;
    m_binary = conflict->is_binary;
    switch (conflict->reason) {
    case svn_wc_conflict_reason_edited:
        m_reason = ConflictReason::Edited;
        break;
    case svn_wc_conflict_reason_obstructed:
        m_reason = ConflictReason::Obstructed;
        break;
    case svn_wc_conflict_reason_deleted:
        m_reason = ConflictReason::Deleted;
        break;
    case svn_wc_conflict_reason_missing:
        m_reason = ConflictReason::Missing;
        break;
    case svn_wc_conflict_reason_unversioned:
        m_reason = ConflictReason::Unversioned;
        break;
    case svn_wc_conflict_reason_added:
        m_reason = ConflictReason::Added;
        break;
    case svn_wc_conflict_reason_replaced:
        m_reason = ConflictReason::Replaced;
        break;
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    case svn_wc_conflict_reason_moved_away:
        m_reason = ConflictReason::MovedAway;
        break;
    case svn_wc_conflict_reason_moved_here:
        m_reason = ConflictReason::MovedHere;
        break;
#endif
    }
}

ConflictDescription::ConflictDescription(const svn_wc_conflict_description_t *conflict)
    : m_pool()
{
    init();
    if (!conflict) {
        return;
    }
    m_baseFile = QString::fromUtf8(conflict->base_file);
    m_mergedFile = QString::fromUtf8(conflict->merged_file);
    m_mimeType = QString::fromUtf8(conflict->mime_type);
    m_myFile = QString::fromUtf8(conflict->my_file);
    m_Path = QString::fromUtf8(conflict->path);
    m_propertyName = QString::fromUtf8(conflict->property_name);
    m_theirFile = QString::fromUtf8(conflict->their_file);
    switch (conflict->action) {
    case svn_wc_conflict_action_edit:
        m_action = ConflictAction::Edit;
        break;
    case svn_wc_conflict_action_add:
        m_action = ConflictAction::Add;
        break;
    case svn_wc_conflict_action_delete:
        m_action = ConflictAction::Delete;
        break;
    case svn_wc_conflict_action_replace:
        m_action = ConflictAction::Replace;
        break;
    }
    switch (conflict->kind) {
    case svn_wc_conflict_kind_text:
        m_Type = ConflictType::Text;
        break;
    case svn_wc_conflict_kind_property:
        m_Type = ConflictType::Property;
        break;
    case svn_wc_conflict_kind_tree:
        m_Type = ConflictType::Tree;
        break;
    }
    m_nodeKind = conflict->node_kind;
    m_binary = conflict->is_binary;
    switch (conflict->reason) {
    case svn_wc_conflict_reason_edited:
        m_reason = ConflictReason::Edited;
        break;
    case svn_wc_conflict_reason_obstructed:
        m_reason = ConflictReason::Obstructed;
        break;
    case svn_wc_conflict_reason_deleted:
        m_reason = ConflictReason::Deleted;
        break;
    case svn_wc_conflict_reason_missing:
        m_reason = ConflictReason::Missing;
        break;
    case svn_wc_conflict_reason_unversioned:
        m_reason = ConflictReason::Unversioned;
        break;
    case svn_wc_conflict_reason_added:
        m_reason = ConflictReason::Added;
        break;
    case svn_wc_conflict_reason_replaced:
        m_reason = ConflictReason::Replaced;
        break;
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    case svn_wc_conflict_reason_moved_away:
        m_reason = ConflictReason::MovedAway;
        break;
    case svn_wc_conflict_reason_moved_here:
        m_reason = ConflictReason::MovedHere;
        break;
#endif
    }
}

}

svn::ConflictDescription::ConflictAction svn::ConflictDescription::action() const
{
    return m_action;
}

const QString &svn::ConflictDescription::baseFile() const
{
    return m_baseFile;
}


/*!
    \fn svn::ConflictDescription::init()
 */
void svn::ConflictDescription::init()
{
    m_action = ConflictAction::Edit;
    m_Type = ConflictType::Text;
    m_reason = ConflictReason::Edited;
    m_binary = false;
    m_nodeKind = svn_node_unknown;
}


bool svn::ConflictDescription::binary() const
{
    return m_binary;
}


const QString &svn::ConflictDescription::mergedFile() const
{
    return m_mergedFile;
}


const QString &svn::ConflictDescription::mimeType() const
{
    return m_mimeType;
}


const QString &svn::ConflictDescription::myFile() const
{
    return m_myFile;
}


svn_node_kind_t svn::ConflictDescription::nodeKind() const
{
    return m_nodeKind;
}


const QString &svn::ConflictDescription::Path() const
{
    return m_Path;
}


const QString &svn::ConflictDescription::propertyName() const
{
    return m_propertyName;
}


svn::ConflictDescription::ConflictReason svn::ConflictDescription::reason() const
{
    return m_reason;
}


const QString &svn::ConflictDescription::theirFile() const
{
    return m_theirFile;
}


svn::ConflictDescription::ConflictType svn::ConflictDescription::Type() const
{
    return m_Type;
}
