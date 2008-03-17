#ifndef _REPOS_LOG_HPP
#define _REPOS_LOG_HPP

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/revision.hpp"

#include <qsqldatabase.h>
#include <qstring.h>

namespace svn
{

class Client;

namespace cache
{

class SVNQT_EXPORT ReposLog
{
protected:
    svn::Client*m_Client;
    QDataBase m_Database;
    QString m_ReposRoot;
    svn::Revision m_latestHead;
    //! internal insert.
    bool _insertLogEntry(const svn::LogEntry&);
    bool checkFill(svn::Revision&_start,svn::Revision&_end);

public:
    ReposLog(svn::Client*aClient,const QString&aRepository=QString::null);

    QString ReposRoot() const
    {
        return m_ReposRoot;
    }

    QDataBase Database() const
    {
        return m_Database;
    }
    svn::Revision latestHeadRev();
    svn::Revision latestCachedRev();
    //! simple retrieves logentries
    /*!
     * This method acts on network, too for checking if there are new entries on server.
     */
    bool simpleLog(LogEntriesMap&target,const svn::Revision&start,const svn::Revision&end,bool noNetwork=false);
    svn::Revision date2numberRev(const svn::Revision&,bool noNetwork=false);
    bool fillCache(const svn::Revision&end);
    bool insertLogEntry(const svn::LogEntry&);
    bool log(const svn::Path&,const svn::Revision&start, const svn::Revision&end,const svn::Revision&peg,svn::LogEntriesMap&target, bool strictNodeHistory,int limit);
    bool itemExists(const svn::Revision&,const svn::Path&);
};

}
}

#endif
