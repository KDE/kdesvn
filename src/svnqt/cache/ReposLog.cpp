#include "ReposLog.hpp"

#include "LogCache.hpp"
#include "src/svnqt/info_entry.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/client.hpp"
#include "src/svnqt/context_listener.hpp"
#include "src/svnqt/cache/DatabaseException.hpp"

#include <qsqldatabase.h>
#include <qsqlcursor.h>

/*!
    \fn svn::cache::ReposLog::ReposLog(svn::Client*aClient,const QString&)
 */
svn::cache::ReposLog::ReposLog(svn::Client*aClient,const QString&aRepository)
    :m_Client(aClient),m_Database(0),m_ReposRoot(aRepository),m_latestHead(svn::Revision::UNDEFINED)
{
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
    if (!m_Database) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database) {
            return svn::Revision::UNDEFINED;
        }
    }
    /// no catch - exception has go trough...
    svn::InfoEntries e = (m_Client->info(m_ReposRoot,false,svn::Revision::HEAD,svn::Revision::HEAD));;
    if (e.count()<1||e[0].reposRoot().isEmpty()) {
        return svn::Revision::UNDEFINED;
    }
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
    if (!m_Database) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database) {
            return svn::Revision::UNDEFINED;
        }
    }
    QString q("select revision from 'logentries' order by revision DESC limit 1");
    QSqlQuery _q(QString::null, m_Database);
    if (!_q.exec(q)) {
        qDebug(_q.lastError().text());
        return svn::Revision::UNDEFINED;
    }
    int _r;
    if (_q.isActive() && _q.next()) {

        qDebug("Sel result: "+_q.value(0).toString());
        _r = _q.value(0).toInt();
    } else {
        qDebug(_q.lastError().text());
        return svn::Revision::UNDEFINED;
    }
    return _r;
}

