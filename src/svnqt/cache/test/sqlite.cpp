#include <qsql.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <iostream>
#include <qapplication.h>
#include <qtextstream.h>

#include "svnqt/client.hpp"
#include "svnqt/svnqttypes.hpp"
#include "svnqt/log_entry.hpp"

#include "svnqt/cache/LogCache.hpp"
#include "svnqt/cache/ReposLog.hpp"
#include "svnqt/cache/test/testconfig.h"
#include "svnqt/cache/DatabaseException.hpp"

#if QT_VERSION < 0x040000
#else
#include <QSqlQuery>
#include <QSqlError>
#endif

int main(int argc,char**argv)
{
    QApplication app(argc,argv);

    svn::ContextP m_CurrentContext;
    svn::Client* m_Svnclient;
    m_Svnclient=svn::Client::getobject(0,0);
    m_CurrentContext = new svn::Context();

    m_Svnclient->setContext(m_CurrentContext);

    QStringList list;
    QStringList::Iterator it;
    // goes into "self" of logcache
    new svn::cache::LogCache(TESTDBPATH);
    list = QSqlDatabase::drivers();
    it = list.begin();
    while( it != list.end() ) {
        std::cout << (*it).TOUTF8().data() << std::endl;
        ++it;
    }
    svn::cache::ReposLog rl(m_Svnclient,"http://www.alwins-world.de/repos/kdesvn");
    QDataBase db = rl.Database();
#if QT_VERSION < 0x040000
    if (!db) {
#else
    if (!db.isValid()) {
#endif
        std::cerr << "No database object."<<std::endl;
        exit(-1);
    }
#if QT_VERSION < 0x040000
    list = db->tables();
#else
    list = db.tables();
#endif
    it = list.begin();
    while( it != list.end() ) {
        std::cout << ( *it ).TOUTF8().data() << std::endl;
        ++it;
    }
    svn::LogEntriesMap lm;
    try {
        rl.simpleLog(lm,100,svn::Revision::HEAD);
    }
    catch (const svn::cache::DatabaseException&cl)
    {
        std::cerr << cl.msg().TOUTF8().data() <<std::endl;
    }
    catch (const svn::Exception&ce)
    {
        std::cerr << "Exception: " << ce.msg().TOUTF8().data() <<std::endl;
    }
    svn::LogEntriesMap::ConstIterator lit = lm.begin();
    std::cout<<"Count: "<<lm.count()<<std::endl;

    svn::Revision r("{2006-09-27}");
    std::cout << r.toString().TOUTF8().data() << " -> " << rl.date2numberRev(r).toString().TOUTF8().data()<<std::endl;
    r = svn::Revision::HEAD;
    std::cout << rl.date2numberRev(r).toString().TOUTF8().data()<<std::endl;
    try {
        rl.insertLogEntry(lm[100]);
    }
    catch (const svn::cache::DatabaseException&cl)
    {
        std::cerr << cl.msg().TOUTF8().data() << std::endl;
    }
    QSqlQuery q("insert or ignore into logentries(revision,date,author,message) values ('100','1122591406','alwin','copy and moving works now in basic form')",db);
    q.exec();
    std::cerr << "\n" << q.lastError().text().TOUTF8().data()<<std::endl;

#if QT_VERSION < 0x040000
#else
    db=QSqlDatabase();
#endif


    return 0;
}
