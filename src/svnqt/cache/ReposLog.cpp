#include "ReposLog.hpp"

#include "LogCache.hpp"
#include "src/svnqt/info_entry.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/client.hpp"
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

/*!
    \fn svn::cache::ReposLog::simpleLog(const svn::Revision&start,const svn::Revision&end,LogEntriesMap&target)
 */
bool svn::cache::ReposLog::simpleLog(LogEntriesMap&target,const svn::Revision&start,const svn::Revision&end,bool force_headupdate)
{
    if (!m_Client||m_ReposRoot.isEmpty()) {
        return false;
    }
    if (!m_Database) {
        m_Database = LogCache::self()->reposDb(m_ReposRoot);
        if (!m_Database) {
            return false;
        }
    }

    svn::Revision _latest=latestCachedRev();
    qDebug("Latest cached rev: %i",_latest.revnum());

    ///@todo insert check if start is later than end, transform revisions in dateformat to a number (via select and/or svn::Client::log)

    svn::Revision _rstart=_latest.revnum()+1;
    svn::Revision _rend = force_headupdate?svn::Revision::HEAD:end;
    if (_rend==svn::Revision::UNDEFINED) {
        _rend=svn::Revision::HEAD;
    }
    LogEntriesMap _internal;
    // no catch - exception should go outside.
    if (_rstart==0){
        _rstart = 1;
    }
    qDebug("Getting log %s -> %s",_rstart.toString().latin1(),_rend.toString().latin1());
    if (_rend==svn::Revision::HEAD) {
        _rend=latestHeadRev();
    }
    QSqlCursor bcur("logentries",true,m_Database);
    QSqlRecord *buffer;
    if (_rend==svn::Revision::HEAD||_rend.revnum()>_latest.revnum()) {
        qDebug("Retrieving from network.");
        if (!m_Client->log(m_ReposRoot,_rstart,_rend,_internal,svn::Revision::UNDEFINED,true,false)) {
            return false;
        }
        LogEntriesMap::ConstIterator it=_internal.begin();
        QSqlQuery _q(QString::null, m_Database);
        QString q;
        QSqlCursor cur("changeditems",true,m_Database);

        for (;it!=_internal.end();++it) {
            m_Database->transaction();
            svn::DateTime dt(it.data().date);
            int j = it.data().revision;
            qDebug("Insert revsion %i",j);
            buffer=bcur.primeInsert();
            buffer->setValue("revision",j);
            buffer->setValue("date",dt.toTime_t());
            buffer->setValue("author",it.data().author);
            buffer->setValue("message",it.data().message);
            bcur.insert();
            if (bcur.lastError().type()!=QSqlError::None) {
                m_Database->rollback();
                throw DatabaseException(QString("Could not insert values: ")+bcur.lastError().text());
            }
            svn::LogChangePathEntries::ConstIterator cpit = it.data().changedPaths.begin();
            for (;cpit!=it.data().changedPaths.end();++cpit){
                buffer = cur.primeInsert();
                buffer->setValue("revision",j);
                buffer->setValue("changeditem",(*cpit).path);
                buffer->setValue("action",(*cpit).action);
                buffer->setValue("copyfrom",(*cpit).copyFromPath);
                buffer->setValue("copyfromrev",Q_LLONG((*cpit).copyFromRevision));
                cur.insert();
                if (cur.lastError().type()!=QSqlError::None) {
                    m_Database->rollback();
                    throw DatabaseException(QString("Could not insert values: ")+cur.lastError().text());
                }
            }
            m_Database->commit();
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
    svn::DateTime dt(aRev.date());
    unsigned int value = dt.toTime_t();
    QSqlCursor bcur("logentries",true,m_Database);
    QSqlIndex order = bcur.index("revision");
    order.setDescending(0,true);
    /// @todo as I understood - the resulting revision should always the last one BEFORE this date...
    //QString q=QString("date>='%1'").arg(value);
    //qDebug(q);
    bcur.select(order);
    if (bcur.lastError().type()!=QSqlError::None) {
        qDebug(bcur.lastError().text());
    }
    bool must_remote=true;
    if (bcur.next()) {
        QDateTime tdt;
        tdt.setTime_t(bcur.value("date").toInt());
        QDateTime t2(dt);
        if (tdt >= t2) {
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
    QString q=QString("date<'%1'").arg(value);
    bcur.select(q,order);
    qDebug(q);
    if (bcur.lastError().type()!=QSqlError::None) {
        qDebug(bcur.lastError().text());
    }
    if (bcur.next()) {
        return bcur.value( "revision" ).toInt();
    }
    // not found...
    svn::InfoEntries e = (m_Client->info(m_ReposRoot,false,svn::Revision::HEAD,svn::Revision::HEAD));;
    if (e.count()<1||e[0].reposRoot().isEmpty()) {
        return aRev;
    }
    return e[0].revision();
}