bool svn::cache::ReposLog::checkFill(svn::Revision&start,svn::Revision&end)
{
    if (!m_Database) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database) {
            return false;
        }
    }
    ContextP cp = m_Client->getContext();
    long long icount=0;

    svn::Revision _latest=latestCachedRev();
    qDebug("Latest cached rev: %i",_latest.revnum());
    if (_latest.revnum()>=latestHeadRev().revnum()) {
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
    qDebug("End: "+end.toString());
    svn::Revision _rstart=_latest.revnum()+1;
    svn::Revision _rend = end;
    if (_rend==svn::Revision::UNDEFINED) {
        _rend=svn::Revision::HEAD;
    }
    // no catch - exception should go outside.
    if (_rstart==0){
        _rstart = 1;
    }
    qDebug("Getting log %s -> %s",_rstart.toString().latin1(),_rend.toString().latin1());
    if (_rend==svn::Revision::HEAD) {
        _rend=latestHeadRev();
    }

    if (_rend==svn::Revision::HEAD||_rend.revnum()>_latest.revnum()) {
        LogEntriesMap _internal;
        qDebug("Retrieving from network.");
        if (!m_Client->log(m_ReposRoot,_rstart,_rend,_internal,svn::Revision::UNDEFINED,true,false)) {
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
    return checkFill(start,end);
}

/*!
    \fn svn::cache::ReposLog::simpleLog(const svn::Revision&start,const svn::Revision&end,LogEntriesMap&target)
 */
bool svn::cache::ReposLog::simpleLog(LogEntriesMap&target,const svn::Revision&_start,const svn::Revision&_end)
{
    if (!m_Client||m_ReposRoot.isEmpty()) {
        return false;
    }
    target.clear();
    ContextP cp = m_Client->getContext();

    svn::Revision end = _end;
    svn::Revision start = _start;
    if (!checkFill(start,end)) {
        return false;
    }

    qDebug("End: "+end.toString());
    if (end==svn::Revision::HEAD) {
        end = latestCachedRev();
    }
    if (start==svn::Revision::HEAD) {
        start=latestCachedRev();
    }

    static QString sEntry("select revision,author,date,message from logentries where revision<=? and revision>=?");
    static QString sItems("select changeditem,action,copyfrom,copyfromrev from changeditems where revision=?");

    QSqlQuery bcur(QString::null,m_Database);
    bcur.prepare(sEntry);

    QSqlQuery cur(QString::null,m_Database);
    cur.prepare(sItems);

    bcur.bindValue(0,Q_LLONG(end.revnum()));
    bcur.bindValue(1,Q_LLONG(start.revnum()));

    if (!bcur.exec()) {
        qDebug(bcur.lastError().text());
        throw svn::cache::DatabaseException(QString("Could not retrieve values: ")+bcur.lastError().text());
        return false;
    }
    Q_LLONG revision;
    while(bcur.next()) {
        revision = bcur.value(0).toLongLong();
        cur.bindValue(0,revision);
        if (!cur.exec()) {
            qDebug(cur.lastError().text());
            throw svn::cache::DatabaseException(QString("Could not retrieve values: ")+cur.lastError().text()
                    ,cur.lastError().number());
            return false;
        }
        target[revision].revision=revision;
        target[revision].author=bcur.value(1).toString();
        target[revision].date=bcur.value(2).toLongLong();
        target[revision].message=bcur.value(3).toString();
        while(cur.next()) {
            char c = cur.value(1).toInt();
            LogChangePathEntry lcp;
            lcp.action=cur.value(1).toString()[0];
            lcp.copyFromPath=cur.value(2).toString().latin1();
            lcp.path= cur.value(0).toString();
            lcp.copyFromRevision=cur.value(3).toLongLong();
            target[revision].changedPaths.push_back(lcp);
        }
        if (cp && cp->getListener()) {
            //cp->getListener()->contextProgress(++icount,bcur.size());
            if (cp->getListener()->contextCancel()) {
                throw svn::cache::DatabaseException(QString("Could not retrieve values: User cancel."));
            }
        }
    }
    return false;
}


/*!
    \fn svn::cache::ReposLog::date2numberRev(const svn::Revision&)
 */
svn::Revision svn::cache::ReposLog::date2numberRev(const svn::Revision&aRev)
{
    if (aRev!=svn::Revision::DATE) {
        return aRev;
    }
    if (!m_Database) {
        return svn::Revision::UNDEFINED;
    }
    static QString _q("select revision from logentries where date<? order by revision desc");
    QSqlQuery query("select revision,date from logentries order by revision desc limit 1",m_Database);

    if (query.lastError().type()!=QSqlError::None) {
        qDebug(query.lastError().text());
    }
    bool must_remote=true;
    if (query.next()) {
        if (query.value(1).toLongLong()>=aRev.date()) {
            must_remote=false;
        }
    }
    if (must_remote) {
        svn::InfoEntries e = (m_Client->info(m_ReposRoot,false,aRev,aRev));;
        if (e.count()<1||e[0].reposRoot().isEmpty()) {
            return aRev;
        }
        return e[0].revision();
    }
    query.prepare(_q);
    query.bindValue(0,Q_LLONG(aRev.date()));
    query.exec();
    if (query.lastError().type()!=QSqlError::None) {
        qDebug(query.lastError().text());
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    // not found...
    svn::InfoEntries e = (m_Client->info(m_ReposRoot,false,svn::Revision::HEAD,svn::Revision::HEAD));;
    if (e.count()<1||e[0].reposRoot().isEmpty()) {
        return aRev;
    }
    return e[0].revision();
}


/*!
    \fn svn::cache::ReposLog::insertLogEntry(const svn::LogEntry&)
 */
bool svn::cache::ReposLog::_insertLogEntry(const svn::LogEntry&aEntry)
{
    QSqlRecord *buffer;
    m_Database->transaction();

#if QT_VERSION < 0x040000
    Q_LLONG j = aEntry.revision;
#else
    qlonglong j = aEntry.revision;
#endif
    static QString qEntry("insert into logentries (revision,date,author,message) values (?,?,?,?)");
    static QString qPathes("insert into changeditems (revision,changeditem,action,copyfrom,copyfromrev) values (?,?,?,?,?)");
    QSqlQuery _q(QString::null,m_Database);
    _q.prepare(qEntry);
    _q.bindValue(0,j);
    _q.bindValue(1,aEntry.date);
    _q.bindValue(2,aEntry.author);
    _q.bindValue(3,aEntry.message);
    if (!_q.exec()) {
        m_Database->rollback();
        qDebug(QString("Could not insert values: ")+_q.lastError().text());
        qDebug(_q.lastQuery());
        throw svn::cache::DatabaseException(QString("Could not insert values: ")+_q.lastError().text(),_q.lastError().number());
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
            m_Database->rollback();
            qDebug(QString("Could not insert values: ")+_q.lastError().text());
            qDebug(_q.lastQuery());
            throw svn::cache::DatabaseException(QString("Could not insert values: ")+_q.lastError().text(),_q.lastError().number());
        }
    }
    m_Database->commit();
    return true;
}

bool svn::cache::ReposLog::insertLogEntry(const svn::LogEntry&aEntry)
{
    _insertLogEntry(aEntry);
}
