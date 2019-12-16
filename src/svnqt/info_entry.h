/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#ifndef SVNQT_INFO_ENTRY_H
#define SVNQT_INFO_ENTRY_H

#include <svnqt/helper.h>
#include <svnqt/lock_entry.h>
#include <svnqt/datetime.h>
#include <svnqt/revision.h>
#include <svnqt/svnqttypes.h>
#include <svnqt/conflictdescription.h>

#include <QString>
#include <QUrl>

#include <svn_version.h>

struct svn_client_info2_t;

namespace svn
{
class SVNQT_EXPORT InfoEntry
{
public:
    static const qlonglong SVNQT_SIZE_UNKNOWN = -1;
    InfoEntry();
    InfoEntry(const svn_client_info2_t *, const char *path);
    InfoEntry(const svn_client_info2_t *, const QString &path);
    ~InfoEntry();

    void init(const svn_client_info2_t *, const char *path);
    void init(const svn_client_info2_t *, const QString &path);

    DateTime cmtDate()const;
    DateTime textTime()const;
    DateTime propTime()const;
    bool hasWc()const;
    /**
     * @return lock for that entry
     * @since subversion 1.2
     */
    const LockEntry &lockEntry()const;
    /**
     * @return last commit author of this file
     */
    const QString &cmtAuthor() const;
    const QString &Name()const;

    const QString &checksum()const;
    const ConflictDescriptionList &conflicts()const;
    const QUrl &copyfromUrl()const;
    const QString &prejfile()const;
    const QUrl &reposRoot()const;
    const QUrl &url()const;
    const QString &uuid()const;
    svn_node_kind_t kind()const;
    const Revision &cmtRev()const;
    const Revision &copyfromRev()const;
    const Revision &revision()const;
    svn_wc_schedule_t Schedule()const;

    qlonglong size()const;
    qlonglong working_size()const;
    const QByteArray &changeList()const;
    svn::Depth depth()const;

    bool isDir()const;

protected:
    DateTime m_last_changed_date;
    DateTime m_text_time;
    DateTime m_prop_time;
    bool m_hasWc;
    LockEntry m_Lock;
    QString m_name;
    QString m_checksum;
    ConflictDescriptionList m_conflicts;
    QUrl m_copyfrom_url;
    QString m_last_author;
    QString m_prejfile;
    QUrl m_repos_root;
    QUrl m_url;
    QString m_UUID;
    svn_node_kind_t m_kind;
    Revision m_copy_from_rev;
    Revision m_last_changed_rev;
    Revision m_revision;
    svn_wc_schedule_t m_schedule;

    qlonglong m_size;
    qlonglong m_working_size;
    QByteArray m_changeList;
    svn::Depth m_depth;

protected:
    void init();
};
}
#endif
