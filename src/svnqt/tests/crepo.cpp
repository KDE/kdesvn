/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "src/svnqt/client.hpp"
#include "src/svnqt/tests/testconfig.h"
#include "src/svnqt/repository.hpp"
#include "src/svnqt/repositorylistener.hpp"
#include "src/svnqt/targets.hpp"

#include "testlistener.h"

#include <iostream>
#include <unistd.h>
#include <qstringlist.h>

class Listener:public svn::repository::RepositoryListener
{
    public:
        Listener(){}
        virtual ~Listener(){}
        virtual void sendWarning(const QString&msg)
        {
            std::cout << msg.toAscii().data() << std::endl;
        }
        virtual void sendError(const QString&msg)
        {
            std::cout << msg.toAscii().data() << std::endl;
        }
        virtual bool isCanceld(){return false;}
};

int main(int,char**)
{
    QString p = TESTREPOPATH;
    Listener ls;
    svn::repository::Repository rp(&ls);
    try {
        rp.CreateOpen(p,"fsfs");
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8().data() << std::endl;
        return -1;
    }

    svn::ContextP m_CurrentContext;
    svn::Client* m_Svnclient;
    m_Svnclient=svn::Client::getobject(0,0);
    TestListener tl;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(&tl);
    p = "file://"+p;

    m_Svnclient->setContext(m_CurrentContext);
    QStringList s; s.append(p+"/trunk"); s.append(p+"/branches"); s.append(p+"/tags");

    try {
        m_Svnclient->mkdir(svn::Targets(s),"Test mkdir");
        m_Svnclient->checkout(p,TESTCOPATH,svn::Revision::HEAD,svn::Revision::HEAD,svn::DepthInfinity,false);
    } catch (svn::ClientException e) {
        QString ex = e.msg();
        std::cout << ex.TOUTF8().data() << std::endl;
        return -1;
    }

    return 0;
}
