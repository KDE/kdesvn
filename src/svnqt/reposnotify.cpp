/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Rajko Albrecht <ral@alwins-world.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "svnqt/reposnotify.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/revision.h"
#include "svnqt/path.h"

#include <svn_version.h>
#include <svn_props.h>

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR < 7))

typedef struct svn_repos_notify_t
{
}

#else
#include <svn_repos.h>
#endif 

namespace svn {
namespace repository {
    
class ReposNotifyData
{
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7) || SVN_VER_MAJOR>1)
    /// TODO own datatype
    svn_repos_notify_action_t _action;
    svn::Revision _rev;
    QString _warning_msg;
    /// TODO own datatype
    svn_repos_notify_warning_t _warning;
    qlonglong _shard;

    svn::Revision _oldrev;
    svn::Revision _newrev;
    
    /// TODO own datatype
    svn_node_action _node_action;
    
    svn::Path _path;

#endif
    
    mutable QString _msg;
    
public:
    ReposNotifyData(const svn_repos_notify_t* notify)
        : _warning_msg(QString()),_msg(QString())
    {
        if (!notify) 
        {
            return;
        }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7) || SVN_VER_MAJOR>1)
        _action = notify->action;
        _rev = notify->revision;
        if (notify->warning_str)
        {
            _warning_msg=QString::FROMUTF8(notify->warning_str);
        }
        _warning = notify->warning;
        _shard = notify->shard;
        _oldrev = notify->old_revision;
        _newrev = notify->new_revision;
        _node_action = notify->node_action;
        if (notify->path != 0L) {
            _path = svn::Path(notify->path);
        }
#endif
    }
    
    ~ReposNotifyData()
    {
    }
    
    const QString&toString()const
    {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7) || SVN_VER_MAJOR>1)
        if (_msg.length()==0)
        {
            switch (_action) {
                case svn_repos_notify_warning:
                {
                    switch (_warning)
                    {
                    case svn_repos_notify_warning_found_old_reference:
                        _msg = "Old Reference: ";
                        break;
                    case svn_repos_notify_warning_found_old_mergeinfo:
                        _msg = "Old mergeinfo found: ";
                        break;
                    case svn_repos_notify_warning_invalid_fspath:
                        _msg = "Invalid path: ";
                        break;
                    default:
                        _msg = "";
                    }
                    _msg += _warning_msg;
                }
                break;
                case svn_repos_notify_dump_rev_end:
                case svn_repos_notify_verify_rev_end:
                {
                    _msg = QString("Revision ").append(_rev.toString()).append(QString(" finished."));
                }
                break;
                case svn_repos_notify_dump_end:
                {
                    _msg = QString("Dump finished");
                }
                break;
                case svn_repos_notify_verify_end:
                {
                    _msg = QString("Verification finished");
                }
                break;
                case svn_repos_notify_pack_shard_start:
                {
                    _msg = QString("Packing revisions in shard %ul").arg(_shard);
                }
                break;
                case svn_repos_notify_pack_shard_end_revprop:
                case svn_repos_notify_pack_shard_end:
                case svn_repos_notify_load_node_done:
                {
                    _msg = QString("Done");
                }
                break;
                case svn_repos_notify_pack_shard_start_revprop:
                {
                    _msg = QString("Packing revsion properties in shard %ul").arg(_shard);
                }
                break;
                case svn_repos_notify_load_txn_start:
                {
                    _msg = QString("Start loading old revision ").append(_oldrev.toString());
                }
                break;
                case svn_repos_notify_load_txn_committed:
                {
                    _msg = QString("Commited new revision ").append(_newrev.toString());
                    if (_oldrev.isValid()) 
                    {
                        _msg.append(" loaded from orignal revision ").append(_oldrev.toString());
                    }
                }
                break;
                case svn_repos_notify_load_node_start:
                {
                    QString action;
                    switch(_node_action){
                        case svn_node_action_change:
                            action = "changing";
                            break;
                        case svn_node_action_add:
                            action = "adding";
                            break;
                        case svn_node_action_delete:
                            action = "deletion";
                            break;
                        case svn_node_action_replace:
                            action = "replacing";
                            break;
                    }
                    _msg = QString("Start ").append(action).append(" on node ").append(_path.native());
                }
                break;
                case svn_repos_notify_load_copied_node:
                {
                    _msg = QString("Copied");
                }
                break;
                case svn_repos_notify_load_normalized_mergeinfo:
                {
                    _msg = QString("Removing \\r from ").append(SVN_PROP_MERGEINFO);
                }
                break;
                case svn_repos_notify_mutex_acquired:
                {}
                break;
                case svn_repos_notify_recover_start:
                {}
                break;
                case svn_repos_notify_upgrade_start:
                {}
                break;
            }
        }
#endif
        return _msg;
    }
};

    
ReposNotify::ReposNotify(const svn_repos_notify_t* notify)
{
    m_data=new ReposNotifyData(notify);
}

ReposNotify::~ReposNotify()
{
    delete m_data;
}

ReposNotify::operator const QString&()const
{
    return m_data->toString();
}

}
}