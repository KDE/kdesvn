#ifndef __INFO_ENTRY_H
#define __INFO_ENTRY_H

#include "svncpp/lock_entry.hpp"

#include <string>

struct svn_info_t;

namespace svn {
  class InfoEntry
  {
public:
    InfoEntry();
    InfoEntry(const svn_info_t*,const char*path);
    ~InfoEntry();
    void init(const svn_info_t*,const char*path);

    apr_time_t cmtDate()const
    {
      return m_last_changed_date;
    }
    apr_time_t textTime()const
    {
      return m_text_time;
    }
    apr_time_t propTime()const
    {
      return m_prop_time;
    }
    bool hasWc()const
    {
      return m_hasWc;
    }
    /**
     * @return lock for that entry
     * @since subversion 1.2
     */
    const LockEntry&
    lockEntry()const
    {
        return m_Lock;
    }
    /**
     * @return last commit author of this file
     */
    const std::string&
    cmtAuthor () const
    {
      return m_last_author;
    }
    const std::string&
    Name()const
    {
      return m_name;
    }

    const std::string& checksum()const
    {
      return m_checksum;
    }

    const std::string& conflictNew()const
    {
      return m_conflict_new;
    }
    const std::string& conflictOld()const
    {
      return m_conflict_old;
    }
    const std::string& conflictWrk()const
    {
      return m_conflict_wrk;
    }
    const std::string& copyfromUrl()const
    {
      return m_copyfrom_url;
    }
    const std::string& prejfile()const
    {
      return m_prejfile;
    }
    const std::string& reposRoot()const
    {
      return m_repos_root;
    }
    const std::string& url()const
    {
      return m_url;
    }
    const std::string& uuid()const
    {
      return m_UUID;
    }
    svn_node_kind_t kind()const
    {
      return m_kind;
    }
    svn_revnum_t cmtRev()const
    {
      return m_last_changed_rev;
    }
    svn_revnum_t copyfromRev()const
    {
      return m_copy_from_rev;
    }
    svn_revnum_t revision()const
    {
      return m_revision;
    }
    svn_wc_schedule_t Schedule()const
    {
        return m_schedule;
    }

protected:
    apr_time_t m_last_changed_date;
    apr_time_t m_text_time;
    apr_time_t m_prop_time;
    bool m_hasWc;
    LockEntry m_Lock;
    std::string m_name;
    std::string m_checksum;
    std::string m_conflict_new;
    std::string m_conflict_old;
    std::string m_conflict_wrk;
    std::string m_copyfrom_url;
    std::string m_last_author;
    std::string m_prejfile;
    std::string m_repos_root;
    std::string m_url;
    std::string m_UUID;
    svn_node_kind_t m_kind;
    svn_revnum_t m_copy_from_rev;
    svn_revnum_t m_last_changed_rev;
    svn_revnum_t m_revision;
    svn_wc_schedule_t m_schedule;
protected:
    void init();
  };
}
#endif

