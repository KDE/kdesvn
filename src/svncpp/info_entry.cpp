#include "svncpp/info_entry.hpp"
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

/*!
    \fn svn::InfoEntry::init(const svn_info_t*)
 */
void svn::InfoEntry::init(const svn_info_t*item,const char*path)
{
  if (!item) {
    init();
    return;
  }
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 2)
  m_name = path?path:"";
  m_last_changed_date=item->last_changed_date;
  m_text_time = item->text_time;
  m_prop_time = item->prop_time;
  if (item->lock) {
    m_Lock.init(item->lock);
  } else {
    m_Lock = LockEntry();
  }
  m_checksum = item->checksum?item->checksum:"";
  m_conflict_new = item->conflict_new?item->conflict_new:"";
  m_conflict_old = item->conflict_old?item->conflict_old:"";
  m_conflict_wrk = item->conflict_wrk?item->conflict_wrk:"";
  m_copyfrom_url = item->copyfrom_url?item->copyfrom_url:"";
  m_last_author = item->last_changed_author?item->last_changed_author:"";
  m_prejfile = item->prejfile?item->prejfile:"";
  m_repos_root = item->repos_root_URL?item->repos_root_URL:"";
  m_url = item->URL?item->URL:"";
  m_UUID = item->repos_UUID?item->repos_UUID:"";
  m_kind = item->kind;
  m_copy_from_rev = item->copyfrom_rev;
  m_last_changed_rev = item->last_changed_rev;
  m_revision = item->rev;
  m_hasWc = item->has_wc_info;
  m_schedule = item->schedule;
#endif
}
