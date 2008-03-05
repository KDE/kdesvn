#ifndef _REPOS_LOG_HPP
#define _REPOS_LOG_HPP

#include "svnqt/svnqt_defines.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/revision.hpp"
#include <qstring.h>

class QSqlDatabase;
class QSqlCursor;

namespace svn
{

class Client;

namespace cache
{

class SVNQT_EXPORT ReposLog
{
protected:
    svn::Client*m_Client;
    QSqlDatabase*m_Database;
    QString m_ReposRoot;
    svn::Revision m_latestHead;
    //! internal insert.
    /*! QSqlCursor as parameter so we may generate them outside a loop for re-usage.
     */
    bool insertLogEntry(const svn::LogEntry&,QSqlCursor&,QSqlCursor&);
    bool checkFill(svn::Revision&_start,svn::Revision&_end);

public:
    ReposLog(svn::Client*aClient,const QString&aRepository=QString::null);

    QString ReposRoot() const
    {
        return m_ReposRoot;
    }

    QSqlDatabase* Database() const
    {
        return m_Database;
    }
    svn::Revision latestHeadRev();
    svn::Revision latestCachedRev();
    //! simple retrieves logentries
    bool simpleLog(LogEntriesMap&target,const svn::Revision&start,const svn::Revision&end);
    svn::Revision date2numberRev(const svn::Revision&);
    bool fillCache(const svn::Revision&end);
};

}
}

#endif
