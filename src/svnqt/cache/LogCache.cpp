#include "LogCache.hpp"

#include <qdir.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qthreadstorage.h>
#include <qmap.h>

#include "src/svnqt/path.hpp"

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

LogCache* LogCache::mSelf = 0;

class ThreadDBStore
{
public:
    ThreadDBStore(){
        m_DB=0;
    }
    ~ThreadDBStore(){
        QSqlDatabase::removeDatabase(key);
        m_DB=0;
        QMap<QString,QString>::Iterator it;
        for (it=reposCacheNames.begin();it!=reposCacheNames.end();++it) {
            QSqlDatabase::removeDatabase(it.data());
        }
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

    bool checkReposDb(QSqlDatabase*aDb)
    {
        if (!aDb) {
            return false;
        }
        if (!aDb->open()) {
            qWarning("Failed to open new database: " + aDb->lastError().text());
            return false;
        }
        QStringList list = aDb->tables();
        QSqlQuery _q(QString::null, aDb);
        if (list.find("logentries")==list.end()) {
            aDb->transaction();
            _q.exec("CREATE TABLE \"logentries\" (\"revision\" INTEGER UNIQUE,\"date\" INTEGER,\"author\" TEXT, \"message\" TEXT)");
            aDb->commit();
        }
        if (list.find("changeditems")==list.end()) {
            aDb->transaction();
            _q.exec("CREATE TABLE \"changeditems\" (\"revision\" INTEGER,\"changeditem\" TEXT,\"action\" TEXT,\"copyfrom\" TEXT,\"copyfromrev\" INTEGER, PRIMARY KEY(revision,changeditem,action))");
            aDb->commit();
        }
        list = aDb->tables();
        if (list.find("logentries")==list.end() || list.find("changeditems")==list.end()) {
            return false;
        }
        return true;
    }

    QString createReposDB(const svn::Path&reposroot) {
        QMutexLocker locker( &m_singleDbMutex );
        QSqlQuery query1(QString::null,getMainDB());
        QString q("insert into "+QString(SQLMAINTABLE)+" (reposroot) VALUES('"+reposroot+"')");
        getMainDB()->transaction();
        query1.exec(q);
        getMainDB()->commit();

        QSqlQuery query(QString::null,getMainDB());
        query.prepare(s_reposSelect);
        query.bindValue(0,reposroot.native());
        query.exec();
        QString db;
        if (query.lastError().type()==QSqlError::None && query.next()) {
            db = query.value(0).toString();
        } else {
            qDebug("Error select_01: "+query.lastError().text()+" ("+query.lastQuery()+")");
        }
        if (!db.isEmpty()) {
            QString fulldb = m_BasePath+"/"+db+".db";
            QSqlDatabase*_db = QSqlDatabase::addDatabase(SQLTYPE,"tmpdb");
            _db->setDatabaseName(fulldb);
            if (!checkReposDb(_db)) {
            }
            QSqlDatabase::removeDatabase("tmpdb");
        }
        return db;
    }

    QSqlDatabase*getReposDB(const svn::Path&reposroot) {
        if (!getMainDB()) {
            return 0;
        }
        bool checkDone = false;
        // make sure path is correct eg. without traling slashes.
        QString dbFile;
        QSqlQuery c(QString::null,getMainDB());
        c.prepare(s_reposSelect);
        c.bindValue(0,reposroot.native());
        c.exec();

        qDebug("Check for path: "+reposroot.native());
        qDebug("Last: "+c.lastQuery());

        // only the first one
        if ( c.next() ) {
            qDebug( c.value(0).toString() + ": " +
                    c.value(0).toString() );
            dbFile = c.value(0).toString();
        }
        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
            if (dbFile.isEmpty()) {
                return 0;
            }
            checkDone=true;
        }
        if (m_mainDB.localData()->reposCacheNames.find(dbFile)!=m_mainDB.localData()->reposCacheNames.end()) {
            return QSqlDatabase::database(m_mainDB.localData()->reposCacheNames[dbFile]);
        }
        int i = 0;
        QString _key = dbFile;
        while (QSqlDatabase::contains(_key)) {
            _key = QString("%1-%2").arg(dbFile).arg(i++);
        }
        qDebug("The repository key is now: %s",_key.latin1());
        QSqlDatabase*_db = QSqlDatabase::addDatabase(SQLTYPE,_key);
        if (!_db) {
            qWarning("Got no db!");
            return 0;
        }
        QString fulldb = m_BasePath+"/"+dbFile+".db";
        _db->setDatabaseName(fulldb);
        qDebug("try database open %s",fulldb.latin1());
        if (!checkReposDb(_db)) {
            qDebug("no DB opened");
            _key = QString("Failed open database %1: %2").arg(fulldb).arg(_db->lastError().text());
            qWarning(_key);
            _db = 0;
        } else {
            qDebug("Insert into map");
            m_mainDB.localData()->reposCacheNames[dbFile]=_key;
        }
        return _db;
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

    static const QString s_reposSelect;
};


QString LogCache::s_CACHE_FOLDER="logcache";
const QString LogCacheData::s_reposSelect=QString("SELECT id from ")+QString(SQLMAINTABLE)+QString(" where reposroot=? ORDER by id DESC");

/*!
    \fn svn::cache::LogCache::LogCache()
 */
LogCache::LogCache()
{
    m_BasePath = QDir::HOMEDIR()+"/.svnqt";
    setupCachePath();
}

LogCache::LogCache(const QString&aBasePath)
{
    if (mSelf) {
        delete mSelf;
    }
    mSelf=this;
    if (aBasePath.isEmpty()) {
        m_BasePath=QDir::HOMEDIR()+"/.svnqt";
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
        mainDB->transaction();
        if (!q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY NOT NULL);")) {
            qWarning("Failed create main database: " + mainDB->lastError().text());
        }
        mainDB->commit();
    }
}

}
}


/*!
    \fn svn::cache::LogCache::self()
 */
svn::cache::LogCache* svn::cache::LogCache::self()
{
    if (!mSelf) {
        mSelf=new LogCache();
    }
    return mSelf;
}


/*!
    \fn svn::cache::LogCache::reposDb()
 */
QSqlDatabase* svn::cache::LogCache::reposDb(const QString&aRepository)
{
    qDebug("reposDB");
    return m_CacheData->getReposDB(aRepository);
}
