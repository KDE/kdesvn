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
#include "svnqt/helper.h"

#include <svn_props.h>
#include <svn_repos.h>

namespace svn
{
namespace repository
{

class ReposNotifyData
{
    QString _warning_msg;

    /// TODO own datatype
    svn_repos_notify_action_t _action;
    svn::Revision _rev;
    /// TODO own datatype
    svn_repos_notify_warning_t _warning;
    qlonglong _shard;

    svn::Revision _oldrev;
    svn::Revision _newrev;

    /// TODO own datatype
    svn_node_action _node_action;

    svn::Path _path;

    mutable QString _msg;

public:
    ReposNotifyData(const svn_repos_notify_t *notify)
        : _warning_msg(QString()), _msg(QString())
    {
        if (!notify) {
            return;
        }
        _action = notify->action;
        _rev = notify->revision;
        if (notify->warning_str) {
            _warning_msg = QString::fromUtf8(notify->warning_str);
        }
        _warning = notify->warning;
        _shard = notify->shard;
        _oldrev = notify->old_revision;
        _newrev = notify->new_revision;
        _node_action = notify->node_action;
        if (notify->path != nullptr) {
            _path = svn::Path(QString::fromUtf8(notify->path));
        }
    }

    ~ReposNotifyData()
    {
    }

    const QString &toString()const
    {
        if (_msg.isEmpty()) {
            switch (_action) {
            case svn_repos_notify_warning: {
                switch (_warning) {
                case svn_repos_notify_warning_found_old_reference:
                    _msg = QStringLiteral("Old Reference: ");
                    break;
                case svn_repos_notify_warning_found_old_mergeinfo:
                    _msg = QStringLiteral("Old mergeinfo found: ");
                    break;
                case svn_repos_notify_warning_invalid_fspath:
                    _msg = QStringLiteral("Invalid path: ");
                    break;
                default:
                    _msg.clear();
                }
                _msg += _warning_msg;
            }
            break;
            case svn_repos_notify_dump_rev_end:
            case svn_repos_notify_verify_rev_end: {
                _msg = QStringLiteral("Revision %1 finished.").arg(_rev.toString());
            }
            break;
            case svn_repos_notify_dump_end: {
                _msg = QStringLiteral("Dump finished");
            }
            break;
            case svn_repos_notify_verify_end: {
                _msg = QStringLiteral("Verification finished");
            }
            break;
            case svn_repos_notify_pack_shard_start: {
                _msg = QStringLiteral("Packing revisions in shard %1").arg(_shard);
            }
            break;
            case svn_repos_notify_pack_shard_end_revprop:
            case svn_repos_notify_pack_shard_end:
            case svn_repos_notify_load_node_done: {
                _msg = QStringLiteral("Done");
            }
            break;
            case svn_repos_notify_pack_shard_start_revprop: {
                _msg = QStringLiteral("Packing revsion properties in shard %1").arg(_shard);
            }
            break;
            case svn_repos_notify_load_txn_start: {
                _msg = QStringLiteral("Start loading old revision %1").arg(_oldrev.toString());
            }
            break;
            case svn_repos_notify_load_txn_committed: {
                _msg = QStringLiteral("Committed new revision %1").arg(_newrev.toString());
                if (_oldrev.isValid()) {
                    _msg.append(QLatin1String(" loaded from original revision ")).append(_oldrev.toString());
                }
            }
            break;
            case svn_repos_notify_load_node_start: {
                QString action;
                switch (_node_action) {
                case svn_node_action_change:
                    action = QStringLiteral("changing");
                    break;
                case svn_node_action_add:
                    action = QStringLiteral("adding");
                    break;
                case svn_node_action_delete:
                    action = QStringLiteral("deletion");
                    break;
                case svn_node_action_replace:
                    action = QStringLiteral("replacing");
                    break;
                }
                _msg = QLatin1String("Start ") + action + QLatin1String(" on node ") + _path.native();
            }
            break;
            case svn_repos_notify_load_copied_node: {
                _msg = QStringLiteral("Copied");
            }
            break;
            case svn_repos_notify_load_normalized_mergeinfo: {
                _msg = QStringLiteral("Removing \\r from ") + QLatin1String(SVN_PROP_MERGEINFO);
            }
            break;
            case svn_repos_notify_mutex_acquired: {
            }
            break;
            case svn_repos_notify_recover_start: {
            }
            break;
            case svn_repos_notify_upgrade_start: {
            }
            break;
            }
        }
        return _msg;
    }
};

ReposNotify::ReposNotify(const svn_repos_notify_t *notify)
{
    m_data = new ReposNotifyData(notify);
}

ReposNotify::~ReposNotify()
{
    delete m_data;
}

ReposNotify::operator const QString &()const
{
    return m_data->toString();
}

}
}

