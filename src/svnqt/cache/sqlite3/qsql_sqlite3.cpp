/****************************************************************************
**
** Implementation of SQLite driver classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_sqlite3.h"

#include <qdatetime.h>
#include <qvaluevector.h>
#include <qregexp.h>
#include <qfile.h>
#include <sqlite3.h>

#if (QT_VERSION-0 < 0x030200)
#  include <qvector.h>
#  if !defined Q_WS_WIN32
#    include <unistd.h>
#  endif
#else
#  include <qptrvector.h>
#  if !defined Q_WS_WIN32
#    include <unistd.h>
#  endif
#endif

typedef struct sqlite3_stmt sqlite3_stmt;

#define QSQLITE3_DRIVER_NAME "QSQLITE3"

static QVariant::Type qSqliteType(int tp)
{
    switch (tp) {
    case SQLITE_INTEGER:
        return QVariant::Int;
    case SQLITE_FLOAT:
        return QVariant::Double;
    case SQLITE_BLOB:
        return QVariant::ByteArray;
    case SQLITE_TEXT:
    default:
        return QVariant::String;
    }
}

static QSqlError qMakeError(sqlite3 *access, const QString &descr, QSqlError::Type type,
                            int errorCode = -1)
{
    return QSqlError(descr,
                     QString::fromUtf8(sqlite3_errmsg(access)),
                     type, errorCode);
}

class QSQLite3DriverPrivate
{
public:
    QSQLite3DriverPrivate();
    sqlite3 *access;
    bool utf8;
};

QSQLite3DriverPrivate::QSQLite3DriverPrivate() : access(0)
{
    utf8 = true;
}

class QSQLite3ResultPrivate
{
public:
    QSQLite3ResultPrivate(QSQLite3Result *res);
    void cleanup();
    bool fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch);
    bool isSelect();
    // initializes the recordInfo and the cache
    void initColumns();
    void finalize();

    QSQLite3Result* q;
    sqlite3 *access;

    sqlite3_stmt *stmt;

    uint skippedStatus: 1; // the status of the fetchNext() that's skipped
    uint skipRow: 1; // skip the next fetchNext()?
    uint utf8: 1;
    QSqlRecord rInf;
};

static const uint initial_cache_size = 128;

QSQLite3ResultPrivate::QSQLite3ResultPrivate(QSQLite3Result* res) : q(res), access(0),
    stmt(0), skippedStatus(false), skipRow(false), utf8(false)
{
}

void QSQLite3ResultPrivate::cleanup()
{
    finalize();
    rInf.clear();
    skippedStatus = false;
    skipRow = false;
    q->setAt(QSql::BeforeFirst);
    q->setActive(false);
    q->cleanup();
}

void QSQLite3ResultPrivate::finalize()
{
    if (!stmt)
        return;

    sqlite3_finalize(stmt);
    stmt = 0;
}

// called on first fetch
void QSQLite3ResultPrivate::initColumns()
{
    rInf.clear();

    int nCols = sqlite3_column_count(stmt);
    if (nCols <= 0)
        return;

    q->init(nCols);

    for (int i = 0; i < nCols; ++i) {
        QString colName = QString::fromUtf8(sqlite3_column_name(stmt, i));

        int dotIdx = colName.findRev('.');
        rInf.append(QSqlField(colName.mid(dotIdx == -1 ? 0 : dotIdx + 1),
                qSqliteType(sqlite3_column_type(stmt, i))));
    }
}

bool QSQLite3ResultPrivate::fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch)
{
    int res;
    unsigned int i;

    if (skipRow) {
        // already fetched
        Q_ASSERT(!initialFetch);
        skipRow = false;
        return skippedStatus;
    }
    skipRow = initialFetch;

    if (!stmt)
        return false;

    // keep trying while busy, wish I could implement this better.
    while ((res = sqlite3_step(stmt)) == SQLITE_BUSY) {
        // sleep instead requesting result again immidiately.
#if defined Q_OS_WIN
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    switch(res) {
    case SQLITE_ROW:
        // check to see if should fill out columns
        if (rInf.isEmpty())
            // must be first call.
            initColumns();
        if (idx < 0 && !initialFetch)
            return true;
        for (i = 0; i < rInf.count(); ++i)
            // todo - handle other types
            values[i + idx] = QString::fromUtf8((char *)(sqlite3_column_text(stmt, i)));
 //           values[i + idx] = utf8 ? QString::fromUtf8(fvals[i]) : QString::fromAscii(fvals[i]);
        return true;
    case SQLITE_DONE:
        if (rInf.isEmpty())
            // must be first call.
            initColumns();
        q->setAt(QSql::AfterLast);
        return false;
    case SQLITE_ERROR:
    case SQLITE_MISUSE:
    default:
        // something wrong, don't get col info, but still return false
        q->setLastError(qMakeError(access, "Unable to fetch row", QSqlError::Connection, res));
        finalize();
        q->setAt(QSql::AfterLast);
        return false;
    }
    return false;
}

QSQLite3Result::QSQLite3Result(const QSQLite3Driver* db)
    : QSqlCachedResult(db)
{
    d = new QSQLite3ResultPrivate(this);
    d->access = db->d->access;
}

QSQLite3Result::~QSQLite3Result()
{
    d->cleanup();
    delete d;
}

/*
   Execute \a query.
*/
bool QSQLite3Result::reset (const QString &query)
{
    // this is where we build a query.
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;

    d->cleanup();

    setSelect(false);

    int res = sqlite3_prepare_v2(d->access, query.utf8().data(), (query.length() + 1) * sizeof(QChar),
                                &d->stmt, 0);

    if (res != SQLITE_OK) {
        setLastError(qMakeError(d->access, "Unable to execute statement", QSqlError::Statement, res));
        d->finalize();
        return false;
    }

    d->skippedStatus = d->fetchNext(cache(), 0, true);

    setSelect(!d->rInf.isEmpty());
    setActive(true);
    return true;
}

