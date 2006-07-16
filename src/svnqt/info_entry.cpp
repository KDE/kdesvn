#include "info_entry.hpp"
#include "svncpp_defines.hpp"
#include <svn_client.h>

namespace svn
{
  InfoEntry::InfoEntry()
  {
    init();
  }

  InfoEntry::InfoEntry(const svn_info_t*info,const char*path)
  {
    init(info,path);
  }

  InfoEntry::InfoEntry(const svn_info_t*info,const QString&path)
  {
    init(info,path);
  }

  InfoEntry::~InfoEntry()
  {
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
  m_conflict_new = "";
  m_conflict_old = "";
  m_conflict_wrk = "";
  m_copyfrom_url = "";
  m_last_author = "";
  m_prejfile = "";
  m_repos_root = "";
  m_url = "";
  m_UUID = "";
  m_kind = svn_node_none;
  m_copy_from_rev = SVN_INVALID_REVNUM;
  m_last_changed_rev = SVN_INVALID_REVNUM;
  m_revision = SVN_INVALID_REVNUM;
  m_schedule = svn_wc_schedule_normal;
}

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
  m_UUID = QString::FROMUTF8(item->repos_UUID);
  m_kind = item->kind;
  m_copy_from_rev = item->copyfrom_rev;
  m_last_changed_rev = item->last_changed_rev;
  m_revision = item->rev;
  m_hasWc = item->has_wc_info;
  m_schedule = item->schedule;
}
