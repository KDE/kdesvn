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
#include "ReposLog.h"

#include "LogCache.h"
#include "svnqt/info_entry.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/client.h"
#include "svnqt/context_listener.h"
#include "svnqt/cache/DatabaseException.h"
#include "svnqt/client_parameter.h"

#include <qsqldatabase.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QFileInfo>
#include <QBuffer>
#define Q_LLONG qlonglong

/*!
    \fn svn::cache::ReposLog::ReposLog(svn::Client*aClient,const QString&)
 */
svn::cache::ReposLog::ReposLog(svn::Client*aClient,const QString&aRepository)
    :m_Client(),
              m_Database(),
              m_ReposRoot(aRepository),m_latestHead(svn::Revision::UNDEFINED)
{
    m_Client=aClient;
    if (!aRepository.isEmpty()) {
        m_Database = LogCache::self()->reposDb(aRepository);
    }
}


/*!
    \fn svn::cache::ReposLog::latestHeadRev()
 */
svn::Revision svn::cache::ReposLog::latestHeadRev()
{
    if (!m_Client||m_ReposRoot.isEmpty()) {
        return svn::Revision::UNDEFINED;
    }
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return svn::Revision::UNDEFINED;
        }
    }
    /// no catch - exception has go trough...
    //qDebug("Getting headrev");
    svn::InfoEntries e = m_Client->info(m_ReposRoot,svn::DepthEmpty,svn::Revision::HEAD,svn::Revision::HEAD);
    if (e.count()<1||e[0].reposRoot().isEmpty()) {
        return svn::Revision::UNDEFINED;
    }
    //qDebug("Getting headrev done");
    return e[0].revision();
}


/*!
    \fn svn::cache::ReposLog::latestCachedRev()
 */
svn::Revision svn::cache::ReposLog::latestCachedRev()
{
    if (m_ReposRoot.isEmpty()) {
        return svn::Revision::UNDEFINED;
    }
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return svn::Revision::UNDEFINED;
        }
    }
    QString q("select revision from 'logentries' order by revision DESC limit 1");
    QSqlQuery _q(QString(), m_Database);
    if (!_q.exec(q)) {
        //qDebug() << _q.lastError().text();
        return svn::Revision::UNDEFINED;
    }
    int _r;
    if (_q.isActive() && _q.next()) {
        //qDebug("Sel result: %s",_q.value(0).toString().TOUTF8().data());
        _r = _q.value(0).toInt();
    } else {
        //qDebug() << _q.lastError().text();
        return svn::Revision::UNDEFINED;
    }
    return _r;
}

qlonglong svn::cache::ReposLog::count()const
{
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return svn::Revision::UNDEFINED;
        }
    }
    QString q("select count(*) from 'logentries'");
    QSqlQuery _q(QString(), m_Database);
    if (!_q.exec(q)) {
        //qDebug() << _q.lastError().text();
        return -1;
    }
    qlonglong _r;
    QVariant v;
    if (_q.isActive() && _q.next()) {
        //qDebug("Sel result: %s",_q.value(0).toString().TOUTF8().data());
        v=_q.value(0);
        if (v.canConvert(QVariant::LongLong)) {
            bool ok = false;
            _r = v.toLongLong(&ok);
            if (ok) {
                return _r;
            }
        }
    }
    return -1;
}

qlonglong svn::cache::ReposLog::fileSize()const
{
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return -1;
        }
    }
    QFileInfo fi(m_Database.databaseName());
    if (fi.exists()) {
        return fi.size();
    }
    return -1;
}

qlonglong svn::cache::ReposLog::itemCount()const
{
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return -1;
        }
    }
    QString q("select count(*) from 'changeditems'");
    QSqlQuery _q(QString(), m_Database);
    if (!_q.exec(q)) {
        //qDebug() << _q.lastError().text();
        return -1;
    }
    qlonglong _r;
    QVariant v;
    if (_q.isActive() && _q.next()) {
        //qDebug("Sel result: %s",_q.value(0).toString().TOUTF8().data());
        v=_q.value(0);
        if (v.canConvert(QVariant::LongLong)) {
            bool ok = false;
            _r = v.toLongLong(&ok);
            if (ok) {
                return _r;
            }
        }
    }
    return -1;
}