bool QSQLite3Result::gotoNext(QSqlCachedResult::ValueCache& row, int idx)
{
    return d->fetchNext(row, idx, false);
}

int QSQLite3Result::size()
{
    return -1;
}

int QSQLite3Result::numRowsAffected()
{
    return sqlite3_changes(d->access);
}

/////////////////////////////////////////////////////////

QSQLite3Driver::QSQLite3Driver(QObject * parent, const char *name)
    : QSqlDriver(parent, name)
{
    d = new QSQLite3DriverPrivate();
}

QSQLite3Driver::QSQLite3Driver(sqlite3 *connection, QObject *parent, const char *name)
    : QSqlDriver(parent, name)
{
    d = new QSQLite3DriverPrivate();
    d->access = connection;
    setOpen(true);
    setOpenError(false);
}


QSQLite3Driver::~QSQLite3Driver()
{
    delete d;
}

bool QSQLite3Driver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
    case Unicode:
    case BLOB:
        return true;
    default:
        break;
    }
    return false;
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSQLite3Driver::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &)
{
    if (isOpen())
        close();

    if (db.isEmpty())
        return false;

    if (sqlite3_open(QFile::encodeName(db), &d->access) == SQLITE_OK) {
        setOpen(true);
        setOpenError(false);
        return true;
    } else {
        setLastError(qMakeError(d->access, "Error opening database",
                     QSqlError::Connection));
        setOpenError(true);
        return false;
    }
}

void QSQLite3Driver::close()
{
    if (isOpen()) {
        if (sqlite3_close(d->access) != SQLITE_OK)
            setLastError(qMakeError(d->access, "Error closing database",
                                    QSqlError::Connection));
        d->access = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlQuery QSQLite3Driver::createQuery() const
{
    return QSqlQuery(new QSQLite3Result(this));
}

bool QSQLite3Driver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createQuery());
    if (!q.exec("BEGIN")) {
        setLastError(QSqlError("Unable to begin transaction",
                               q.lastError().databaseText(), QSqlError::Transaction));
        return false;
    }

    return true;
}

bool QSQLite3Driver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createQuery());
    if (!q.exec("COMMIT")) {
        setLastError(QSqlError("Unable to begin transaction",
                               q.lastError().databaseText(), QSqlError::Transaction));
        return false;
    }

    return true;
}

bool QSQLite3Driver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createQuery());
    if (!q.exec("ROLLBACK")) {
        setLastError(QSqlError("Unable to begin transaction",
                               q.lastError().databaseText(), QSqlError::Transaction));
        return false;
    }

    return true;
}

QStringList QSQLite3Driver::tables(const QString &typeName) const
{
    QStringList res;
    if (!isOpen())
        return res;
    int type = typeName.toInt();

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
#if (QT_VERSION-0 >= 0x030200)
    if ((type & (int)QSql::Tables) && (type & (int)QSql::Views))
        q.exec("SELECT name FROM sqlite_master WHERE type='table' OR type='view'");
    else if (typeName.isEmpty() || (type & (int)QSql::Tables))
        q.exec("SELECT name FROM sqlite_master WHERE type='table'");
    else if (type & (int)QSql::Views)
        q.exec("SELECT name FROM sqlite_master WHERE type='view'");
#else
        q.exec("SELECT name FROM sqlite_master WHERE type='table' OR type='view'");
#endif


    if (q.isActive()) {
        while(q.next())
            res.append(q.value(0).toString());
    }

#if (QT_VERSION-0 >= 0x030200)
    if (type & (int)QSql::SystemTables) {
        // there are no internal tables beside this one:
        res.append("sqlite_master");
    }
#endif

    return res;
}

QSqlIndex QSQLite3Driver::primaryIndex(const QString &tblname) const
{
    QSqlRecordInfo rec(recordInfo(tblname)); // expensive :(

    if (!isOpen())
        return QSqlIndex();

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    // finrst find a UNIQUE INDEX
    q.exec("PRAGMA index_list('" + tblname + "');");
    QString indexname;
    while(q.next()) {
	if (q.value(2).toInt()==1) {
	    indexname = q.value(1).toString();
	    break;
	}
    }
    if (indexname.isEmpty())
	return QSqlIndex();

    q.exec("PRAGMA index_info('" + indexname + "');");

    QSqlIndex index(indexname);
    while(q.next()) {
	QString name = q.value(2).toString();
	QSqlVariant::Type type = QSqlVariant::Invalid;
	if (rec.contains(name))
	    type = rec.find(name).type();
	index.append(QSqlField(name, type));
    }
    return index;
}

QSqlRecordInfo QSQLite3Driver::recordInfo(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecordInfo();

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    q.exec("SELECT * FROM " + tbl + " LIMIT 1");
    return recordInfo(q);
}

QSqlRecord QSQLite3Driver::record(const QString &tblname) const
{
    if (!isOpen())
        return QSqlRecord();

    return recordInfo(tblname).toRecord();
}

QSqlRecord QSQLite3Driver::record(const QSqlQuery& query) const
{
    if (query.isActive() && query.driver() == this) {
        QSQLite3Result* result = (QSQLite3Result*)query.result();
        return result->d->rInf;
    }
    return QSqlRecord();
}

QSqlRecordInfo QSQLite3Driver::recordInfo(const QSqlQuery& query) const
{
    if (query.isActive() && query.driver() == this) {
        QSQLite3Result* result = (QSQLite3Result*)query.result();
        return QSqlRecordInfo(result->d->rInf);
    }
    return QSqlRecordInfo();
}
