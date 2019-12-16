/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#include <qsql.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <iostream>
#include <qcoreapplication.h>
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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    svn::ContextP m_CurrentContext(new svn::Context);
    svn::ClientP m_Svnclient = svn::Client::getobject(m_CurrentContext);

    QStringList list;
    QStringList::Iterator it;
    // goes into "self" of logcache
    new svn::cache::LogCache(TESTDBPATH);
    list = QSqlDatabase::drivers();
    it = list.begin();
    while (it != list.end()) {
        std::cout << qPrintable(*it) << std::endl;
        ++it;
    }
    svn::cache::ReposLog rl(m_Svnclient, "svn://anonsvn.kde.org/home/kde/");
    QSqlDatabase db = rl.Database();
    if (!db.isValid()) {
        std::cerr << "No database object." << std::endl;
        return 1;
    }
    list = db.tables();
    it = list.begin();
    while (it != list.end()) {
        std::cout << "Table: " << qPrintable(*it) << std::endl;
        ++it;
    }
    rl.cleanLogEntries();
    svn::LogEntriesMap lm;
    try {
        rl.simpleLog(lm, 100, 199);
    } catch (const svn::cache::DatabaseException &cl) {
        std::cerr << qPrintable(cl.msg()) << std::endl;
    } catch (const svn::Exception &ce) {
        std::cerr << "Exception: " << qPrintable(ce.msg()) << std::endl;
        return 2;
    }
    std::cout << "Count: " << lm.count() << std::endl;
    if (lm.count() != 100) {
        std::cerr << "got " << lm.count() << " log entries - expected 100" << std::endl;
        return 3;
    }

    svn::Revision r("{2014-09-27}");
    const svn::Revision rNumber = rl.date2numberRev(r);
    std::cout << qPrintable(r.toString()) << " -> " << rNumber.revnum() << std::endl;
    if (rNumber.revnum() != 1400899) {
        std::cerr << "expected revision number 1400899" << std::endl;
        return 4;
    }
    r = svn::Revision::HEAD;
    std::cout << qPrintable(rl.date2numberRev(r).toString()) << std::endl;

    // make sure we can insert this revision
    rl.cleanLogEntries();
    try {
        std::cout << "Trying to insert log entry with rev " << lm[100].revision << std::endl;
        rl.insertLogEntry(lm[100]);
    } catch (const svn::cache::DatabaseException &cl) {
        std::cerr << qPrintable(cl.msg()) << std::endl;
        return 5;
    }
    QSqlQuery q(db);
    QString stmt("insert into logentries(revision,date,author,message) values ('101','1122591406','alwin','copy and moving works now in basic form')");
    if (!q.exec(stmt)) {
        std::cerr << "\nSelf: \n" << qPrintable(q.lastError().text()) << std::endl;
        return 6;
    }

    db = QSqlDatabase();
    try {
        const QString svnPath(QLatin1String("/trunk/src/svnqt"));
        if (!rl.log(svn::Path(svnPath), 1, 1000, svn::Revision::UNDEFINED, lm, false, -1)) {
            std::cerr << "Could not retrieve log " << qPrintable(svnPath) << std::endl;
            return 7;
        }
    } catch (const svn::cache::DatabaseException &cl) {
        std::cerr << qPrintable(cl.msg()) << std::endl;
    } catch (const svn::Exception &ce) {
        std::cerr << "Exception: " << qPrintable(ce.msg()) << std::endl;
        return 8;
    }
    std::cout << "Count: " << lm.count() << std::endl;

    QStringList s; s << "a" << "b" << "c";

    svn::cache::ReposConfig::self()->setValue("http://www.alwins-world.de/repos/kdesvn", "bommel", s);
    list = svn::cache::ReposConfig::self()->readEntry("http://www.alwins-world.de/repos/kdesvn", "bommel", QStringList());
    std::cout << "Value: ";
    foreach (const QString &entry, list) {
        std::cout << qPrintable(entry) << ",";
    }
    std::cout << std::endl;
    return 0;
}