bool svn::cache::ReposLog::checkFill(svn::Revision&start,svn::Revision&end,bool checkHead)
{
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return false;
        }
    }
    ContextP cp = m_Client->getContext();
//     long long icount=0;

    svn::Revision _latest=latestCachedRev();
//    //qDebug("Latest cached rev: %i",_latest.revnum());
//    //qDebug("End revision is: %s",end.toString().TOUTF8().data());

    if (checkHead && _latest.revnum()>=latestHeadRev().revnum()) {
        return true;
    }

    start=date2numberRev(start);
    end=date2numberRev(end);

    // both should now one of START, HEAD or NUMBER
    if (start==svn::Revision::HEAD || (end==svn::Revision::NUMBER && start==svn::Revision::NUMBER && start.revnum()>end.revnum())) {
        svn::Revision tmp = start;
        start = end;
        end = tmp;
    }
    svn::Revision _rstart=_latest.revnum()+1;
    svn::Revision _rend = end;
    if (_rend==svn::Revision::UNDEFINED) {
//        //qDebug("Setting end to Head");
        _rend=svn::Revision::HEAD;
    }
    // no catch - exception should go outside.
    if (_rstart==0){
        _rstart = 1;
    }
//    //qDebug("Getting log %s -> %s",_rstart.toString().TOUTF8().data(),_rend.toString().TOUTF8().data());
    if (_rend==svn::Revision::HEAD) {
        _rend=latestHeadRev();
    }

    if (_rend==svn::Revision::HEAD||_rend.revnum()>_latest.revnum()) {
        LogEntriesMap _internal;
//        //qDebug("Retrieving from network.");
        LogParameter params;

        if (!m_Client->log(params.targets(m_ReposRoot).revisionRange(_rstart,_rend).peg(svn::Revision::UNDEFINED).discoverChangedPathes(true).strictNodeHistory(false),_internal)) {
            return false;
        }
        LogEntriesMap::ConstIterator it=_internal.begin();

        for (;it!=_internal.end();++it) {
            _insertLogEntry((*it));
            if (cp && cp->getListener()) {
                //cp->getListener()->contextProgress(++icount,_internal.size());
                if (cp->getListener()->contextCancel()) {
                    throw DatabaseException(QString("Could not retrieve values: User cancel."));
                }
            }
        }
    }
    return true;
}

bool svn::cache::ReposLog::fillCache(const svn::Revision&_end)
{
    svn::Revision end = _end;
    svn::Revision start = latestCachedRev().revnum()+1;
    return checkFill(start,end,false);
}

/*!
    \fn svn::cache::ReposLog::simpleLog(const svn::Revision&start,const svn::Revision&end,LogEntriesMap&target)
 */
