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

#include "svnqt/client.h"
#include "svnqt/client_parameter.h"
#include "svnqt/status.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/tests/testconfig.h"
#include <iostream>

int main(int, char **)
{
    svn::ContextP m_CurrentContext(new svn::Context);
    svn::ClientP m_Svnclient = svn::Client::getobject(m_CurrentContext);

    svn::DirEntries dlist;
    QString p = QString("file://%1").arg(TESTREPOPATH);
    QString l = QString("%1").arg(TESTCOPATH);

    try {
        dlist = m_Svnclient->list(svn::Path(p), svn::Revision::HEAD, svn::Revision::HEAD, svn::DepthInfinity, true);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }
    std::cout << "List 1 " << dlist.size() << std::endl;
    Q_FOREACH (const svn::DirEntry &entry, dlist) {
        QDateTime dt(svn::DateTime(entry.time()).toQDateTime());
        std::cout << entry.name().toUtf8().data() << " " << entry.lastAuthor().toUtf8().data() << " " << entry.size() << " " << dt.toSecsSinceEpoch()
                  << std::endl;
    }
    try {
        dlist = m_Svnclient->list(svn::Path(p), svn::Revision::HEAD, svn::Revision::HEAD, svn::DepthImmediates, false);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }
    std::cout << "================" << std::endl;
    std::cout << "List 2 " << dlist.size() << std::endl;
    Q_FOREACH (const svn::DirEntry &entry, dlist) {
        QDateTime dt = svn::DateTime(entry.time()).toQDateTime();
        std::cout << entry.name().toUtf8().data() << " " << entry.lastAuthor().toUtf8().data() << " " << entry.size() << " " << dt.toSecsSinceEpoch()
                  << std::endl;
    }
    std::cout << "================" << std::endl;
    svn::StatusEntries slist;
    svn::StatusParameter params(p);
    try {
        slist = m_Svnclient->status(
            params.depth(svn::DepthInfinity).all(true).update(true).noIgnore(true).revision(svn::Revision::HEAD).detailedRemote(true).ignoreExternals(false));
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }
    for (int i = 0; i < slist.size(); ++i) {
        std::cout << slist[i]->path().toUtf8().data() << std::endl;
    }
    std::cout << "================" << std::endl;
    std::cout << "Second status:" << std::endl;

    try {
        slist = m_Svnclient->status(params.path(l)
                                        .depth(svn::DepthInfinity)
                                        .all(true)
                                        .update(true)
                                        .noIgnore(true)
                                        .revision(svn::Revision::WORKING)
                                        .detailedRemote(true)
                                        .ignoreExternals(false));
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }
    for (int i = 0; i < slist.size(); ++i) {
        std::cout << slist[i]->path().toUtf8().data() << std::endl;
    }

    return 0;
}
