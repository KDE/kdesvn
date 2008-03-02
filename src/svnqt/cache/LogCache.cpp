#include "LogCache.hpp"

#include <qdir.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qthreadstorage.h>
#include <qmap.h>

#ifndef NO_SQLITE3
#include "sqlite3/qsql_sqlite3.h"
#define SQLTYPE "QSQLITE3"
#else
#define SQLTYPE "QSQLITE"
#endif

#define SQLMAIN "logmain-logcache"
#define SQLMAINTABLE "logdb"

namespace svn {
namespace cache {

class ThreadDBStore
{
public:
    ThreadDBStore(){
        m_DB=0;
    }
    ~ThreadDBStore(){
        QSqlDatabase::removeDatabase(key);
        m_DB=0;
    }

    QSqlDatabase*m_DB;
    QString key;
    QMap<QString,QString> reposCacheNames;
};

class LogCacheData
{

protected:
    QMutex m_singleDbMutex;

public:
    LogCacheData(){}
    ~LogCacheData(){
        if (m_mainDB.hasLocalData()) {
            m_mainDB.setLocalData(0L);
        }
    }

    QString createReposDB(const QString&reposroot) {
        QMutexLocker locker( &m_singleDbMutex );
        QSqlQuery query1(QString::null,getMainDB());
        QString q("insert into "+QString(SQLMAINTABLE)+" (reposroot) VALUES('"+reposroot+"')");
        query1.exec("BEGIN TRANSACTION;");
        query1.exec(q);
        query1.exec("commit;");
        QSqlQuery query("SELECT id from "+QString(SQLMAINTABLE)+" where reposroot='"+reposroot+"' ORDER by id DESC",getMainDB());
        QString db;
        if (query.isActive() && query.next()) {
            //int next =
            db = query.value(0).toString();
        } else {
            qDebug("Error select: "+query.lastError().text());
        }
        return db;
    }

    QSqlDatabase*getReposDB(const QString&reposroot) {
        if (!getMainDB()) {
            return 0;
        }
        QSqlCursor c(SQLMAINTABLE,true,getMainDB());
        c.select("reposroot='"+reposroot+"'");
        QString dbFile;
        // only the first one
        if ( c.next() ) {
            qDebug( c.value( "reposroot" ).toString() + ": " +
                    c.value( "id" ).toString() );
            dbFile = c.value( "id" ).toString();
        }
        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
        }
    }

    QSqlDatabase*getMainDB()
    {
        if (!m_mainDB.hasLocalData()) {
            unsigned i=0;
            QString _key = SQLMAIN;
            while (QSqlDatabase::contains(_key)) {
                _key.sprintf("%s-%i",SQLMAIN,i++);
            }
            qDebug("The key is now: %s",_key.latin1());
            QSqlDatabase*db = QSqlDatabase::addDatabase(SQLTYPE,_key);
            db->setDatabaseName(m_BasePath+"/maindb.db");
            if (!db->open()) {
                qWarning("Failed to open main database: " + db->lastError().text());
            } else {
                m_mainDB.setLocalData(new ThreadDBStore);
                m_mainDB.localData()->key = _key;
                m_mainDB.localData()->m_DB = db;
            }
        }
        if (m_mainDB.hasLocalData()) {
            return m_mainDB.localData()->m_DB;
        } else {
            return 0;
        }
    }
    QString m_BasePath;

    QThreadStorage<ThreadDBStore*> m_mainDB;
};

QString LogCache::s_CACHE_FOLDER="logcache";

/*!
    \fn svn::cache::LogCache::LogCache()
 */
LogCache::LogCache()
{
    m_BasePath = QDir::HOMEDIR();
    setupCachePath();
}

LogCache::LogCache(const QString&aBasePath)
{
    if (aBasePath.isEmpty()) {
        m_BasePath=QDir::HOMEDIR();
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
    m_CacheData = new LogCacheData;
    m_CacheData->m_BasePath=m_BasePath;
    QDir d;
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    m_BasePath=m_BasePath+"/"+s_CACHE_FOLDER;
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    m_CacheData->m_BasePath=m_BasePath;
    if (d.exists(m_BasePath)) {
        setupMainDb();
    }
}

void LogCache::setupMainDb()
{
#ifndef NO_SQLITE3
    if (!QSqlDatabase::isDriverAvailable(SQLTYPE)) {
        QSqlDatabase::registerSqlDriver(SQLTYPE,new QSqlDriverCreator<QSQLite3Driver>);
    }
#endif
    QSqlDatabase*mainDB = m_CacheData->getMainDB();
    if (!mainDB || !mainDB->open()) {
        qWarning("Failed to open main database: " + (mainDB?mainDB->lastError().text():"No database object."));
    } else {
        QSqlQuery q(QString::null, mainDB);
        q.exec("BEGIN TRANSACTION;");
        if (!q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY NOT NULL);")) {
            qWarning("Failed create main database: " + mainDB->lastError().text());
        }
        q.exec("COMMIT;");
        QString v = m_CacheData->createReposDB("blub");
        qDebug(v);
    }
}

}
}