bool svn::cache::ReposLog::simpleLog(LogEntriesMap&target,const svn::Revision&_start,const svn::Revision&_end,bool noNetwork,const QStringList&exclude)
{
    if (!m_Client||m_ReposRoot.isEmpty()) {
        return false;
    }
    target.clear();
    ContextP cp = m_Client->getContext();

    svn::Revision end = _end;
    svn::Revision start = _start;
    if (!noNetwork) {
        if (!checkFill(start,end,true)) {
            return false;
        }
    } else {
        end=date2numberRev(end,noNetwork);
        start=date2numberRev(start,noNetwork);
    }

    if (end==svn::Revision::HEAD) {
        end = latestCachedRev();
    }
    if (start==svn::Revision::HEAD) {
        start=latestCachedRev();
    }
    static QString sCount("select count(*) from logentries where revision<=? and revision>=?");
    static QString sEntry("select revision,author,date,message from logentries where revision<=? and revision>=?");
    static QString sItems("select changeditem,action,copyfrom,copyfromrev from changeditems where revision=?");

    for (int i = 0; i < exclude.size();++i) {
        sItems+=" and changeditem not like '"+exclude[i]+"%'";
    }

    QSqlQuery bcount(QString(),m_Database);
    bcount.prepare(sCount);

    QSqlQuery bcur(QString(),m_Database);

    QSqlQuery cur(QString(),m_Database);

    bcount.bindValue(0,Q_LLONG(end.revnum()));
    bcount.bindValue(1,Q_LLONG(start.revnum()));
    if (!bcount.exec()) {
        //qDebug() << bcount.lastError().text();
        throw svn::cache::DatabaseException(QString("Could not retrieve count: ")+bcount.lastError().text());
        return false;
    }
    bcount.next();
    if (bcount.value(0).toLongLong()<1) {
        // we didn't found logs with this parameters
        return false;
    }

    bcur.prepare(sEntry);
    bcur.bindValue(0,Q_LLONG(end.revnum()));
    bcur.bindValue(1,Q_LLONG(start.revnum()));

    if (!bcur.exec()) {
        throw svn::cache::DatabaseException(QString("Could not retrieve values: ")+bcur.lastError().text());
        return false;
    }
    Q_LLONG revision;
    while(bcur.next()) {
        cur.prepare(sItems);
        revision = bcur.value(0).toLongLong();
        cur.bindValue(0,revision);

        if (!cur.exec()) {
            //qDebug() << cur.lastError().text();
            throw svn::cache::DatabaseException(QString("Could not retrieve revision values: ")+cur.lastError().text()
                    ,cur.lastError().number());
            return false;
        }
        target[revision].revision=revision;
        target[revision].author=bcur.value(1).toString();
        target[revision].date=bcur.value(2).toLongLong();
        target[revision].message=bcur.value(3).toString();
        while(cur.next()) {
            LogChangePathEntry lcp;
            QString ac = cur.value(1).toString();
            lcp.action=ac[0].toLatin1();
            lcp.copyFromPath=cur.value(2).toString();
            lcp.path= cur.value(0).toString();
            lcp.copyFromRevision=cur.value(3).toLongLong();
            target[revision].changedPaths.push_back(lcp);
        }
        if (cp && cp->getListener()) {
            if (cp->getListener()->contextCancel()) {
                throw svn::cache::DatabaseException(QString("Could not retrieve values: User cancel."));
                return false;
            }
        }
    }
    return true;
}


/*!
    \fn svn::cache::ReposLog::date2numberRev(const svn::Revision&)
 */
