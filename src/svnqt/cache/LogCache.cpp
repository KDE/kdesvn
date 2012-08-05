/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "LogCache.h"

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

#include "svnqt/path.h"
#include "svnqt/cache/DatabaseException.h"

#define SQLTYPE "QSQLITE"
#define SQLMAIN "logmain-logcache"
#define SQLMAINTABLE "logdb"
#define SQLREPOSPARAMETER "repoparameter"
#define SQLSTATUS QString("logstatus")

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
        m_DB.commit();
        m_DB.close();
        m_DB=QSqlDatabase();
        QMap<QString,QString>::Iterator it;
        for (it=reposCacheNames.begin();it!=reposCacheNames.end();++it) {
            if (QSqlDatabase::database(it.value()).isOpen()) {
                QSqlDatabase::database(it.value()).commit();
                QSqlDatabase::database(it.value()).close();
            }
            QSqlDatabase::removeDatabase(it.value());
        }
        QSqlDatabase::removeDatabase(key);
    }

    void deleteDb(const QString&path)
    {
        QMap<QString,QString>::Iterator it;
        for (it=reposCacheNames.begin();it!=reposCacheNames.end();++it) {
            QSqlDatabase _db = QSqlDatabase::database(it.value());
            if (_db.databaseName()==path) {
                qDebug()<<"Removing database "<<_db.databaseName()<<endl;
                if (_db.isOpen()) {
                    _db.commit();
                    _db.close();
                }
                QSqlDatabase::removeDatabase(it.value());
                it = reposCacheNames.begin();
            }
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

    QString idToPath(const QString&id)
    {
        return m_BasePath+'/'+id+".db";
    }

    bool deleteRepository(const QString&aRepository)
    {
        QString id = getReposId(aRepository);

        static QString s_q(QString("delete from ")+QString(SQLREPOSPARAMETER)+" where id = ?");
        static QString r_q(QString("delete from ")+QString(SQLMAINTABLE)+" where id = ?");
        QDataBase mainDB = getMainDB();
        if (!mainDB.isValid()) {
            qWarning("Failed to open main database.");
            return false;
        }
        qDebug()<<m_mainDB.localData()->reposCacheNames;
        m_mainDB.localData()->deleteDb(idToPath(id));
        qDebug()<<m_mainDB.localData()->reposCacheNames;
        QFile fi(idToPath(id));
        if (fi.exists()) {
            if (!fi.remove()) {
                qWarning()<<"Could not delete "<< fi.fileName();
                return false;
            }
        }
        qDebug()<<"Removed "<<fi.fileName()<<endl;
        mainDB.transaction();
        QSqlQuery _q(QString(),mainDB);
        _q.prepare(s_q);
        _q.bindValue(0,id);
        if (!_q.exec()) {
            qDebug() << "Error delete value: " << _q.lastError().text() << "(" << _q.lastQuery() << ")";
            _q.finish();
            mainDB.rollback();
            return false;
        }
        _q.prepare(r_q);
        _q.bindValue(0,id);
        if (!_q.exec()) {
            qDebug() << "Error delete value: " << _q.lastError().text() << "(" << _q.lastQuery() << ")";
            _q.finish();
            mainDB.rollback();
            return false;
        }
        mainDB.commit();
        return true;
    }

    bool checkReposDb(QDataBase aDb)
    {
        if (!aDb.open()) {
            return false;
        }

        QSqlQuery _q(QString(), aDb);
        QStringList list = aDb.tables();

        if (list.indexOf("logentries")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"logentries\" (\"idx\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \"revision\" INTEGER UNIQUE,\"date\" INTEGER,\"author\" TEXT, \"message\" TEXT)");
            aDb.commit();
        }
        if (list.indexOf("changeditems")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"changeditems\" (\"idx\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \"revision\" INTEGER,\"changeditem\" TEXT,\"action\" TEXT,\"copyfrom\" TEXT,\"copyfromrev\" INTEGER, UNIQUE(revision,changeditem,action))");
            aDb.commit();
        }
        if (list.indexOf("mergeditems")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"mergeditems\" (\"revision\" INTEGER,\"mergeditems\" TEXT, PRIMARY KEY(revision))");
            aDb.commit();
        }
        if (list.indexOf("dbversion")==-1) {
            aDb.transaction();
            _q.exec("CREATE TABLE \"dbversion\" (\"version\" INTEGER)");
            qDebug()<<_q.lastError();
            _q.exec("insert into \"dbversion\" (version) values(0)");
            aDb.commit();
        }
        list = aDb.tables();
        if (list.indexOf("logentries")==-1 || list.indexOf("changeditems")==-1 || list.indexOf("mergeditems")==-1||list.indexOf("dbversion")==-1) {
            qDebug()<<"lists: "<<list;
            return false;
        }
        _q.exec("SELECT VERSION from dbversion limit 1");
        if (_q.lastError().type()==QSqlError::NoError && _q.next()) {
            int version = _q.value(0).toInt();
            if (version == 0){
                _q.exec("create index if not exists main.authorindex on logentries(author)");
                if (_q.lastError().type()!=QSqlError::NoError) {
                    qDebug()<<_q.lastError();
                } else {
                    _q.exec("UPDATE dbversion SET VERSION=1");
                }
                ++version;
            }
            if (version == 1){
                _q.exec("create index if not exists main.dateindex on logentries(date)");
                if (_q.lastError().type()!=QSqlError::NoError) {
                    qDebug()<<_q.lastError();
                } else {
                    _q.exec("UPDATE dbversion SET VERSION=2");
                }
                ++version;
            }
        } else {
            qDebug()<<"Select: "<<_q.lastError();
        }
        return true;
    }

    QString createReposDB(const svn::Path&reposroot) {
        QMutexLocker locker( &m_singleDbMutex );

        QDataBase _mdb = getMainDB();

        QSqlQuery query1(QString(),_mdb);
        QString q("insert into "+QString(SQLMAINTABLE)+" (reposroot) VALUES('"+reposroot+"')");
        _mdb.transaction();

        query1.exec(q);
        _mdb.commit();
        QSqlQuery query(QString(),_mdb);
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
            QString fulldb = idToPath(db);
            QDataBase _db = QSqlDatabase::addDatabase(SQLTYPE,"tmpdb");
            _db.setDatabaseName(fulldb);
            if (!checkReposDb(_db)) {
            }
            QSqlDatabase::removeDatabase("tmpdb");
        }
        return db;
    }

    QString getReposId(const svn::Path&reposroot)
    {
        if (!getMainDB().isValid()) {
            return QString();
        }
        QSqlQuery c(QString(),getMainDB());
        c.prepare(s_reposSelect);
        c.bindValue(0,reposroot.native());
        c.exec();

        // only the first one
        if ( c.next() ) {
            return c.value(0).toString();
        }
        return QString();
    }

    QDataBase getReposDB(const svn::Path&reposroot)
    {
        if (!getMainDB().isValid()) {
            return QDataBase();
        }
        QString dbFile = getReposId(reposroot);

        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
            if (dbFile.isEmpty()) {
                return QSqlDatabase();
            }
        }
        QDataBase _db;
        if (m_mainDB.localData()->reposCacheNames.find(dbFile)!=m_mainDB.localData()->reposCacheNames.end()) {
            _db = QSqlDatabase::database(m_mainDB.localData()->reposCacheNames[dbFile]);
            checkReposDb(_db);
            return _db;
        }
        int i = 0;
        QString _key = dbFile;
        while (QSqlDatabase::contains(_key)) {
            _key = QString("%1-%2").arg(dbFile).arg(i++);
        }
        _db = QSqlDatabase::addDatabase(SQLTYPE,_key);
        QString fulldb = idToPath(dbFile);
        _db.setDatabaseName(fulldb);
        if (!checkReposDb(_db)) {
            _db = QSqlDatabase();
        } else {
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
    m_BasePath=m_BasePath+'/'+s_CACHE_FOLDER;
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
        QStringList list = mainDB.tables();
        QSqlQuery q(QString(), mainDB);
        if (list.indexOf(SQLSTATUS)==-1) {
            mainDB.transaction();
            if (q.exec("CREATE TABLE \""+SQLSTATUS+"\" (\"key\" TEXT PRIMARY KEY NOT NULL, \"value\" TEXT);")) {
                q.exec("INSERT INTO \""+SQLSTATUS+"\" (key,value) values(\"version\",\"0\");");
            }
            mainDB.commit();
        }
        int version = databaseVersion();
        if (version == 0) {
            mainDB.transaction();
            if (list.indexOf(SQLMAINTABLE)==-1) {
                q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);");
            }/* else {
                q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLMAINTABLE)+"new\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);");
                q.exec("insert into \""+QString(SQLMAINTABLE)+"new\" select \"reposroot\",\"id\" from \""+QString(SQLMAINTABLE)+"\");");
                q.exec("drop table \""+QString(SQLMAINTABLE)+"\";");
                q.exec("alter table \""+QString(SQLMAINTABLE)+"new\" to \""+QString(SQLMAINTABLE)+"\";");
            }*/
            ++version;
        }
        if (version == 1) {
            mainDB.transaction();
            if (!q.exec("CREATE TABLE IF NOT EXISTS \""+QString(SQLREPOSPARAMETER)+"\" (\"id\" INTEGER NOT NULL, \"parameter\" TEXT, \"value\" TEXT, PRIMARY KEY(\"id\",\"parameter\"));")) {
                qDebug() << "Error create: " << q.lastError().text() << "(" << q.lastQuery() << ")";
            }
            mainDB.commit();
            ++version;
        }
        databaseVersion(version);
    }
}

