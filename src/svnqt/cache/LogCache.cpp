/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#include "LogCache.h"

#include <QDebug>
#include <QDir>
#include <QMap>
#include <QMutex>
#include <QThreadStorage>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "svnqt/path.h"
#include "svnqt/cache/DatabaseException.h"

static QString SQLTYPE() { return QStringLiteral("QSQLITE"); }
static QString SQLMAIN() { return QStringLiteral("logmain-logcache"); }
static QString SQLMAINTABLE() { return QStringLiteral("logdb"); }
static QString SQLTMPDB() { return QStringLiteral("tmpdb"); }
static QString SQLREPOSPARAMETER() { return QStringLiteral("repoparameter"); }
static QString SQLSTATUS() { return QStringLiteral("logstatus"); }

namespace svn
{
namespace cache
{

LogCache *LogCache::mSelf = nullptr;

class ThreadDBStore
{
public:
    ThreadDBStore()
    {
        m_DB = QSqlDatabase();
    }
    ~ThreadDBStore()
    {
        m_DB.commit();
        m_DB.close();
        m_DB = QSqlDatabase();
        for (const QString &dbName : reposCacheNames) {
            if (QSqlDatabase::database(dbName).isOpen()) {
                QSqlDatabase::database(dbName).commit();
                QSqlDatabase::database(dbName).close();
            }
            QSqlDatabase::removeDatabase(dbName);
        }
        QSqlDatabase::removeDatabase(key);
    }

    void deleteDb(const QString &path)
    {
        for (auto it = reposCacheNames.begin(); it != reposCacheNames.end(); ++it) {
            QSqlDatabase _db = QSqlDatabase::database(it.value());
            if (_db.databaseName() == path) {
                qDebug() << "Removing database " << _db.databaseName() << endl;
                if (_db.isOpen()) {
                    _db.commit();
                    _db.close();
                }
                _db = QSqlDatabase();
                QSqlDatabase::removeDatabase(it.value());
                reposCacheNames.erase(it);
                break;
            }
        }
    }
    QSqlDatabase m_DB;
    QString key;
    QMap<QString, QString> reposCacheNames;
};

class LogCacheData
{

protected:
    QMutex m_singleDbMutex;

public:
    LogCacheData() {}
    ~LogCacheData()
    {
        if (m_mainDB.hasLocalData()) {
            m_mainDB.localData()->m_DB.close();
            m_mainDB.setLocalData(nullptr);
        }
    }

    QString idToPath(const QString &id) const
    {
        return m_BasePath + QLatin1Char('/') + id + QLatin1String(".db");
    }

    bool deleteRepository(const QString &aRepository)
    {
        const QString id = getReposId(aRepository);

        static const QString s_q(QLatin1String("delete from ") + SQLREPOSPARAMETER() + QLatin1String(" where id = ?"));
        static const QString r_q(QLatin1String("delete from ") + SQLMAINTABLE() + QLatin1String(" where id = ?"));
        QSqlDatabase mainDB = getMainDB();
        if (!mainDB.isValid()) {
            qWarning("Failed to open main database.");
            return false;
        }
        qDebug() << m_mainDB.localData()->reposCacheNames;
        m_mainDB.localData()->deleteDb(idToPath(id));
        qDebug() << m_mainDB.localData()->reposCacheNames;
        QFile fi(idToPath(id));
        if (fi.exists()) {
            if (!fi.remove()) {
                qWarning() << "Could not delete " << fi.fileName();
                return false;
            }
        }
        qDebug() << "Removed " << fi.fileName() << endl;
        mainDB.transaction();
        QSqlQuery _q(mainDB);
        _q.prepare(s_q);
        _q.bindValue(0, id);
        if (!_q.exec()) {
            qDebug() << "Error delete value: " << _q.lastError().text() << "(" << _q.lastQuery() << ")";
            _q.finish();
            mainDB.rollback();
            return false;
        }
        _q.prepare(r_q);
        _q.bindValue(0, id);
        if (!_q.exec()) {
            qDebug() << "Error delete value: " << _q.lastError().text() << "(" << _q.lastQuery() << ")";
            _q.finish();
            mainDB.rollback();
            return false;
        }
        mainDB.commit();
        return true;
    }

