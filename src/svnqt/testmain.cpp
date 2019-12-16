/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
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
#include "client.h"
#include "repository.h"
#include "context.h"
#include "datetime.h"
#include "client_parameter.h"

int main(int, char **)
{
    svn::repository::Repository rep(0L);

    svn::ContextP m_CurrentContext(new svn::Context);
    svn::ClientP m_Svnclient = svn::Client::getobject(m_CurrentContext);
    bool gotit = true;
    svn::LogEntriesMap m_OldHistory;
    svn::LogParameter params;
    svn::Paths s;
    s.append(svn::Path(QLatin1String("svn://anonsvn.kde.org/home/kde/")));

    try {
        m_Svnclient->log(params.targets(svn::Targets(s)).revisionRange(svn::Revision::HEAD, 20).peg(svn::Revision::UNDEFINED).discoverChangedPathes(true).
                         strictNodeHistory(false).limit(0), m_OldHistory);
    } catch (const svn::ClientException &ce) {
        gotit = false;
    }
    return gotit ? 0 : 1;
}