void LogCache::databaseVersion(int newversion)
{
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return;
    }
    static QString _qs("update \""+SQLSTATUS+"\" SET value = ? WHERE \"key\" = \"version\"");
    QSqlQuery cur(QString(),mainDB);
    cur.prepare(_qs);
    cur.bindValue(0,newversion);
    if (!cur.exec()) {
        qDebug() << "Error set version: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
    }
}

int LogCache::databaseVersion()const
{
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return -1;
    }
    static QString _qs("select value from \""+SQLSTATUS+"\" WHERE \"key\" = \"version\"");
    QSqlQuery cur(QString(),mainDB);
    cur.prepare(_qs);
    if (!cur.exec()) {
        qDebug() << "Error select version: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
        return -1;
    }
    if (cur.isActive() && cur.next()) {
        //qDebug("Sel result: %s",_q.value(0).toString().TOUTF8().data());
        return cur.value(0).toInt();
    }
    return -1;
}

QVariant LogCache::getRepositoryParameter(const svn::Path&repository,const QString&key)const
{
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return QVariant();
    }
    static QString qs("select \"value\",\"repoparameter\".\"parameter\" as \"key\" from \""+QString(SQLREPOSPARAMETER)+"\" INNER JOIN \""+QString(SQLMAINTABLE)+"\" ON (\""+QString(SQLREPOSPARAMETER)+"\".id = \""+QString(SQLMAINTABLE)+"\".id and \""+QString(SQLMAINTABLE)+"\".reposroot = ?)  WHERE \"parameter\" = ?;");
    QSqlQuery cur(QString(),mainDB);
    cur.prepare(qs);
    cur.bindValue(0,repository.native());
    cur.bindValue(1,key);
    if (!cur.exec()) {
        qWarning() << "Error select: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
        return QVariant();
    }
    if (cur.isActive() && cur.next()) {
        return cur.value(0);
    }
    return QVariant();
}

