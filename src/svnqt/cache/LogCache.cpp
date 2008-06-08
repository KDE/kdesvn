#include "LogCache.hpp"

#include <qdir.h>
#include <qsql.h>
#include <qsqldatabase.h>
#if QT_VERSION < 0x040000
#include <qthreadstorage.h>
#else
#include <QMutex>
#include <QThreadStorage>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#endif
#include <qmap.h>

#include "svnqt/path.hpp"
#include "svnqt/cache/DatabaseException.hpp"

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
#if QT_VERSION < 0x040000
        m_DB=0;
#else
        m_DB=QSqlDatabase();
#endif
    }
    ~ThreadDBStore(){
#if QT_VERSION < 0x040000
        m_DB=0;
#else
        m_DB=QSqlDatabase();
#endif
        QSqlDatabase::removeDatabase(key);
        QMap<QString,QString>::Iterator it;
        for (it=reposCacheNames.begin();it!=reposCacheNames.end();++it) {
#if QT_VERSION < 0x040000
            QSqlDatabase::removeDatabase(it.data());
#else
            QSqlDatabase::removeDatabase(it.value());
#endif
        }
    }

    QDataBase m_DB;
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

    bool checkReposDb(QDataBase aDb)
    {
#if QT_VERSION < 0x040000
        if (!aDb) {
            return false;
        }
        if (!aDb->open()) {
            return false;
        }
#else
        if (!aDb.open()) {
            return false;
        }
#endif

        QSqlQuery _q(QString::null, aDb);
#if QT_VERSION < 0x040000
        QStringList list = aDb->tables();
#else
        QStringList list = aDb.tables();
#endif

#if QT_VERSION < 0x040000
        if (list.find("logentries")==list.end()) {
            aDb->transaction();
#else
        if (list.indexOf("logentries")==-1) {
            aDb.transaction();
#endif
            _q.exec("CREATE TABLE \"logentries\" (\"revision\" INTEGER UNIQUE,\"date\" INTEGER,\"author\" TEXT, \"message\" TEXT)");
#if QT_VERSION < 0x040000
            aDb->commit();
#else
            aDb.commit();
#endif
        }
#if QT_VERSION < 0x040000
        if (list.find("changeditems")==list.end()) {
            aDb->transaction();
#else
        if (list.indexOf("changeditems")==-1) {
            aDb.transaction();
#endif
            _q.exec("CREATE TABLE \"changeditems\" (\"revision\" INTEGER,\"changeditem\" TEXT,\"action\" TEXT,\"copyfrom\" TEXT,\"copyfromrev\" INTEGER, PRIMARY KEY(revision,changeditem,action))");
#if QT_VERSION < 0x040000
            aDb->commit();
#else
            aDb.commit();
#endif
        }
#if QT_VERSION < 0x040000
        list = aDb->tables();
        if (list.find("logentries")==list.end() || list.find("changeditems")==list.end()) {
#else
        list = aDb.tables();
        if (list.indexOf("logentries")==-1 || list.indexOf("changeditems")==-1) {
#endif
            return false;
        }
        return true;
    }

    QString createReposDB(const svn::Path&reposroot) {
        QMutexLocker locker( &m_singleDbMutex );

        QDataBase _mdb = getMainDB();

        QSqlQuery query1(QString::null,_mdb);
        QString q("insert into "+QString(SQLMAINTABLE)+" (reposroot) VALUES('"+reposroot+"')");
#if QT_VERSION < 0x040000
        _mdb->transaction();
#else
        _mdb.transaction();
#endif

        query1.exec(q);
#if QT_VERSION < 0x040000
        _mdb->commit();
#else
        _mdb.commit();
#endif
        QSqlQuery query(QString::null,_mdb);
        query.prepare(s_reposSelect);
        query.bindValue(0,reposroot.native());
        query.exec();
        QString db;
#if QT_VERSION < 0x040000
        if (query.lastError().type()==QSqlError::None && query.next()) {
#else
        if (query.lastError().type()==QSqlError::NoError && query.next()) {
#endif
            db = query.value(0).toString();
        }
        else {
            qDebug("Error select_01: %s (%s)",query.lastError().text().TOUTF8().data(),
                   query.lastQuery().TOUTF8().data());
        }
        if (!db.isEmpty()) {
            QString fulldb = m_BasePath+"/"+db+".db";
            QDataBase _db = QSqlDatabase::addDatabase(SQLTYPE,"tmpdb");
#if QT_VERSION < 0x040000
            _db->setDatabaseName(fulldb);
#else
            _db.setDatabaseName(fulldb);
#endif
            if (!checkReposDb(_db)) {
            }
            QSqlDatabase::removeDatabase("tmpdb");
        }
        return db;
    }

    QDataBase getReposDB(const svn::Path&reposroot) {
#if QT_VERSION < 0x040000
        if (!getMainDB()) {
            return 0;
#else
        if (!getMainDB().isValid()) {
            return QDataBase();
#endif
        }
        bool checkDone = false;
        // make sure path is correct eg. without traling slashes.
        QString dbFile;
        QSqlQuery c(QString::null,getMainDB());
        c.prepare(s_reposSelect);
        c.bindValue(0,reposroot.native());
        c.exec();

#if QT_VERSION < 0x040000
        qDebug("Check for path: "+reposroot.native());
#endif

        // only the first one
        if ( c.next() ) {
#if QT_VERSION < 0x040000
            qDebug( c.value(0).toString() + ": " +
                    c.value(0).toString() );
#endif
            dbFile = c.value(0).toString();
        }
        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
            if (dbFile.isEmpty()) {
#if QT_VERSION < 0x040000
                return 0;
#else
                return QSqlDatabase();
#endif
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
        qDebug("The repository key is now: %s",_key.TOUTF8().data());
        QDataBase _db = QSqlDatabase::addDatabase(SQLTYPE,_key);
#if QT_VERSION < 0x040000
        if (!_db) {
            return 0;
        }
#endif
        QString fulldb = m_BasePath+"/"+dbFile+".db";
#if QT_VERSION < 0x040000
        _db->setDatabaseName(fulldb);
#else
        _db.setDatabaseName(fulldb);
#endif
        qDebug("try database open %s",fulldb.TOUTF8().data());
        if (!checkReposDb(_db)) {
            qDebug("no DB opened");
#if QT_VERSION < 0x040000
            _db = 0;
#else
            _db = QSqlDatabase();
#endif
        } else {
            qDebug("Insert into map");
            m_mainDB.localData()->reposCacheNames[dbFile]=_key;
        }
        return _db;
    }

    QDataBase getMainDB()const
    {
        if (!m_mainDB.hasLocalData()) {
            unsigned i=0;
            QString _key = SQLMAIN;
            while (QSqlDatabase::contains(_key)) {
                _key.sprintf("%s-%i",SQLMAIN,i++);
            }
            qDebug("The key is now: %s",_key.TOUTF8().data());

            QDataBase db = QSqlDatabase::addDatabase(SQLTYPE,_key);
#if QT_VERSION < 0x040000
            db->setDatabaseName(m_BasePath+"/maindb.db");
            if (!db->open()) {
#else
            db.setDatabaseName(m_BasePath+"/maindb.db");
            if (!db.open()) {
#endif
#if QT_VERSION < 0x040000
                qWarning("Failed to open main database: " + db->lastError().text());
#endif
            } else {
                m_mainDB.setLocalData(new ThreadDBStore);
                m_mainDB.localData()->key = _key;
                m_mainDB.localData()->m_DB = db;
            }
        }
        if (m_mainDB.hasLocalData()) {
            return m_mainDB.localData()->m_DB;
        } else {
#if QT_VERSION < 0x040000
            return 0;
#else
            return QSqlDatabase();
#endif
        }
    }
    QString m_BasePath;

    mutable QThreadStorage<ThreadDBStore*> m_mainDB;

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
    QDataBase mainDB = m_CacheData->getMainDB();
#if QT_VERSION < 0x040000
    if (!mainDB || !mainDB->open()) {
        qWarning("Failed to open main database: " + (mainDB?mainDB->lastError().text():"No database object."));
#else
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
#endif
    } else {
        QSqlQuery q(QString::null, mainDB);
#if QT_VERSION < 0x040000
        mainDB->transaction();
#else
        mainDB.transaction();
#endif
        if (!q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY NOT NULL);")) {
#if QT_VERSION < 0x040000
            qWarning("Failed create main database: " + mainDB->lastError().text());
#endif
        }
#if QT_VERSION < 0x040000
        mainDB->commit();
#else
        mainDB.commit();
#endif
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
QDataBase  svn::cache::LogCache::reposDb(const QString&aRepository)
{
    qDebug("reposDB");
    return m_CacheData->getReposDB(aRepository);
}


/*!
    \fn svn::cache::LogCache::cachedRepositories()const
 */
QStringList svn::cache::LogCache::cachedRepositories()const
{
    static QString s_q(QString("select \"reposroot\" from ")+QString(SQLMAINTABLE)+QString("order by reposroot"));
    QDataBase mainDB = m_CacheData->getMainDB();
    QStringList _res;
#if QT_VERSION < 0x040000
    if (!mainDB || !mainDB->open()) {
#else
    if (!mainDB.isValid()) {
#endif
        qWarning("Failed to open main database.");
        return _res;
    }
    QSqlQuery cur(QString::null,mainDB);
    cur.prepare(s_q);
    if (!cur.exec()) {
        qDebug(cur.lastError().text().TOUTF8().data());
        throw svn::cache::DatabaseException(QString("Could not retrieve values: ")+cur.lastError().text());
        return _res;
    }
    while (cur.next()) {
        _res.append(cur.value(0).toString());
    }

    return _res;
}
