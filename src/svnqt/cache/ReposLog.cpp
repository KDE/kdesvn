#include "ReposLog.hpp"

/*!
    \fn svn::cache::ReposLog::ReposLog()
 */
svn::cache::ReposLog::ReposLog(svn::Client*aClient,QSqlDatabase*aDB)
    :m_Client(aClient),m_Database(aDB)
{
}