    bool checkReposDb(QSqlDatabase aDb)
    {
        if (!aDb.open()) {
            return false;
        }

        QSqlQuery _q(aDb);
        QStringList list = aDb.tables();

        aDb.transaction();
        if (!list.contains(QStringLiteral("logentries"))) {
            _q.exec(QStringLiteral("CREATE TABLE \"logentries\" (\"idx\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \"revision\" INTEGER UNIQUE,\"date\" INTEGER,\"author\" TEXT, \"message\" TEXT)"));
        }
        if (!list.contains(QStringLiteral("changeditems"))) {
            _q.exec(QStringLiteral("CREATE TABLE \"changeditems\" (\"idx\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \"revision\" INTEGER,\"changeditem\" TEXT,\"action\" TEXT,\"copyfrom\" TEXT,\"copyfromrev\" INTEGER, UNIQUE(revision,changeditem,action))"));
        }
        if (!list.contains(QStringLiteral("mergeditems"))) {
            _q.exec(QStringLiteral("CREATE TABLE \"mergeditems\" (\"revision\" INTEGER,\"mergeditems\" TEXT, PRIMARY KEY(revision))"));
        }
        if (!list.contains(QStringLiteral("dbversion"))) {
            _q.exec(QStringLiteral("CREATE TABLE \"dbversion\" (\"version\" INTEGER)"));
            qDebug() << _q.lastError();
            _q.exec(QStringLiteral("INSERT INTO \"dbversion\" (version) VALUES(0)"));
        }
        aDb.commit();
        list = aDb.tables();
        if (!list.contains(QStringLiteral("logentries")) ||
            !list.contains(QStringLiteral("changeditems")) ||
            !list.contains(QStringLiteral("mergeditems")) ||
            !list.contains(QStringLiteral("dbversion"))) {
            qDebug() << "lists: " << list;
            return false;
        }
        _q.exec(QStringLiteral("SELECT VERSION from dbversion limit 1"));
        if (_q.lastError().type() == QSqlError::NoError && _q.next()) {
            int version = _q.value(0).toInt();
            if (version == 0) {
                _q.exec(QStringLiteral("create index if not exists main.authorindex on logentries(author)"));
                if (_q.lastError().type() != QSqlError::NoError) {
                    qDebug() << _q.lastError();
                } else {
                    _q.exec(QStringLiteral("UPDATE dbversion SET VERSION=1"));
                }
                ++version;
            }
            if (version == 1) {
                _q.exec(QStringLiteral("create index if not exists main.dateindex on logentries(date)"));
                if (_q.lastError().type() != QSqlError::NoError) {
                    qDebug() << _q.lastError();
                } else {
                    _q.exec(QStringLiteral("UPDATE dbversion SET VERSION=2"));
                }
                ++version;
            }
        } else {
            qDebug() << "Select: " << _q.lastError();
        }
        return true;
    }

    QString createReposDB(const svn::Path &reposroot)
    {
        QMutexLocker locker(&m_singleDbMutex);

        QSqlDatabase _mdb = getMainDB();

        _mdb.transaction();
        QSqlQuery query(_mdb);
        QString q(QLatin1String("insert into ") + SQLMAINTABLE() + QLatin1String(" (reposroot) VALUES('") + reposroot.path() + QLatin1String("')"));

        if (!query.exec(q)) {
            return QString();
        }

        _mdb.commit();
        query.prepare(reposSelect());
        query.bindValue(0, reposroot.native());
        QString db;
        if (query.exec() && query.next()) {
            db = query.value(0).toString();
        } else {
            //qDebug() << "Error select_01: " << query.lastError().text() << "(" << query.lastQuery() << ")";
        }
        if (!db.isEmpty()) {
            QString fulldb = idToPath(db);
            QSqlDatabase _db = QSqlDatabase::addDatabase(SQLTYPE(), SQLTMPDB());
            _db.setDatabaseName(fulldb);
            if (!checkReposDb(_db)) {
            }
            _db = QSqlDatabase();
            QSqlDatabase::removeDatabase(SQLTMPDB());
        }
        return db;
    }

    QString getReposId(const svn::Path &reposroot)
    {
        if (!getMainDB().isValid()) {
            return QString();
        }
        QSqlQuery c(getMainDB());
        c.prepare(reposSelect());
        c.bindValue(0, reposroot.native());

        // only the first one
        if (c.exec() && c.next()) {
            return c.value(0).toString();
        }
        return QString();
    }

