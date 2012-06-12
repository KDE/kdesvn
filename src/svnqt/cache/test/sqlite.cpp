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
#include <qsql.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <iostream>
#include <qapplication.h>
#include <qtextstream.h>

#include "svnqt/client.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/log_entry.h"

#include "svnqt/cache/LogCache.h"
#include "svnqt/cache/ReposLog.h"
#include "svnqt/cache/ReposConfig.h"
#include "svnqt/cache/test/testconfig.h"
#include "svnqt/cache/DatabaseException.h"

#include <QSqlQuery>
#include <QSqlError>

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
    if (!db.isValid()) {
        std::cerr << "No database object."<<std::endl;
        exit(-1);
    }
    list = db.tables();
    it = list.begin();
    while( it != list.end() ) {
        std::cout << "Table: "<<( *it ).TOUTF8().data() << std::endl;
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
    QSqlQuery q("insert into logentries(revision,date,author,message) values ('100','1122591406','alwin','copy and moving works now in basic form')",db);
    q.exec();
    std::cerr << "\nSelf: \n" << q.lastError().text().TOUTF8().data()<<std::endl;


    db=QSqlDatabase();
    try {
        rl.log("/trunk/src/svnqt",1,1000,svn::Revision::UNDEFINED,lm,false,-1);
    }
    catch (const svn::cache::DatabaseException&cl)
    {
        std::cerr << cl.msg().TOUTF8().data() <<std::endl;
    }
    catch (const svn::Exception&ce)
    {
        std::cerr << "Exception: " << ce.msg().TOUTF8().data() <<std::endl;
    }
    std::cout<<"Count: "<<lm.count()<<std::endl;

    QStringList s; s << "a" << "b" << "c";

    svn::cache::ReposConfig::self()->setValue("http://www.alwins-world.de/repos/kdesvn","bommel",s);
    list = svn::cache::ReposConfig::self()->readEntry("http://www.alwins-world.de/repos/kdesvn","bommel",QStringList());
    std::cout<<"Value: ";
    foreach(const QString &entry, list) {
        std::cout << entry.TOUTF8().data()<<",";
    }
    std::cout << std::endl;
    return 0;
}
