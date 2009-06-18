/***************************************************************************
 *   Copyright (C) 2006-2007 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#include "client.hpp"
#include "repository.hpp"
#include "context.hpp"
#include "datetime.hpp"
#include "client_parameter.hpp"

#include <qdatastream.h>

int main(int,char**)
{
    svn::Client::getobject(0,0);
    svn::repository::Repository rep(0L);
    svn::ContextP myContext = new svn::Context();

    QByteArray tout;
    svn::Client*m_Svnclient = svn::Client::getobject(0,0);
    svn::ContextP m_CurrentContext = new svn::Context();
    m_Svnclient->setContext(m_CurrentContext);
    bool gotit = true;
    svn::LogEntriesMap m_OldHistory;
    svn::LogParameter params;

    try {
        m_Svnclient->log(params.targets("http://www.alwins-world.de/repos/kdesvn/trunk").revisionRange(svn::Revision::HEAD,20).peg(svn::Revision::UNDEFINED).discoverChangedPathes(true).
        strictNodeHistory(false).limit(0),m_OldHistory);
    } catch (svn::ClientException ce) {
        gotit = false;
    }
  return 1;
}
