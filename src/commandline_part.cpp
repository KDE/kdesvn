/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "commandline_part.h"
#include "kdesvn_part.h"
#include "settings.h"
#include "svnfrontend/svnactions.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>

class pCPart
{
public:
    pCPart();
    ~pCPart();

    QString cmd;
    QString url;
    bool ask_revision;
    SvnActions*m_SvnWrapper;
};

pCPart::pCPart()
    :cmd(""),url(""),ask_revision(false)
{
    m_SvnWrapper = 0;
}

pCPart::~pCPart()
{
    delete m_SvnWrapper;
}

commandline_part::commandline_part(QObject *parent, const char *name, const QStringList &args )
 : QObject(parent, name)
{
    KGlobal::locale()->insertCatalogue("kdesvn");
    KInstance * inst = kdesvnPartFactory::instance();
    KGlobal::locale()->insertCatalogue(inst->instanceName());
    KGlobal::dirs()->addResourceType( inst->instanceName() + "data",
        KStandardDirs::kde_default("data")+ QString::fromLatin1( inst->instanceName() ) + '/' );

    m_pCPart = new pCPart;
    QStringList _a = args;
    if (_a.count()>0) {
        m_pCPart->cmd = _a[0];
        _a.erase(_a.begin());
    }

    if (_a.count()>0) {
        m_pCPart->url = _a[_a.count()-1];
    }
    while (m_pCPart->url.endsWith("/")) {
        m_pCPart->url.truncate(m_pCPart->url.length()-1);
    }

    m_pCPart->m_SvnWrapper = new SvnActions(0);
    connect(m_pCPart->m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(clientException(const QString&)));
    m_pCPart->m_SvnWrapper->reInitClient();
}


commandline_part::~commandline_part()
{
    delete m_pCPart;
}

int commandline_part::exec()
{
    kdDebug()<<"Executing " << m_pCPart->cmd<<endl;
    if (m_pCPart->cmd == "log") {
        svn::Revision start(svn::Revision::START);
        svn::Revision end(svn::Revision::HEAD);
        bool list = Settings::self()->log_always_list_changed_files();
        m_pCPart->m_SvnWrapper->makeLog(start,end,m_pCPart->url,list);
    }
    return 0;
}



/*!
    \fn commandline_part::clientException(const QString&)
 */
void commandline_part::clientException(const QString&what)
{
    KMessageBox::sorry(0,what,i18n("SVN Error"));
}

#include "commandline_part.moc"
