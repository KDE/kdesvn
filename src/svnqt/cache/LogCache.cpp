#include "LogCache.hpp"

#include <qdir.h>
#include <qsql.h>
#include <qsqldatabase.h>

#include <QMutex>
#include <QThreadStorage>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <qmap.h>
#include <QtDebug>

#include "svnqt/path.hpp"
#include "svnqt/cache/DatabaseException.hpp"

#define SQLTYPE "QSQLITE"
#define SQLMAIN "logmain-logcache"
#define SQLMAINTABLE "logdb"

namespace svn {
namespace cache {

LogCache* LogCache::mSelf = 0;

class ThreadDBStore
{
public:
    ThreadDBStore(){
        m_DB=QSqlDatabase();
    }
    ~ThreadDBStore(){
        m_DB.close();
        m_DB=QSqlDatabase();
        QSqlDatabase::removeDatabase(key);
        QMap<QString,QString>::Iterator it;
        for (it=reposCacheNames.begin();it!=reposCacheNames.end();++it) {
            QSqlDatabase::database(it.value()).close();
            QSqlDatabase::removeDatabase(it.value());
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
            m_mainDB.localData()->m_DB.close();
            m_mainDB.setLocalData(0L);
        }
    }

    bool checkReposDb(QDataBase aDb)
    {
        if (!aDb.open()) {
            return false;
        }

        QSqlQuery _q(QString::null, aDb);
        QStringList list = aDb.tables();

        if (list.indexOf("logentries")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"logentries\" (\"revision\" INTEGER UNIQUE,\"date\" INTEGER,\"author\" TEXT, \"message\" TEXT)");
            aDb.commit();
        }
        if (list.indexOf("changeditems")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"changeditems\" (\"revision\" INTEGER,\"changeditem\" TEXT,\"action\" TEXT,\"copyfrom\" TEXT,\"copyfromrev\" INTEGER, PRIMARY KEY(revision,changeditem,action))");
            aDb.commit();
        }
        if (list.indexOf("mergeditems")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"mergeditems\" (\"revision\" INTEGER,\"mergeditems\" TEXT, PRIMARY KEY(revision))");
            aDb.commit();
        }
        list = aDb.tables();
        if (list.indexOf("logentries")==-1 || list.indexOf("changeditems")==-1 || list.indexOf("mergeditems")==-1) {
            return false;
        }
        return true;
    }

    QString createReposDB(const svn::Path&reposroot) {
        QMutexLocker locker( &m_singleDbMutex );

        QDataBase _mdb = getMainDB();

        QSqlQuery query1(QString::null,_mdb);
        QString q("insert into "+QString(SQLMAINTABLE)+" (reposroot) VALUES('"+reposroot+"')");
        _mdb.transaction();

        query1.exec(q);
        _mdb.commit();
        QSqlQuery query(QString::null,_mdb);
        query.prepare(s_reposSelect);
        query.bindValue(0,reposroot.native());
        query.exec();
        QString db;
        if (query.lastError().type()==QSqlError::NoError && query.next()) {
            db = query.value(0).toString();
        }
        else {
            //qDebug() << "Error select_01: " << query.lastError().text() << "(" << query.lastQuery() << ")";
        }
        if (!db.isEmpty()) {
            QString fulldb = m_BasePath+"/"+db+".db";
            QDataBase _db = QSqlDatabase::addDatabase(SQLTYPE,"tmpdb");
            _db.setDatabaseName(fulldb);
            if (!checkReposDb(_db)) {
            }
            QSqlDatabase::removeDatabase("tmpdb");
        }
        return db;
    }

    QDataBase getReposDB(const svn::Path&reposroot) {
        if (!getMainDB().isValid()) {
            return QDataBase();
        }
        bool checkDone = false;
        // make sure path is correct eg. without traling slashes.
        QString dbFile;
        QSqlQuery c(QString::null,getMainDB());
        c.prepare(s_reposSelect);
        c.bindValue(0,reposroot.native());
        c.exec();

        // only the first one
        if ( c.next() ) {
            dbFile = c.value(0).toString();
        }
        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
            if (dbFile.isEmpty()) {
                return QSqlDatabase();
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
        QDataBase _db = QSqlDatabase::addDatabase(SQLTYPE,_key);
        QString fulldb = m_BasePath+"/"+dbFile+".db";
        _db.setDatabaseName(fulldb);
//        //qDebug("try database open %s",fulldb.TOUTF8().data());
        if (!checkReposDb(_db)) {
            //qDebug("no DB opened");
            _db = QSqlDatabase();
        } else {
            //qDebug("Insert into map");
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
            //qDebug("The key is now: %s",_key.TOUTF8().data());

            QDataBase db = QSqlDatabase::addDatabase(SQLTYPE,_key);
            db.setDatabaseName(m_BasePath+"/maindb.db");
            if (db.open()) {
                m_mainDB.setLocalData(new ThreadDBStore);
                m_mainDB.localData()->key = _key;
                m_mainDB.localData()->m_DB = db;
            }
        }
        if (m_mainDB.hasLocalData()) {
            return m_mainDB.localData()->m_DB;
        } else {
            return QSqlDatabase();
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
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
    } else {
        QSqlQuery q(QString::null, mainDB);
        mainDB.transaction();
        if (!q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY NOT NULL);")) {
        }
        mainDB.commit();
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
//    //qDebug("reposDB");
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
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
        return _res;
    }
    QSqlQuery cur(QString::null,mainDB);
    cur.prepare(s_q);
    if (!cur.exec()) {
        //qDebug() << cur.lastError().text();
        throw svn::cache::DatabaseException(QString("Could not retrieve values: ")+cur.lastError().text());
        return _res;
    }
    while (cur.next()) {
        _res.append(cur.value(0).toString());
    }

    return _res;
}

bool svn::cache::LogCache::valid()const
{
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return false;
    }
    return true;
}
