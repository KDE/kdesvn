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
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "svnqt/info_entry.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/pool.h"
#include "svnqt/conflictdescription.h"
#include <svn_client.h>
#include <svn_path.h>
#ifdef HAS_SVN_VERSION_H
#include <svn_version.h>
#endif

namespace svn
{

  InfoEntry::InfoEntry()
  {
    init();
  }

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
  InfoEntry::InfoEntry(const svn_client_info2_t*info, const char*path)
  {
      init(info,path);
  }

  InfoEntry::InfoEntry(const svn_client_info2_t*info, const QString&path)
  {
      init(info,path);
  }
#else
    InfoEntry::InfoEntry(const svn_info_t*info,const char*path)
    {
        init(info,path);
    }

    InfoEntry::InfoEntry(const svn_info_t*info,const QString&path)
    {
        init(info,path);
    }
#endif

  InfoEntry::~InfoEntry()
  {
  }

  DateTime InfoEntry::cmtDate()const
  {
      return m_last_changed_date;
  }
  DateTime InfoEntry::textTime()const
  {
      return m_text_time;
  }
  DateTime InfoEntry::propTime()const
  {
      return m_prop_time;
  }
  bool InfoEntry::hasWc()const
  {
      return m_hasWc;
  }
  const LockEntry&InfoEntry::lockEntry()const
  {
      return m_Lock;
  }
  const QString&InfoEntry::cmtAuthor () const
  {
      return m_last_author;
  }
  const QString&InfoEntry::Name()const
  {
      return m_name;
  }
  const QString& InfoEntry::checksum()const
  {
      return m_checksum;
  }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
  const QLIST<ConflictDescriptionP> & InfoEntry::conflicts()const
  {
      return m_conflicts;
  }
#else
  const QString& InfoEntry::conflictNew()const
  {
      return m_conflict_new;
  }
  const QString& InfoEntry::conflictOld()const
  {
      return m_conflict_old;
  }
  const QString& InfoEntry::conflictWrk()const
  {
      return m_conflict_wrk;
  }
#endif
  const QString& InfoEntry::copyfromUrl()const
  {
      return m_copyfrom_url;
  }
  const QString& InfoEntry::prejfile()const
  {
      return m_prejfile;
  }
  const QString& InfoEntry::reposRoot()const
  {
      return m_repos_root;
  }
  const QString& InfoEntry::url()const
  {
      return m_url;
  }
  const QString& InfoEntry::uuid()const
  {
      return m_UUID;
  }
  svn_node_kind_t InfoEntry::kind()const
  {
      return m_kind;
  }
  const Revision& InfoEntry::cmtRev()const
  {
      return m_last_changed_rev;
  }
  const Revision& InfoEntry::copyfromRev()const
  {
      return m_copy_from_rev;
  }
  const Revision& InfoEntry::revision()const
  {
      return m_revision;
  }
  svn_wc_schedule_t InfoEntry::Schedule()const
  {
      return m_schedule;
  }
  const QString&InfoEntry::prettyUrl()const
  {
      return m_pUrl;
  }
  bool InfoEntry::isDir()const
  {
      return kind()==svn_node_dir;
  }
  const QByteArray&InfoEntry::changeList()const
  {
      return m_changeList;
  }
  QLONG InfoEntry::size()const
  {
      return m_size;
  }
  QLONG InfoEntry::working_size()const
  {
      return m_working_size;
  }
  svn::Depth InfoEntry::depth()const
  {
      return m_depth;
  }
}

/*!
    \fn svn::InfoEntry::init()
 */