svn::Revision svn::cache::ReposLog::date2numberRev(const svn::Revision&aRev,bool noNetwork)
{
    if (aRev!=svn::Revision::DATE) {
        return aRev;
    }
    if (!m_Database.isValid()) {
        return svn::Revision::UNDEFINED;
    }
    static QString _q("select revision from logentries where date<? order by revision desc");
    QSqlQuery query("select revision,date from logentries order by revision desc limit 1",m_Database);

    if (query.lastError().type()!=QSqlError::NoError) {
        //qDebug() << query.lastError().text();
    }
    bool must_remote=!noNetwork;
    if (query.next()) {
        if (query.value(1).toLongLong()>=aRev.date()) {
            must_remote=false;
        }
    }
    if (must_remote) {
        svn::InfoEntries e = (m_Client->info(m_ReposRoot,svn::DepthEmpty,aRev,aRev));;
        if (e.count()<1||e[0].reposRoot().isEmpty()) {
            return aRev;
        }
        return e[0].revision();
    }
    query.prepare(_q);
    query.bindValue(0,Q_LLONG(aRev.date()));
    query.exec();
    if (query.lastError().type()!=QSqlError::NoError) {
        //qDebug() << query.lastError().text();
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    // not found...
    if (noNetwork) {
        return svn::Revision::UNDEFINED;
    }
    svn::InfoEntries e = (m_Client->info(m_ReposRoot,svn::DepthEmpty,svn::Revision::HEAD,svn::Revision::HEAD));;
    if (e.count()<1||e[0].reposRoot().isEmpty()) {
        return svn::Revision::UNDEFINED;
    }
    return e[0].revision();
}


/*!
    \fn svn::cache::ReposLog::insertLogEntry(const svn::LogEntry&)
 */
bool svn::cache::ReposLog::_insertLogEntry(const svn::LogEntry&aEntry)
{
    m_Database.transaction();
    qlonglong j = aEntry.revision;
    static QString qEntry("insert into logentries (revision,date,author,message) values (?,?,?,?)");
    static QString qPathes("insert into changeditems (revision,changeditem,action,copyfrom,copyfromrev) values (?,?,?,?,?)");
    QSqlQuery _q(QString(),m_Database);
    _q.prepare(qEntry);
    _q.bindValue(0,j);
    _q.bindValue(1,aEntry.date);
    _q.bindValue(2,aEntry.author);
    _q.bindValue(3,aEntry.message);
    if (!_q.exec()) {
        m_Database.rollback();
        //qDebug("Could not insert values: %s",_q.lastError().text().TOUTF8().data());
        //qDebug() << _q.lastQuery();
        throw svn::cache::DatabaseException(QString("_insertLogEntry_0: Could not insert values: ")+_q.lastError().text(),_q.lastError().number());
    }
    _q.prepare(qPathes);
    svn::LogChangePathEntries::ConstIterator cpit = aEntry.changedPaths.begin();
    for (;cpit!=aEntry.changedPaths.end();++cpit){
        _q.bindValue(0,j);
        _q.bindValue(1,(*cpit).path);
        _q.bindValue(2,QString(QChar((*cpit).action)));
        _q.bindValue(3,(*cpit).copyFromPath);
        _q.bindValue(4,Q_LLONG((*cpit).copyFromRevision));
        if (!_q.exec()) {
            m_Database.rollback();
            //qDebug("Could not insert values: %s",_q.lastError().text().TOUTF8().data());
            //qDebug() << _q.lastQuery();
            throw svn::cache::DatabaseException(QString("Could not insert values: ")+_q.lastError().text(),_q.lastError().number());
        }
    }
    if (aEntry.m_MergedInRevisions.size()>0) {
        static QString qMerges("insert into mergeditems(revision,mergeditems) values(?,?)");
        _q.prepare(qMerges);
        QByteArray _merges;
        QBuffer buffer(&_merges);
        buffer.open(QIODevice::ReadWrite);
        QDataStream af(&buffer);
        af << aEntry.m_MergedInRevisions;
        buffer.close();
        _q.bindValue(0,j);
        _q.bindValue(1,_merges.data());
        if (!_q.exec()) {
            m_Database.rollback();
            //qDebug("Could not insert values: %s",_q.lastError().text().TOUTF8().data());
            //qDebug() << _q.lastQuery();
            throw svn::cache::DatabaseException(QString("Could not insert values: ")+_q.lastError().text(),_q.lastError().number());
        }
    }
    m_Database.commit();
    return true;
}

bool svn::cache::ReposLog::insertLogEntry(const svn::LogEntry&aEntry)
{
    return _insertLogEntry(aEntry);
}


/*!
    \fn svn::cache::ReposLog::log(const svn::Path&,const svn::Revision&start, const svn::Revision&end,const svn::Revision&peg,svn::LogEntriesMap&target, bool strictNodeHistory,int limit))
 */
bool svn::cache::ReposLog::log(const svn::Path&what,const svn::Revision&_start, const svn::Revision&_end,const svn::Revision&_peg,svn::LogEntriesMap&target, bool strictNodeHistory,int limit)
{
    Q_UNUSED(strictNodeHistory);
    static QString s_q("select logentries.revision,logentries.author,logentries.date,logentries.message from logentries where logentries.revision in (select changeditems.revision from changeditems where (changeditems.changeditem='%1' or changeditems.changeditem GLOB '%2/*') %3 GROUP BY changeditems.revision) ORDER BY logentries.revision DESC");

    static QString s_e("select changeditem,action,copyfrom,copyfromrev from changeditems where changeditems.revision='%1'");
    static QString s_m("select mergeditems from mergeditems where mergeditems.revision='%1'");

    svn::Revision peg = date2numberRev(_peg,true);
    svn::Revision end = date2numberRev(_end,true);
    svn::Revision start = date2numberRev(_start,true);
    QString query_string = QString(s_q).arg(what.native()).arg(what.native()).arg((peg==svn::Revision::UNDEFINED?"":QString(" AND revision<=%1").arg(peg.revnum())));
    if (peg==svn::Revision::UNDEFINED) {
        peg = latestCachedRev();
    }
    if (!itemExists(peg,what)) {
        throw svn::cache::DatabaseException(QString("Entry '%1' does not exists at revision %2").arg(what.native()).arg(peg.toString()));
    }
    if (limit>0) {
        query_string+=QString(" LIMIT %1").arg(limit);
    }
    QSqlQuery _q(QString(),m_Database);
    QSqlQuery _q2(QString(),m_Database);
    _q.prepare(query_string);
    if (!_q.exec()) {
        //qDebug("Could not select values: %s",_q.lastError().text().TOUTF8().data());
        //qDebug() << _q.lastQuery();
        throw svn::cache::DatabaseException(QString("Could not select values: ")+_q.lastError().text(),_q.lastError().number());
    }
    while(_q.next()) {
        Q_LLONG revision = _q.value(0).toLongLong();
        target[revision].revision=revision;
        target[revision].author=_q.value(1).toString();
        target[revision].date=_q.value(2).toLongLong();
        target[revision].message=_q.value(3).toString();
        query_string=s_e.arg(revision);
        _q2.prepare(query_string);
        if (!_q2.exec()) {
            //qDebug("Could not select values: %s",_q2.lastError().text().TOUTF8().data());
        } else {
            while (_q2.next()) {
                target[revision].changedPaths.push_back (
                        LogChangePathEntry (_q2.value(0).toString(),
                                            _q2.value(1).toChar().toLatin1(),
                                            _q2.value(2).toString(),
                                            _q2.value(3).toLongLong()
                                           )
                                                        );
            }
        }
        query_string=s_m.arg(revision);
        _q2.prepare(query_string);
        if (!_q2.exec()) {
            //qDebug("Could not select values: %s",_q2.lastError().text().TOUTF8().data());
        } else {
            if (_q2.next()) {
                QByteArray byteArray = _q2.value(0).toByteArray();
                QBuffer buffer(&byteArray);
                QDataStream in(&buffer);
                in >> target[revision].m_MergedInRevisions;
            }
        }
    }
    return true;
}


/*!
    \fn svn::cache::ReposLog::itemExists(const svn::Revision&,const QString&)
 */
bool svn::cache::ReposLog::itemExists(const svn::Revision&peg,const svn::Path&path)
{
    /// @todo this moment I have no idea how to check real  with all moves and deletes of parent folders without a hell of sql statements so we make it quite simple: it exists if we found it.

    Q_UNUSED(peg);
    Q_UNUSED(path);

#if 0
    static QString _s1("select revision from changeditems where changeditem='%1' and action='A' and revision<=%2 order by revision desc limit 1");
    QSqlQuery _q(QString(),m_Database);
    QString query_string=QString(_s1).arg(path.native()).arg(peg.revnum());
    if (!_q.exec(query_string)) {
        //qDebug("Could not select values: %s",_q.lastError().text().TOUTF8().data());
        //qDebug(_q.lastQuery().TOUTF8().data());
        throw svn::cache::DatabaseException(QString("Could not select values: ")+_q.lastError().text(),_q.lastError().number());
    }
    //qDebug(_q.lastQuery().TOUTF8().data());


    svn::Path _p = path;
    static QString _s2("select revision from changeditem where changeditem in (%1) and action='D' and revision>%2 and revision<=%3 order by revision desc limit 1");
    QStringList p_list;
    while (_p.length()>0) {
        p_list.append(QString("'%1'").arg(_p.native()));
        _p.removeLast();
    }
    query_string=QString(_s2).arg(p_list.join(",")).arg();
#endif
    return true;
}

bool svn::cache::ReposLog::isValid()const
{
    if (!m_Database.isValid()) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database.isValid()) {
            return false;
        }
    }
    return true;
}

void svn::cache::ReposLog::cleanLogEntries()
{

    if (!isValid()) {
        return;
    }
    m_Database.transaction();
    QSqlQuery _q(QString(),m_Database);
    if (!_q.exec("delete from logentries")) {
        m_Database.rollback();
        return;
    }
    if (!_q.exec("delete from changeditems")) {
        m_Database.rollback();
        return;
    }
    if (!_q.exec("delete from mergeditems")) {
        m_Database.rollback();
        return;
    }

    m_Database.commit();
    _q.exec("vacuum");
}
