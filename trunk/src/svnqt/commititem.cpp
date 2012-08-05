/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "commititem.h"

#include <svn_client.h>
#include <svn_props.h>

namespace svn {

CommitItem::CommitItem(const svn_client_commit_item_t*_item)
{
    init();
    if (_item) {
        m_Path = QString::FROMUTF8(_item->path);
        m_Kind = _item->kind;
        m_Url = QString::FROMUTF8(_item->url);
        if (_item->state_flags & SVN_CLIENT_COMMIT_ITEM_IS_COPY) {
            m_CopyFromRevision = _item->revision;
        } else {
            m_Revision = _item->revision;
        }
        m_CopyFromUrl = QString::FROMUTF8(_item->copyfrom_url);
        m_State = _item->state_flags;
        convertprop(_item->wcprop_changes);
    }
}

CommitItem::CommitItem(const svn_client_commit_item2_t*_item)
{
    init();

    if (_item) {
        m_Path = QString::FROMUTF8(_item->path);
        m_Kind = _item->kind;
        m_Url = QString::FROMUTF8(_item->url);
        m_Revision = _item->revision;
        m_CopyFromRevision = _item->copyfrom_rev;
        m_CopyFromUrl = QString::FROMUTF8(_item->copyfrom_url);
        m_State = _item->state_flags;
        convertprop(_item->wcprop_changes);
    }
}

CommitItem::CommitItem(const svn_client_commit_item3_t*_item)
{
    init();

    if (_item) {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        m_Path = QString::FROMUTF8(_item->path);
        m_Kind = _item->kind;
        m_Url = QString::FROMUTF8(_item->url);
        m_Revision = _item->revision;
        m_CopyFromRevision = _item->copyfrom_rev;
        m_CopyFromUrl = QString::FROMUTF8(_item->copyfrom_url);
        m_State = _item->state_flags;
        convertprop(_item->incoming_prop_changes);
        if (_item->outgoing_prop_changes)
        {
            convertprop(_item->outgoing_prop_changes);
        }
#endif
    }
}

void CommitItem::convertprop(apr_array_header_t * list)
{
    if (!list) {
        m_CommitProperties.clear();
        return;
    }
    for (int j = 0; j < list->nelts; ++j) {
        svn_prop_t * item = ((svn_prop_t **)list->elts)[j];
        if (!item) continue;
        m_CommitProperties[QString::FROMUTF8(item->name)]=QString::FROMUTF8(item->value->data,item->value->len);
    }
}

void CommitItem::init()
{
    m_Kind = svn_node_unknown;
    m_Revision=m_CopyFromRevision = -1;
    m_State = 0;
    m_CommitProperties.clear();
}

CommitItem::~CommitItem()
{
}

const QString& CommitItem::path()const
{
    return m_Path;
}

const QString& CommitItem::url()const
{
    return m_Url;
}

const QString& CommitItem::copyfromurl()const
{
    return m_CopyFromUrl;
}

const PropertiesMap& CommitItem::properties()const
{
    return m_CommitProperties;
}

svn_revnum_t CommitItem::revision()const
{
    return m_Revision;
}

svn_revnum_t CommitItem::copyfromrevision()const
{
    return m_CopyFromRevision;
}

svn_node_kind_t CommitItem::kind()const
{
    return m_Kind;
}

apr_byte_t CommitItem::state()const
{
    return m_State;
}

char CommitItem::actionType()const
{
    char r=0;
    if (m_State & SVN_CLIENT_COMMIT_ITEM_IS_COPY) {
        r = 'C';
    } else if (m_State & SVN_CLIENT_COMMIT_ITEM_ADD){
        r = 'A';
    } else if (m_State & SVN_CLIENT_COMMIT_ITEM_DELETE){
        r = 'D';
    } else if (m_State & SVN_CLIENT_COMMIT_ITEM_PROP_MODS ||
        m_State & SVN_CLIENT_COMMIT_ITEM_TEXT_MODS){
        r = 'M';
    } else if (m_State & SVN_CLIENT_COMMIT_ITEM_LOCK_TOKEN){
        r = 'L';
    }
    return r;
}

}
