#ifndef __INFO_ENTRY_H
#define __INFO_ENTRY_H

#include "lock_entry.hpp"

#include <qstring.h>

struct svn_info_t;

namespace svn {
  class SVNQT_EXPORT InfoEntry
  {
public:
    InfoEntry();
    InfoEntry(const svn_info_t*,const char*path);
    InfoEntry(const svn_info_t*,const QString&path);
    ~InfoEntry();

    void init(const svn_info_t*,const char*path);
    void init(const svn_info_t*,const QString&path);

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
    const QString&
    cmtAuthor () const
    {
      return m_last_author;
    }
    const QString&
    Name()const
    {
      return m_name;
    }

    const QString& checksum()const
    {
      return m_checksum;
    }

    const QString& conflictNew()const
    {
      return m_conflict_new;
    }
    const QString& conflictOld()const
    {
      return m_conflict_old;
    }
    const QString& conflictWrk()const
    {
      return m_conflict_wrk;
    }
    const QString& copyfromUrl()const
    {
      return m_copyfrom_url;
    }
    const QString& prejfile()const
    {
      return m_prejfile;
    }
    const QString& reposRoot()const
    {
      return m_repos_root;
    }
    const QString& url()const
    {
      return m_url;
    }
    const QString& uuid()const
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

    const QString&prettyUrl()const
    {
        return m_pUrl;
    }

    bool isDir()const
    {
        return kind()==svn_node_dir;
    }
    QString prettyUrl(const char*)const;

protected:
    apr_time_t m_last_changed_date;
    apr_time_t m_text_time;
    apr_time_t m_prop_time;
    bool m_hasWc;
    LockEntry m_Lock;
    QString m_name;
    QString m_checksum;
    QString m_conflict_new;
    QString m_conflict_old;
    QString m_conflict_wrk;
    QString m_copyfrom_url;
    QString m_last_author;
    QString m_prejfile;
    QString m_repos_root;
    QString m_url;
    QString m_pUrl;
    QString m_UUID;
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

