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
#include <kcmdlineargs.h>

class pCPart
{
public:
    pCPart();
    ~pCPart();

    QString cmd;
    QString url;
    bool ask_revision;
    bool rev_set;
    SvnActions*m_SvnWrapper;
    KCmdLineArgs *args;
    svn::Revision start,end;
};

pCPart::pCPart()
    :cmd(""),url(""),ask_revision(false),rev_set(false)
{
    m_SvnWrapper = 0;
    start = svn::Revision::START;
    end = svn::Revision::HEAD;
}

pCPart::~pCPart()
{
    delete m_SvnWrapper;
}

commandline_part::commandline_part(QObject *parent, const char *name,KCmdLineArgs *args)
 : QObject(parent, name)
{
    KGlobal::locale()->insertCatalogue("kdesvn");
    KInstance * inst = kdesvnPartFactory::instance();
    KGlobal::locale()->insertCatalogue(inst->instanceName());
    KGlobal::dirs()->addResourceType( inst->instanceName() + "data",
        KStandardDirs::kde_default("data")+ QString::fromLatin1( inst->instanceName() ) + '/' );

    m_pCPart = new pCPart;
    m_pCPart->args = args;

    m_pCPart->m_SvnWrapper = new SvnActions(0);
    connect(m_pCPart->m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(clientException(const QString&)));
    connect(m_pCPart->m_SvnWrapper,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    m_pCPart->m_SvnWrapper->reInitClient();
}

commandline_part::~commandline_part()
{
    delete m_pCPart;
}

int commandline_part::exec()
{
    if (!m_pCPart->args) {
        return -1;
    }
    for (int i = 0; i < m_pCPart->args->count();++i) {
        kdDebug()<<"Arg at " << i << " == " << m_pCPart->args->arg(i)<<endl;
    }
    if (m_pCPart->args->count()>=2) {
        m_pCPart->cmd=m_pCPart->args->arg(1);
        m_pCPart->cmd=m_pCPart->cmd.lower();
    }
    QString slotCmd;
    if (!QString::compare(m_pCPart->cmd,"log")) {
        slotCmd=SLOT(slotCmd_log());
    } else if (!QString::compare(m_pCPart->cmd,"cat")) {
        slotCmd=SLOT(slotCmd_cat());
    } else if (!QString::compare(m_pCPart->cmd,"help")) {
        slotCmd=SLOT(slotCmd_help());
    } else if (!QString::compare(m_pCPart->cmd,"blame")||
               !QString::compare(m_pCPart->cmd,"annotate")) {
        slotCmd=SLOT(slotCmd_blame());
    } else if (!QString::compare(m_pCPart->cmd,"update")) {
        slotCmd=SLOT(slotCmd_update());
    } else if (!QString::compare(m_pCPart->cmd,"diff")) {
        slotCmd=SLOT(slotCmd_diff());
    }

    bool found = connect(this,SIGNAL(executeMe()),this,slotCmd.ascii());
    if (!found) {
        slotCmd=i18n("Command \"%1\" not implemented or known").arg(m_pCPart->cmd);
        KMessageBox::sorry(0,slotCmd,i18n("SVN Error"));
        return -1;
    }

    if (m_pCPart->args->count()>=3) {
        m_pCPart->url=m_pCPart->args->arg(2);
    }

    while (m_pCPart->url.endsWith("/")) {
        m_pCPart->url.truncate(m_pCPart->url.length()-1);
    }

    if (m_pCPart->url.isEmpty()) {
        m_pCPart->url = ".";
    }

    if (m_pCPart->args->isSet("R")) {
        m_pCPart->ask_revision = true;
    } else if (m_pCPart->args->isSet("r")){
        scanRevision();
    }

    emit executeMe();
    return 0;
}



/*!
    \fn commandline_part::clientException(const QString&)
 */
void commandline_part::clientException(const QString&what)
{
    kdDebug()<<what<<endl;
    KMessageBox::sorry(0,what,i18n("SVN Error"));
}


/*!
    \fn commandline_part::slotCmdLog
 */
void commandline_part::slotCmd_log()
{
    kdDebug()<<"Executing log" << endl;
    bool list = Settings::self()->log_always_list_changed_files();
    m_pCPart->m_SvnWrapper->makeLog(m_pCPart->start,m_pCPart->end,m_pCPart->url,list);
}

void commandline_part::slotCmd_blame()
{
    kdDebug()<<"Executing blame" << endl;
    m_pCPart->m_SvnWrapper->makeBlame(m_pCPart->start,m_pCPart->end,m_pCPart->url);
}

void commandline_part::slotCmd_update()
{
    kdDebug()<<"Executing update" << endl;
    QStringList t;
    t.append(m_pCPart->url);
    m_pCPart->m_SvnWrapper->makeUpdate(t,
        (m_pCPart->rev_set?m_pCPart->start:m_pCPart->end),true);
}

void commandline_part::slotCmd_diff()
{
    kdDebug()<<"Executing diff" << endl;
    m_pCPart->m_SvnWrapper->makeDiff(m_pCPart->url,m_pCPart->start,m_pCPart->end);
}

/*!
    \fn commandline_part::scanRevision()
 */
bool commandline_part::scanRevision()
{
    /// @todo scan for date-ranges, too.
    QString revstring = m_pCPart->args->getOption("r");
    QStringList revl = QStringList::split(":",revstring);
    if (revl.count()==0) {
        return false;
    }
    m_pCPart->start=revl[0].toInt();
    m_pCPart->rev_set=true;
    if (revl.count()>1) {
        m_pCPart->end=revl[1].toInt();
    }
    return true;
}

void commandline_part::slotNotifyMessage(const QString&msg)
{
    kdDebug()<<msg<<endl;
}

#include "commandline_part.moc"
