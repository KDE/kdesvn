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
#include "conflictdescription.hpp"
#include "svnqt_defines.hpp"

#include <svn_wc.h>

namespace svn {

ConflictDescription::ConflictDescription()
    :m_pool()
{
    init();
}


ConflictDescription::~ConflictDescription()
{
}

ConflictDescription::ConflictDescription(const svn_wc_conflict_description_t*conflict)
    :m_pool()
{
    init();
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    if (!conflict) {
        return;
    }
    m_baseFile=QString::FROMUTF8(conflict->base_file);
    m_mergedFile=QString::FROMUTF8(conflict->merged_file);
    m_mimeType=QString::FROMUTF8(conflict->mime_type);
    m_myFile=QString::FROMUTF8(conflict->my_file);
    m_Path=QString::FROMUTF8(conflict->path);
    m_propertyName=QString::FROMUTF8(conflict->property_name);
    m_theirFile=QString::FROMUTF8(conflict->their_file);
    switch(conflict->action) {
        case svn_wc_conflict_action_edit:
            m_action=ConflictEdit;
            break;
        case svn_wc_conflict_action_add:
            m_action=ConflictAdd;
            break;
        case svn_wc_conflict_action_delete:
            m_action=ConflictDelete;
            break;
    }
    switch (conflict->kind) {
        case svn_wc_conflict_kind_text:
            m_Type=ConflictText;
            break;
        case svn_wc_conflict_kind_property:
            m_Type=ConflictProperty;
            break;
    }
    m_nodeKind=conflict->node_kind;
    m_binary=conflict->is_binary;
    switch (conflict->reason) {
        case svn_wc_conflict_reason_edited:
            m_reason=ReasonEdited;
            break;
        case svn_wc_conflict_reason_obstructed:
            m_reason=ReasonObstructed;
            break;
        case svn_wc_conflict_reason_deleted:
            m_reason=ReasonDeleted;
            break;
        case svn_wc_conflict_reason_missing:
            m_reason=ReasonMissing;
            break;
        case svn_wc_conflict_reason_unversioned:
            m_reason=ReasonUnversioned;
            break;
    }
#else
    Q_UNUSED(conflict);
#endif
}

ConflictDescription::ConflictDescription(const ConflictDescription&)
    :m_pool()
{
}

}

svn::ConflictDescription::ConflictAction svn::ConflictDescription::action() const
{
    return m_action;
}

const QString&svn::ConflictDescription::baseFile() const
{
    return m_baseFile;
}


/*!
    \fn svn::ConflictDescription::init()
 */
void svn::ConflictDescription::init()
{
    m_action=ConflictEdit;
    m_Type=ConflictText;
    m_reason=ReasonEdited;
    m_binary=false;
    m_nodeKind = svn_node_unknown;
}


bool svn::ConflictDescription::binary() const
{
    return m_binary;
}


const QString& svn::ConflictDescription::mergedFile() const
{
    return m_mergedFile;
}


const QString& svn::ConflictDescription::mimeType() const
{
    return m_mimeType;
}


const QString& svn::ConflictDescription::myFile() const
{
    return m_myFile;
}


svn_node_kind_t svn::ConflictDescription::nodeKind() const
{
    return m_nodeKind;
}


const QString& svn::ConflictDescription::Path() const
{
    return m_Path;
}


const QString& svn::ConflictDescription::propertyName() const
{
    return m_propertyName;
}


svn::ConflictDescription::ConflictReason svn::ConflictDescription::reason() const
{
    return m_reason;
}


const QString& svn::ConflictDescription::theirFile() const
{
    return m_theirFile;
}


svn::ConflictDescription::ConflictType svn::ConflictDescription::Type() const
{
    return m_Type;
}
