#ifndef _REPOS_LOG_HPP
#define _REPOS_LOG_HPP

class QSqlDatabase;

namespace svn
{

class Client;

namespace cache
{

class ReposLog
{
protected:
    svn::Client*m_Client;
    QSqlDatabase*m_Database;

public:
    ReposLog(svn::Client*,QSqlDatabase*);
};

}
}

#endif

