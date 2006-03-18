/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "commititem.hpp"

#include <svn_client.h>

namespace svn {

CommitItem::CommitItem(const svn_client_commit_item_t*_item)
{
    init();
    if (_item) {
        m_Path = QString::fromUtf8(_item->path);
        m_Kind = _item->kind;
        m_Url = QString::fromUtf8(_item->url);
        if (_item->state_flags & SVN_CLIENT_COMMIT_ITEM_IS_COPY) {
            m_CopyFromRevision = _item->revision;
        } else {
            m_Revision = _item->revision;
        }
        m_CopyFromUrl = QString::fromUtf8(_item->copyfrom_url);
        m_State = _item->state_flags;
        convertprop(_item->wcprop_changes);
    }
}

CommitItem::CommitItem(const svn_client_commit_item2_t*_item)
{
    init();

    if (_item) {
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 3)
        m_Path = QString::fromUtf8(_item->path);
        m_Kind = _item->kind;
        m_Url = QString::fromUtf8(_item->url);
        m_Revision = _item->revision;
        m_CopyFromRevision = _item->copyfrom_rev;
        m_CopyFromUrl = QString::fromUtf8(_item->copyfrom_url);
        m_State = _item->state_flags;
        convertprop(_item->wcprop_changes);
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
        m_CommitProperties[QString::fromUtf8(item->name)]=QString::fromUtf8(item->value->data,item->value->len);
    }
}

void CommitItem::init()
{
    m_Path=m_Url=m_CopyFromUrl = QString::null;
    m_Kind = svn_node_unknown;
    m_Revision=m_CopyFromRevision = -1;
    m_State = 0;
    m_CommitProperties.clear();
}

CommitItem::~CommitItem()
{
}


}
