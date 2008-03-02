/****************************************************************************
**
** Definition of SQLite driver classes.
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

#ifndef QSQL_SQLITE3_H
#define QSQL_SQLITE3_H

#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include "qsqlcachedresult.h"

#if (QT_VERSION-0 >= 0x030200)
typedef QVariant QSqlVariant;
#endif

#if defined (Q_OS_WIN32)
# include <qt_windows.h>
#endif

class QSQLite3DriverPrivate;
class QSQLite3ResultPrivate;
class QSQLite3Driver;
struct sqlite3;

class QSQLite3Result : public QSqlCachedResult
{
    friend class QSQLite3Driver;
    friend class QSQLite3ResultPrivate;
public:
    QSQLite3Result(const QSQLite3Driver* db);
    ~QSQLite3Result();

protected:
    bool gotoNext(QSqlCachedResult::ValueCache& row, int idx);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();

private:
    QSQLite3ResultPrivate* d;
};

class QSQLite3Driver : public QSqlDriver
{
    friend class QSQLite3Result;
public:
    QSQLite3Driver(QObject *parent = 0, const char *name = 0);
    QSQLite3Driver(sqlite3 *connection, QObject *parent = 0, const char *name = 0);
    ~QSQLite3Driver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts);
    bool open( const QString & db,
               const QString & user,
               const QString & password,
               const QString & host,
               int port ) { return open (db, user, password, host, port, QString()); }
    void close();
    QSqlQuery createQuery() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(const QString &user) const;

    QSqlRecord record(const QString& tablename) const;
    QSqlRecordInfo recordInfo(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;
    QSqlRecord record(const QSqlQuery& query) const;
    QSqlRecordInfo recordInfo(const QSqlQuery& query) const;

private:
    QSQLite3DriverPrivate* d;
};
#endif
