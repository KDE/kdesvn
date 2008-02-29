#include "LogCache.hpp"

#include <qdir.h>

#if 0
/// don't care about qt ones in qt3 - it uses sqlite2
#include <qsql.h>
#include <qsqldatabase.h>
#endif

#define SQLTYPE "QSQLITE"
#define SQLMAIN "logmain"

namespace svn {
namespace cache {

QString LogCache::s_CACHE_FOLDER="logcache";

/*!
    \fn svn::cache::LogCache::LogCache()
 */
LogCache::LogCache()
{
    m_BasePath=QDir::homeDirPath();
    setupCachePath();
}

LogCache::LogCache(const QString&aBasePath)
{
    if (aBasePath.isEmpty()) {
        m_BasePath=QDir::homeDirPath();
    } else {
        m_BasePath=aBasePath;
    }
    setupCachePath();
}


LogCache::~LogCache()
{
}

/*!
    \fn svn::cache::LogCache::setupCachePath()
 */
void LogCache::setupCachePath()
{
    QDir d;
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    m_BasePath=m_BasePath+"/"+s_CACHE_FOLDER;
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    if (d.exists(m_BasePath)) {
        setupMainDb();
    }
}

void LogCache::setupMainDb()
{

#if 0
    QSqlDatabase*mainDB = QSqlDatabase::addDatabase(SQLTYPE,SQLMAIN);
    mainDB->setDatabaseName(m_BasePath+"/maindb.db");
    if (!mainDB->open()) {
        qWarning("Failed to open main database: " + mainDB->lastError().text());
    }
    QSqlQuery q(QString::null, mainDB);
    q.exec("BEGIN TRANSACTION;");
    q.exec("CREATE TABLE \"logdb\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY,\"databasefile\" TEXT);");
    q.exec("COMMIT;");
#endif
}

}
}