    QSqlDatabase getReposDB(const svn::Path &reposroot)
    {
        if (!getMainDB().isValid()) {
            return QSqlDatabase();
        }
        QString dbFile = getReposId(reposroot);

        if (dbFile.isEmpty()) {
            dbFile = createReposDB(reposroot);
            if (dbFile.isEmpty()) {
                return QSqlDatabase();
            }
        }
        if (m_mainDB.localData()->reposCacheNames.find(dbFile) != m_mainDB.localData()->reposCacheNames.end()) {
            QSqlDatabase db = QSqlDatabase::database(m_mainDB.localData()->reposCacheNames.value(dbFile));
            checkReposDb(db);
            return db;
        }
        unsigned i = 0;
        QString _key = dbFile;
        while (QSqlDatabase::contains(_key)) {
            _key = QStringLiteral("%1-%2").arg(dbFile).arg(i++);
        }
        const QString fulldb = idToPath(dbFile);
        QSqlDatabase db = QSqlDatabase::addDatabase(SQLTYPE(), _key);
        db.setDatabaseName(fulldb);
        if (!checkReposDb(db)) {
            db = QSqlDatabase();
        } else {
            m_mainDB.localData()->reposCacheNames[dbFile] = _key;
        }
        return db;
    }

    QSqlDatabase getMainDB()const
    {
        if (!m_mainDB.hasLocalData()) {
            unsigned i = 0;
            QString _key = SQLMAIN();
            while (QSqlDatabase::contains(_key)) {
                _key = QStringLiteral("%1-%2").arg(SQLMAIN()).arg(i++);
            }
            QSqlDatabase db = QSqlDatabase::addDatabase(SQLTYPE(), _key);
            db.setDatabaseName(m_BasePath + QLatin1String("/maindb.db"));
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

    mutable QThreadStorage<ThreadDBStore *> m_mainDB;

    static QString reposSelect()
    {
        return QStringLiteral("SELECT id from ") +
            SQLMAINTABLE() +
            QStringLiteral(" where reposroot=? ORDER by id DESC");
    }
};

/*!
    \fn svn::cache::LogCache::LogCache()
 */
LogCache::LogCache()
    : m_BasePath(QDir::homePath() + QLatin1String("/.svnqt"))
{
    setupCachePath();
}

LogCache::LogCache(const QString &aBasePath)
{
    delete mSelf;
    mSelf = this;
    if (aBasePath.isEmpty()) {
        m_BasePath = QDir::homePath() + QLatin1String("/.svnqt");
    } else {
        m_BasePath = aBasePath;
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
    m_CacheData.reset(new LogCacheData);
    m_CacheData->m_BasePath = m_BasePath;
    QDir d;
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    m_BasePath = m_BasePath + QLatin1Char('/') + QLatin1String("logcache");
    if (!d.exists(m_BasePath)) {
        d.mkdir(m_BasePath);
    }
    m_CacheData->m_BasePath = m_BasePath;
    if (d.exists(m_BasePath)) {
        setupMainDb();
    }
}

void LogCache::setupMainDb()
{
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
    } else {
        const QStringList list = mainDB.tables();
        QSqlQuery q(mainDB);
        if (!list.contains(SQLSTATUS())) {
            mainDB.transaction();
            if (q.exec(QLatin1String("CREATE TABLE \"") + SQLSTATUS() + QLatin1String("\" (\"key\" TEXT PRIMARY KEY NOT NULL, \"value\" TEXT);"))) {
                q.exec(QLatin1String("INSERT INTO \"") + SQLSTATUS() + QLatin1String("\" (key,value) values(\"version\",\"0\");"));
            }
            mainDB.commit();
        }
        int version = databaseVersion();
        if (version == 0) {
            mainDB.transaction();
            if (!list.contains(SQLMAINTABLE())) {
                q.exec(QLatin1String("CREATE TABLE IF NOT EXISTS \"") + SQLMAINTABLE() + QLatin1String("\" (\"reposroot\" TEXT,\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);"));
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
            if (!q.exec(QLatin1String("CREATE TABLE IF NOT EXISTS \"") + SQLREPOSPARAMETER() +
                        QLatin1String("\" (\"id\" INTEGER NOT NULL, \"parameter\" TEXT, \"value\" TEXT, PRIMARY KEY(\"id\",\"parameter\"));"))) {
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
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return;
    }
    static const QString _qs(QLatin1String("update \"") + SQLSTATUS() + QLatin1String("\" SET value = ? WHERE \"key\" = \"version\""));
    QSqlQuery cur(mainDB);
    cur.prepare(_qs);
    cur.bindValue(0, newversion);
    if (!cur.exec()) {
        qDebug() << "Error set version: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
    }
}

int LogCache::databaseVersion()const
{
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return -1;
    }
    static const QString _qs(QLatin1String("select value from \"") + SQLSTATUS() + QLatin1String("\" WHERE \"key\" = \"version\""));
    QSqlQuery cur(mainDB);
    cur.prepare(_qs);
    if (!cur.exec()) {
        qDebug() << "Error select version: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
        return -1;
    }
    if (cur.isActive() && cur.next()) {
        //qDebug("Sel result: %s",_q.value(0).toString().toUtf8().data());
        return cur.value(0).toInt();
    }
    return -1;
}

QVariant LogCache::getRepositoryParameter(const svn::Path &repository, const QString &key)const
{
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return QVariant();
    }
    static const QString qs(QLatin1String("select \"value\",\"repoparameter\".\"parameter\" as \"key\" from \"") + SQLREPOSPARAMETER() +
                            QLatin1String("\" INNER JOIN \"") + SQLMAINTABLE() + QLatin1String("\" ON (\"") + SQLREPOSPARAMETER() +
                            QLatin1String("\".id = \"") + SQLMAINTABLE() + QLatin1String("\".id and \"") + SQLMAINTABLE() +
                            QLatin1String("\".reposroot = ?)  WHERE \"parameter\" = ?;"));
    QSqlQuery cur(mainDB);
    cur.prepare(qs);
    cur.bindValue(0, repository.native());
    cur.bindValue(1, key);
    if (!cur.exec()) {
        qWarning() << "Error select: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
        return QVariant();
    }
    if (cur.isActive() && cur.next()) {
        return cur.value(0);
    }
    return QVariant();
}

bool LogCache::setRepositoryParameter(const svn::Path &repository, const QString &key, const QVariant &value)
{
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    if (!mainDB.isValid()) {
        return false;
    }
    QString id = m_CacheData->getReposId(repository);
    if (id.isEmpty()) {
        return false;
    }
    static const QString qs(QLatin1String("INSERT OR REPLACE INTO \"") + SQLREPOSPARAMETER() +
                            QLatin1String("\" (\"id\",\"parameter\",\"value\") values (\"%1\",\"%2\",?);"));
    static const QString dqs(QLatin1String("DELETE FROM \"") + SQLREPOSPARAMETER() +
                             QLatin1String("\" WHERE \"id\"=? and \"parameter\" = ?"));
    mainDB.transaction();
    QSqlQuery cur(mainDB);
    if (value.isValid()) {
        QString _qs = qs.arg(id,key);//.arg(value.toByteArray());
        cur.prepare(_qs);
        cur.bindValue(0, value);
        if (!cur.exec()) {
            qDebug() << "Error insert new value: " << cur.lastError().text() << "(" << cur.lastQuery() << ")";
            cur.finish();
            mainDB.rollback();
            return false;
        }
    } else {
        cur.prepare(dqs);
        cur.bindValue(0, id);
        cur.bindValue(1, key);
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
svn::cache::LogCache *svn::cache::LogCache::self()
{
    if (!mSelf) {
        mSelf = new LogCache();
    }
    return mSelf;
}

/*!
    \fn svn::cache::LogCache::reposDb()
 */
QSqlDatabase  svn::cache::LogCache::reposDb(const QString &aRepository)
{
//    //qDebug("reposDB");
    return m_CacheData->getReposDB(aRepository);
}

/*!
    \fn svn::cache::LogCache::cachedRepositories()const
 */
QStringList svn::cache::LogCache::cachedRepositories()const
{
    static const QString s_q(QLatin1String("select \"reposroot\" from ") + SQLMAINTABLE() + QLatin1String(" order by reposroot"));
    QSqlDatabase mainDB = m_CacheData->getMainDB();
    QStringList _res;
    if (!mainDB.isValid()) {
        qWarning("Failed to open main database.");
        return _res;
    }
    QSqlQuery cur(mainDB);
    cur.prepare(s_q);
    if (!cur.exec()) {
        throw svn::cache::DatabaseException(QLatin1String("Could not retrieve values: ") + cur.lastError().text());
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

bool svn::cache::LogCache::deleteRepository(const QString &aRepository)
{
    return m_CacheData->deleteRepository(aRepository);
}
