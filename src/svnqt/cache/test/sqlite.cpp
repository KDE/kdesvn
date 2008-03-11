#include <qsql.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <iostream>
#include <qapplication.h>
#include <qtextstream.h>

#include "src/svnqt/client.hpp"
#include "src/svnqt/svnqttypes.hpp"
#include "src/svnqt/log_entry.hpp"

#include "src/svnqt/cache/LogCache.hpp"
#include "src/svnqt/cache/ReposLog.hpp"
#include "src/svnqt/cache/test/testconfig.h"
#include "src/svnqt/cache/DatabaseException.hpp"

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
        std::cout << (*it) << std::endl;
        ++it;
    }
    svn::cache::ReposLog rl(m_Svnclient,"http://www.alwins-world.de/repos/kdesvn");
    QSqlDatabase*db = rl.Database();
    if (!db) {
        std::cerr << "No database object."<<std::endl;
        exit(-1);
    }
    list = db->tables();
    it = list.begin();
    while( it != list.end() ) {
        std::cout << ( *it ) << std::endl;
        ++it;
    }
    svn::LogEntriesMap lm;
    try {
        rl.simpleLog(lm,100,svn::Revision::HEAD);
    }
    catch (const svn::cache::DatabaseException&cl)
    {
        std::cerr << cl.msg() <<std::endl;
    }
    catch (const svn::Exception&ce)
    {
        std::cerr << "Exception: " << ce.msg().latin1() <<std::endl;
    }
    svn::LogEntriesMap::ConstIterator lit = lm.begin();
    std::cout<<"Count: "<<lm.count()<<std::endl;

    svn::Revision r("{2006-09-27}");
    std::cout << r.toString() << " -> " << rl.date2numberRev(r).toString()<<std::endl;
    r = svn::Revision::HEAD;
    std::cout << rl.date2numberRev(r).toString()<<std::endl;
    try {
        rl.insertLogEntry(lm[100]);
    }
    catch (const svn::cache::DatabaseException&cl)
    {
        std::cerr << cl.msg() << std::endl;
    }
    QSqlQuery q("insert or ignore into logentries(revision,date,author,message) values ('100','1122591406','alwin','copy and moving works now in basic form')",db);
    q.exec();
    std::cerr << "\n" << q.lastError().text().latin1()<<std::endl;

    return 0;
}