bool LogCache::setRepositoryParameter(const svn::Path&repository,const QString&key,const QVariant&value)
{
    QDataBase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return false;
    }
    QString id = m_CacheData->getReposId(repository);
    if (id.isEmpty()) {
        return false;
    }
    static QString qs("INSERT OR REPLACE INTO \""+QString(SQLREPOSPARAMETER)+"\" (\"id\",\"parameter\",\"value\") values (\"%1\",\"%2\",?);");
    static QString dqs("DELETE FROM \""+QString(SQLREPOSPARAMETER)+"\" WHERE \"id\"=? and \"parameter\" = ?");
    mainDB.transaction();
    QSqlQuery cur(QString(),mainDB);
    if (value.isValid()) {
        QString _qs = qs.arg(id).arg(key);//.arg(value.toByteArray());
        cur.prepare(_qs);
        cur.bindValue(0,value);
        if (!cur.exec()) {
            qDebug() << "Error insert new value: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
            cur.finish();
            mainDB.rollback();
            return false;
        }
    } else {
        cur.prepare(dqs);
        cur.bindValue(0,id);
        cur.bindValue(1,key);
        if (!cur.exec()) {
            qDebug() << "Error delete value: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
            cur.finish();
            mainDB.rollback();
            return false;
        }
    }
    mainDB.commit();
    return true;
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
    static QString s_q(QString("select \"reposroot\" from ")+QString(SQLMAINTABLE)+QString(" order by reposroot"));
    QDataBase mainDB = m_CacheData->getMainDB();
    QStringList _res;
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
        return _res;
    }
    QSqlQuery cur(QString(),mainDB);
    cur.prepare(s_q);
    if (!cur.exec()) {
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
    return m_CacheData->getMainDB().isValid();
}

bool svn::cache::LogCache::deleteRepository(const QString&aRepository)
{
    return m_CacheData->deleteRepository(aRepository);
}