void svn::InfoEntry::init()
{
  m_name = "";
  m_last_changed_date=0;
  m_text_time = 0;
  m_prop_time = 0;
  m_hasWc = false;
  m_Lock = LockEntry();
  m_checksum = "";
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
#else
  m_conflict_new = "";
  m_conflict_old = "";
  m_conflict_wrk = "";
#endif
  m_copyfrom_url = "";
  m_last_author = "";
  m_prejfile = "";
  m_repos_root = "";
  m_url = "";
  m_pUrl = "";
  m_UUID = "";
  m_kind = svn_node_none;
  m_copy_from_rev = SVN_INVALID_REVNUM;
  m_last_changed_rev = SVN_INVALID_REVNUM;
  m_revision = SVN_INVALID_REVNUM;
  m_schedule = svn_wc_schedule_normal;

  m_size = m_working_size = SVNQT_SIZE_UNKNOWN;
  m_changeList=QByteArray();
  m_depth = DepthUnknown;
}

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
void svn::InfoEntry::init(const svn_client_info2_t*item,const char*path)
{
    init(item,QString::FROMUTF8(path));
}

void svn::InfoEntry::init(const svn_client_info2_t*item,const QString&path)
{
    if (!item) {
        init();
        return;
    }
    m_name = path;
    m_last_changed_date=item->last_changed_date;
    if (item->lock) {
        m_Lock.init(item->lock);
    } else {
        m_Lock = LockEntry();
    }
    m_size = item->size!=SVN_INVALID_FILESIZE?QLONG(item->size):SVNQT_SIZE_UNKNOWN;
    m_repos_root = QString::FROMUTF8(item->repos_root_URL);
    m_url = QString::FROMUTF8(item->URL);
    m_pUrl = prettyUrl(item->URL);
    m_UUID = QString::FROMUTF8(item->repos_UUID);
    m_kind = item->kind;
    m_revision = item->rev;
    m_last_changed_rev = item->last_changed_rev;
    m_last_author = QString::FROMUTF8(item->last_changed_author);
    if (item->wc_info != 0)
    {
        m_hasWc = true;
        m_schedule = item->wc_info->schedule;
        m_copyfrom_url = QString::FROMUTF8(item->wc_info->copyfrom_url);
        m_copy_from_rev = item->wc_info->copyfrom_rev;
        if (item->wc_info->changelist) {
            m_changeList = QByteArray(item->wc_info->changelist,strlen(item->wc_info->changelist));
        } else {
            m_changeList=QByteArray();
        }
        if (item->wc_info->conflicts != 0) {
            for (int j = 0; j < item->wc_info->conflicts->nelts; ++j) {
                svn_wc_conflict_description2_t*_desc = ((svn_wc_conflict_description2_t **)item->wc_info->conflicts->elts)[j];
                m_conflicts.push_back(new ConflictDescription(_desc));
            }
        }
        
        switch (item->wc_info->depth) {
            case svn_depth_exclude:
                m_depth=DepthExclude;
                break;
            case svn_depth_empty:
                m_depth=DepthEmpty;
                break;
            case svn_depth_files:
                m_depth=DepthFiles;
                break;
            case svn_depth_immediates:
                m_depth=DepthImmediates;
                break;
            case svn_depth_infinity:
                m_depth=DepthInfinity;
                break;
            case svn_depth_unknown:
            default:
                m_depth=DepthUnknown;
                break;
        }
    } else {
        m_hasWc = false;
    }
}

#else

void svn::InfoEntry::init(const svn_info_t*item,const char*path)
{
    init(item,QString::FROMUTF8(path));
}

/*!
    \fn svn::InfoEntry::init(const svn_info_t*)
 */
