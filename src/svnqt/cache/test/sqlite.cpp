#include <qsql.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <iostream>
#include <qapplication.h>
#include "src/svnqt/cache/LogCache.hpp"
#include "src/svnqt/cache/ReposLog.hpp"
#include "src/svnqt/client.hpp"
#include "src/svnqt/cache/test/testconfig.h"
#include "src/svnqt/svnqttypes.hpp"

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
    //std::cerr << rl.latestHeadRev().toString()<<std::endl;
    svn::cache::ReposLog rl2(m_Svnclient,"http://www.alwins-world.de/repos/");
    try {
        std::cerr << rl2.latestCachedRev().toString()<<std::endl;
    }
    catch (svn::ClientException cl)
    {
        std::cerr << "Exception catched"<<std::endl;
    }
    svn::LogEntriesMap lm;
    rl.simpleLog(lm,1,svn::Revision::HEAD,false);
    return 0;
}
