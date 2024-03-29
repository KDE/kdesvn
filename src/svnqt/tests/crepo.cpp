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
#include "svnqt/repoparameter.h"
#include "svnqt/repository.h"
#include "svnqt/repositorylistener.h"
#include "svnqt/targets.h"
#include "svnqt/tests/testconfig.h"

#include "testlistener.h"

#include <iostream>
#include <qstringlist.h>
#include <unistd.h>

class Listener : public svn::repository::RepositoryListener
{
public:
    Listener()
    {
    }
    virtual ~Listener()
    {
    }
    virtual void sendWarning(const QString &msg)
    {
        std::cout << msg.toLatin1().data() << std::endl;
    }
    virtual void sendError(const QString &msg)
    {
        std::cout << msg.toLatin1().data() << std::endl;
    }
    virtual bool isCanceld()
    {
        return false;
    }
};

int main(int, char **)
{
    QString p = TESTREPOPATH;
    Listener ls;
    svn::repository::Repository rp(&ls);
    try {
        rp.CreateOpen(svn::repository::CreateRepoParameter().path(p).fstype("fsfs"));
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }

    svn::ClientP m_Svnclient = svn::Client::getobject(svn::ContextP());
    TestListener tl;
    svn::ContextP m_CurrentContext(new svn::Context);
    m_CurrentContext->setListener(&tl);
    p = "file://" + p;

    m_Svnclient->setContext(m_CurrentContext);
    svn::Paths s;
    s.append(svn::Path(p + QLatin1String("/trunk")));
    s.append(svn::Path(p + QLatin1String("/branches")));
    s.append(svn::Path(p + QLatin1String("/tags")));
    svn::CheckoutParameter cparams;
    cparams.moduleName(p).destination(QString::fromLatin1(TESTCOPATH)).revision(svn::Revision::HEAD).peg(svn::Revision::HEAD).depth(svn::DepthInfinity);

    try {
        m_Svnclient->mkdir(svn::Targets(s), "Test mkdir");
        m_Svnclient->checkout(cparams);
    } catch (const svn::ClientException &e) {
        QString ex = e.msg();
        std::cout << ex.toUtf8().data() << std::endl;
        return -1;
    }
    return 0;
}