void svn::InfoEntry::init(const svn_info_t*item,const QString&path)
{
  if (!item) {
    init();
    return;
  }
  m_name = path;
  m_last_changed_date=item->last_changed_date;
  m_text_time = item->text_time;
  m_prop_time = item->prop_time;
  if (item->lock) {
    m_Lock.init(item->lock);
  } else {
    m_Lock = LockEntry();
  }
  m_checksum = QString::FROMUTF8(item->checksum);
  m_conflict_new = QString::FROMUTF8(item->conflict_new);
  m_conflict_old = QString::FROMUTF8(item->conflict_old);
  m_conflict_wrk = QString::FROMUTF8(item->conflict_wrk);
  m_copyfrom_url = QString::FROMUTF8(item->copyfrom_url);
  m_last_author = QString::FROMUTF8(item->last_changed_author);
  m_prejfile = QString::FROMUTF8(item->prejfile);
  m_repos_root = QString::FROMUTF8(item->repos_root_URL);
  m_url = QString::FROMUTF8(item->URL);
  m_pUrl = prettyUrl(item->URL);
  m_UUID = QString::FROMUTF8(item->repos_UUID);
  m_kind = item->kind;
  m_copy_from_rev = item->copyfrom_rev;
  m_last_changed_rev = item->last_changed_rev;
  m_revision = item->rev;
  m_hasWc = item->has_wc_info;
  m_schedule = item->schedule;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6)) || (SVN_VER_MAJOR > 1)
  m_size = item->size64!=SVN_INVALID_FILESIZE?QLONG(item->size64):SVNQT_SIZE_UNKNOWN;
  m_working_size = item->working_size64!=SVN_INVALID_FILESIZE?QLONG(item->working_size64):SVNQT_SIZE_UNKNOWN;
  if (m_working_size == SVNQT_SIZE_UNKNOWN) {
      m_working_size = item->working_size!=SVN_INFO_SIZE_UNKNOWN?QLONG(item->working_size):SVNQT_SIZE_UNKNOWN;
  }
#else
  m_size = item->size!=SVN_INFO_SIZE_UNKNOWN?QLONG(item->size):SVNQT_SIZE_UNKNOWN;
  m_working_size = item->working_size!=SVN_INFO_SIZE_UNKNOWN?QLONG(item->working_size):SVNQT_SIZE_UNKNOWN;
#endif

  if (item->changelist) {
      m_changeList = QByteArray(item->changelist,strlen(item->changelist));
  } else {
      m_changeList=QByteArray();
  }

  switch (item->depth) {
      case svn_depth_exclude:
          m_depth=DepthExclude;
          break;
      case svn_depth_empty:
          m_depth=DepthEmpty;
          break;
      case svn_depth_files:
          m_depth=DepthFiles;
          break;
      case svn_depth_immediates:
          m_depth=DepthImmediates;
          break;
      case svn_depth_infinity:
          m_depth=DepthInfinity;
          break;
      case svn_depth_unknown:
      default:
          m_depth=DepthUnknown;
          break;
  }
}
#endif

QString svn::InfoEntry::prettyUrl(const char*_url)const
{
    if (_url) {
        Pool pool;
        _url = svn_path_uri_decode(_url,pool);
        return QString::FROMUTF8(_url);
    }
    return QString::FROMUTF8("");
}

svn::InfoEntry::InfoEntry(const InfoEntry&other)
{
    m_name = other.m_name;
    m_last_changed_date=other.m_last_changed_date;
    m_text_time = other.m_text_time;
    m_prop_time = other.m_prop_time;
    m_Lock = other.m_Lock;
    m_checksum = other.m_checksum;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)    
    m_conflicts = other.m_conflicts;
#else
    m_conflict_new = other.m_conflict_new;
    m_conflict_old = other.m_conflict_old;
    m_conflict_wrk = other.m_conflict_wrk;
#endif
    m_copyfrom_url = other.m_copyfrom_url;
    m_last_author = other.m_last_author;
    m_prejfile = other.m_prejfile;
    m_repos_root = other.m_repos_root;
    m_url = other.m_url;
    m_pUrl = other.m_pUrl;
    m_UUID = other.m_UUID;
    m_kind = other.m_kind;
    m_copy_from_rev = other.m_copy_from_rev;
    m_last_changed_rev = other.m_last_changed_rev;
    m_revision = other.m_revision;
    m_hasWc = other.m_hasWc;
    m_schedule = other.m_schedule;
    m_size = other.m_size;
    m_working_size = other.m_working_size;
}
