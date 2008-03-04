#include "ReposLog.hpp"

#include "LogCache.hpp"
#include "src/svnqt/info_entry.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/client.hpp"

#include <qsqldatabase.h>

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
    /*
    if (start==svn::Revision::HEAD||end==svn::Revision::HEAD||_latest==svn::Revision::UNDEFINED) {
        force_headupdate=true;
    }
    */

    ///@todo insert check if start is later than end, transform revisions in dateformat to a number (via select)
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
    if (_rend==svn::Revision::HEAD||_rend.revnum()>_latest.revnum()) {
        if (!m_Client->log(m_ReposRoot,_rstart,_rend,_internal,svn::Revision::UNDEFINED,true,false)) {
            return false;
        }
        LogEntriesMap::ConstIterator it=_internal.begin();
        int j = 0;
        m_Database->transaction();
        QSqlQuery _q(QString::null, m_Database);
        QString q;

        for (;it!=_internal.end();++it) {
            svn::DateTime dt(it.data().date);
            q=QString("insert into logentries (revision,date,author,message) values('%1','%2','%3','%4')").arg(it.data().revision).
                                                                                arg(dt.toTime_t()).
                                                                                arg(it.data().author).
                                                                                arg(it.data().message);
            _q.exec(q);
            if (++j>100) {
                m_Database->commit();
                m_Database->transaction();
            }
        }
        m_Database->commit();
    }
    return false;
}
