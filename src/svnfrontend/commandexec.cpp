/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include "commandexec.h"
#include "src/settings/kdesvnsettings.h"
#include "svnfrontend/svnactions.h"
#include "svnfrontend/dummydisplay.h"
#include "src/svnqt/targets.h"
#include "src/svnqt/url.h"
#include "src/svnqt/dirent.h"
#include "src/helpers/sub2qt.h"
#include "src/helpers/ktranslateurl.h"
#include "src/helpers/sshagent.h"
#include "src/svnfrontend/fronthelpers/rangeinput_impl.h"
#include "src/svnfrontend/copymoveview_impl.h"

#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klocale.h>
// #include <kdialogbase.h>
#include <KDialog>
#include <KVBox>
#include <ktextbrowser.h>

#include <qfile.h>
#include <QTextStream>

class pCPart
{
public:
    pCPart();
    ~pCPart();

    QString cmd;
    QStringList url;
    bool ask_revision;
    bool rev_set;
    bool outfile_set;
    bool single_revision;
    bool force;
    int log_limit;
    SvnActions*m_SvnWrapper;
    KCmdLineArgs *args;
    svn::Revision start,end;

    // for output
    QString outfile;
    QTextStream Stdout,Stderr;
    DummyDisplay * disp;
    QMap<int,svn::Revision> extraRevisions;
    QMap<int,QString> baseUrls;
};

pCPart::pCPart()
    :cmd(""),url(),ask_revision(false),rev_set(false),outfile_set(false),single_revision(false),log_limit(0),Stdout(stdout),Stderr(stderr)
{
    m_SvnWrapper = 0;
    start = svn::Revision::UNDEFINED;
    end = svn::Revision::UNDEFINED;
    disp = new DummyDisplay();
    m_SvnWrapper = new SvnActions(disp,0,true);
}

pCPart::~pCPart()
{
    delete m_SvnWrapper;
    delete disp;
}

CommandExec::CommandExec(QObject*parent)
    : QObject(parent)
{
    m_pCPart = new pCPart;
    m_pCPart->args = 0;
    SshAgent ag;
    ag.querySshAgent();

    connect(m_pCPart->m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(clientException(const QString&)));
    connect(m_pCPart->m_SvnWrapper,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    m_pCPart->m_SvnWrapper->reInitClient();
}


CommandExec::~CommandExec()
{
    delete m_pCPart;
}

int CommandExec::exec(KCmdLineArgs*args)
{
    m_pCPart->args=args;
    if (!m_pCPart->args) {
        return -1;
    }
    m_lastMessages = "";
    m_lastMessagesLines = 0;
    m_pCPart->m_SvnWrapper->reInitClient();
    bool dont_check_second = false;
    bool dont_check_all = false;
    bool path_only = false;
    bool no_revision = false;
    bool check_force=false;

    if (m_pCPart->args->count()>=2) {
        m_pCPart->cmd=m_pCPart->args->arg(1);
        m_pCPart->cmd=m_pCPart->cmd.toLower();
    }
    QString slotCmd;
    if (!QString::compare(m_pCPart->cmd,"log")) {
        slotCmd=SLOT(slotCmd_log());
    } else if (!QString::compare(m_pCPart->cmd,"cat")) {
        slotCmd=SLOT(slotCmd_cat());
        m_pCPart->single_revision=true;
    } else if (!QString::compare(m_pCPart->cmd,"get")) {
        slotCmd=SLOT(slotCmd_get());
        m_pCPart->single_revision=true;
    } else if (!QString::compare(m_pCPart->cmd,"help")) {
        slotCmd=SLOT(slotCmd_help());
    } else if (!QString::compare(m_pCPart->cmd,"blame")||
               !QString::compare(m_pCPart->cmd,"annotate")) {
        slotCmd=SLOT(slotCmd_blame());
    } else if (!QString::compare(m_pCPart->cmd,"update")) {
        slotCmd=SLOT(slotCmd_update());
        m_pCPart->single_revision=true;
    } else if (!QString::compare(m_pCPart->cmd,"diff")) {
        m_pCPart->start = svn::Revision::WORKING;
        slotCmd=SLOT(slotCmd_diff());
    } else if (!QString::compare(m_pCPart->cmd,"info")) {
        slotCmd=SLOT(slotCmd_info());
        m_pCPart->single_revision=true;
    } else if (!QString::compare(m_pCPart->cmd,"commit")||
               !QString::compare(m_pCPart->cmd,"ci")) {
        slotCmd=SLOT(slotCmd_commit());
    } else if (!QString::compare(m_pCPart->cmd,"list")||
               !QString::compare(m_pCPart->cmd,"ls")) {
        slotCmd=SLOT(slotCmd_list());
    } else if (!QString::compare(m_pCPart->cmd,"copy")||
               !QString::compare(m_pCPart->cmd,"cp")) {
        slotCmd=SLOT(slotCmd_copy());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"move")||
               !QString::compare(m_pCPart->cmd,"rename")||
               !QString::compare(m_pCPart->cmd,"mv")) {
        slotCmd=SLOT(slotCmd_move());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"checkout")||
               !QString::compare(m_pCPart->cmd,"co")) {
        slotCmd=SLOT(slotCmd_checkout());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"checkoutto")||
               !QString::compare(m_pCPart->cmd,"coto")) {
        slotCmd=SLOT(slotCmd_checkoutto());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"export")) {
        slotCmd=SLOT(slotCmd_export());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"exportto")) {
        slotCmd=SLOT(slotCmd_exportto());
        dont_check_second = true;
    } else if (!QString::compare(m_pCPart->cmd,"delete")||
               !QString::compare(m_pCPart->cmd,"del")||
               !QString::compare(m_pCPart->cmd,"rm")||
               !QString::compare(m_pCPart->cmd,"remove")) {
        slotCmd=SLOT(slotCmd_delete());
    } else if (!QString::compare(m_pCPart->cmd,"add")) {
        slotCmd=SLOT(slotCmd_add());
        dont_check_all = true;
        path_only=true;
    } else if (!QString::compare(m_pCPart->cmd,"undo")||
               !QString::compare(m_pCPart->cmd,"revert")) {
        slotCmd=SLOT(slotCmd_revert());
    } else if (!QString::compare(m_pCPart->cmd,"checknew")||
               !QString::compare(m_pCPart->cmd,"addnew")) {
        slotCmd=SLOT(slotCmd_addnew());
    } else if (!QString::compare(m_pCPart->cmd,"switch")) {
        slotCmd=SLOT(slotCmd_switch());
    } else if (!QString::compare(m_pCPart->cmd,"tree")) {
        slotCmd=SLOT(slotCmd_tree());
    } else if (!QString::compare(m_pCPart->cmd,"lock")) {
        slotCmd=SLOT(slotCmd_lock());
        no_revision = true;
        check_force=true;
    } else if (!QString::compare(m_pCPart->cmd,"unlock")) {
        slotCmd=SLOT(slotCmd_unlock());
        no_revision=true;
        check_force=true;
    }

    bool found = connect(this,SIGNAL(executeMe()),this,slotCmd.toAscii());
    if (!found) {
        slotCmd=i18n("Command \"%1\" not implemented or known",m_pCPart->cmd);
        KMessageBox::sorry(0,slotCmd,i18n("SVN Error"));
        return -1;
    }

    QString tmp,query,proto,v;
    QMap<QString,QString> q;

    KUrl tmpurl;
    QString mainProto;
    QString _baseurl;
    for (int j = 2; j<m_pCPart->args->count();++j) {
        tmpurl = helpers::KTranslateUrl::translateSystemUrl(m_pCPart->args->url(j).prettyUrl());
        query = tmpurl.query();
        q = m_pCPart->args->url(j).queryItems();
        if (q.find("rev")!=q.end()) {
             v = q["rev"];
        } else {
            v = "";
        }
        tmpurl.setProtocol(svn::Url::transformProtokoll(tmpurl.protocol()));
        if (tmpurl.protocol().indexOf("ssh")!=-1) {
            SshAgent ag;
            // this class itself checks if done before
            ag.addSshIdentities();
        }
        m_pCPart->extraRevisions[j-2]=svn::Revision::HEAD;

        if (tmpurl.isLocalFile() && (j==2 || !dont_check_second) && !dont_check_all) {
            if (m_pCPart->m_SvnWrapper->isLocalWorkingCopy("file://"+tmpurl.path(),_baseurl)) {
                tmp = tmpurl.path();
                m_pCPart->baseUrls[j-2]=_baseurl;
                m_pCPart->extraRevisions[j-2]=svn::Revision::WORKING;
                if (j==2) mainProto = "";
            } else {
                tmp = "file://"+tmpurl.path();
                if (j==2) mainProto = "file://";
            }
        } else if (path_only){
            tmp = tmpurl.path();
        } else {
            tmp = tmpurl.url();
            if (j==2) mainProto=tmpurl.protocol();
        }
        if ( (j>2 && dont_check_second) || dont_check_all) {
            if (mainProto.isEmpty()) {
                tmp = tmpurl.path();
            }
        }
        QStringList l = tmp.split('?',QString::SkipEmptyParts);
        if (l.count()>0) {
            tmp=l[0];
        }
        while (tmp.endsWith('/')) {
            tmp.truncate(tmp.length()-1);
        }
        m_pCPart->url.append(tmp);
        if ( (j>2 && dont_check_second) || dont_check_all) {
            continue;
        }
        svn::Revision re = v;
        if (re) {
            m_pCPart->extraRevisions[j-2]=re;
        }
    }
    if (m_pCPart->url.count()==0) {
        m_pCPart->url.append(".");
    }

    if (!no_revision)
    {
        if (m_pCPart->args->isSet("R"))
        {
            m_pCPart->ask_revision = true;
            if (!askRevision())
            {
                return 0;
            }
        }
        else if (m_pCPart->args->isSet("r"))
        {
            scanRevision();
        }
    }

    m_pCPart->force=check_force && m_pCPart->args->isSet("f");

    if (m_pCPart->args->isSet("o")) {
        m_pCPart->outfile_set=true;
        m_pCPart->outfile = m_pCPart->args->getOption("o");
    }
    if (m_pCPart->args->isSet("l")) {
        QString s = m_pCPart->args->getOption("l");
        m_pCPart->log_limit = s.toInt();
        if (m_pCPart->log_limit<0) {
            m_pCPart->log_limit = 0;
        }
    }

    emit executeMe();
    if (Kdesvnsettings::self()->cmdline_show_logwindow() &&
        m_lastMessagesLines >= Kdesvnsettings::self()->cmdline_log_minline()) {
        KDialog dlg(KApplication::activeModalWidget());
        KVBox *Dialog1Layout = new KVBox(&dlg);
        dlg.setMainWidget(Dialog1Layout);
        KTextBrowser*ptr = new KTextBrowser(Dialog1Layout);
        ptr->setText(m_lastMessages);
        KConfigGroup _k(Kdesvnsettings::self()->config(),"kdesvn_cmd_log");
        dlg.restoreDialogSize(_k);
        dlg.exec();
        dlg.saveDialogSize(_k);
    }
    return 0;
}



/*!
    \fn CommandExec::clientException(const QString&)
 */
void CommandExec::clientException(const QString&what)
{
    m_pCPart->Stderr<<what<<endl;
    KMessageBox::sorry(0,what,i18n("SVN Error"));
}


void CommandExec::slotCmd_log()
{
    int limit = m_pCPart->log_limit;
    if (m_pCPart->end == svn::Revision::UNDEFINED) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (m_pCPart->start == svn::Revision::UNDEFINED) {
        m_pCPart->start = 1;
    }
    bool list = Kdesvnsettings::self()->log_always_list_changed_files();
    if (m_pCPart->extraRevisions[0]==svn::Revision::WORKING) {
        m_pCPart->extraRevisions[0]=svn::Revision::UNDEFINED;
    }
    m_pCPart->m_SvnWrapper->makeLog(m_pCPart->start,m_pCPart->end,m_pCPart->extraRevisions[0],m_pCPart->url[0],
            Kdesvnsettings::log_follows_nodes(),
            list,
            limit);
}

void CommandExec::slotCmd_tree()
{
    if (m_pCPart->end == svn::Revision::UNDEFINED) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (m_pCPart->start == svn::Revision::UNDEFINED) {
        m_pCPart->start = 1;
    }
    m_pCPart->m_SvnWrapper->makeTree(m_pCPart->url[0],m_pCPart->extraRevisions[0],m_pCPart->start,m_pCPart->end);
}

void CommandExec::slotCmd_checkout()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(m_pCPart->url[0],false);
}

void CommandExec::slotCmd_checkoutto()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(m_pCPart->url[0],false,true);
}

void CommandExec::slotCmd_export()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(m_pCPart->url[0],true);
}

void CommandExec::slotCmd_exportto()
{
    m_pCPart->m_SvnWrapper->CheckoutExport(m_pCPart->url[0],true,true);
}

void CommandExec::slotCmd_blame()
{
    if (!m_pCPart->end) {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (!m_pCPart->start) {
        m_pCPart->start = 1;
    }
    m_pCPart->m_SvnWrapper->makeBlame(m_pCPart->start,m_pCPart->end,m_pCPart->url[0]);
}

void CommandExec::slotCmd_cat()
{
    if (m_pCPart->extraRevisions.find(0)!=m_pCPart->extraRevisions.end()) {
        m_pCPart->rev_set=true;
        m_pCPart->start=m_pCPart->extraRevisions[0];
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    m_pCPart->m_SvnWrapper->slotMakeCat(
        (m_pCPart->rev_set?m_pCPart->start:m_pCPart->end),m_pCPart->url[0],m_pCPart->url[0]
        ,(m_pCPart->rev_set?m_pCPart->start:m_pCPart->end),0);
}

void CommandExec::slotCmd_get()
{
    if (m_pCPart->extraRevisions.find(0)!=m_pCPart->extraRevisions.end()) {
        m_pCPart->rev_set=true;
        m_pCPart->start=m_pCPart->extraRevisions[0];
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    if (!m_pCPart->outfile_set || m_pCPart->outfile.isEmpty()) {
        clientException(i18n("\"GET\" requires output file!"));
        return;
    }
    m_pCPart->m_SvnWrapper->makeGet((m_pCPart->rev_set?m_pCPart->start:m_pCPart->end),m_pCPart->url[0], m_pCPart->outfile,
        (m_pCPart->rev_set?m_pCPart->start:m_pCPart->end));
}

void CommandExec::slotCmd_update()
{
    m_pCPart->m_SvnWrapper->makeUpdate(m_pCPart->url,
        (m_pCPart->rev_set?m_pCPart->start:svn::Revision::HEAD),svn::DepthUnknown);
}

void CommandExec::slotCmd_diff()
{
    if (m_pCPart->url.count()==1) {
        if (!m_pCPart->rev_set && !svn::Url::isValid(m_pCPart->url[0])) {
            m_pCPart->start = svn::Revision::BASE;
            m_pCPart->end = svn::Revision::WORKING;
        }
        m_pCPart->m_SvnWrapper->makeDiff(m_pCPart->url[0],m_pCPart->start,m_pCPart->url[0],m_pCPart->end);
    } else {
        svn::Revision r1 = svn::Revision::HEAD;
        svn::Revision r2 = svn::Revision::HEAD;
        if (m_pCPart->extraRevisions.find(0)!=m_pCPart->extraRevisions.end()) {
            r1 = m_pCPart->extraRevisions[0];
        } else if (!svn::Url::isValid(m_pCPart->url[0])) {
            r1 = svn::Revision::WORKING;
        }
        if (m_pCPart->extraRevisions.find(1)!=m_pCPart->extraRevisions.end()) {
            r2 = m_pCPart->extraRevisions[1];
        } else if (!svn::Url::isValid(m_pCPart->url[1])) {
            r2 = svn::Revision::WORKING;
        }
        m_pCPart->m_SvnWrapper->makeDiff(m_pCPart->url[0],r1,m_pCPart->url[1],r2);
    }
}

void CommandExec::slotCmd_info()
{
    if (m_pCPart->extraRevisions.find(0)!=m_pCPart->extraRevisions.end()) {
        m_pCPart->rev_set=true;
        m_pCPart->start=m_pCPart->extraRevisions[0];
    }
    m_pCPart->m_SvnWrapper->makeInfo(m_pCPart->url,(m_pCPart->rev_set?m_pCPart->start:m_pCPart->end),svn::Revision::UNDEFINED,false);
}

void CommandExec::slotCmd_commit()
{
    QStringList targets;
    for (long j=0; j<m_pCPart->url.count();++j) {
        targets.push_back(svn::Path(m_pCPart->url[j]));
    }
    m_pCPart->m_SvnWrapper->makeCommit(svn::Targets(targets));
}

void CommandExec::slotCmd_list()
{
    svn::DirEntries res;
    svn::Revision rev = m_pCPart->end;
    if (m_pCPart->rev_set){
        rev = m_pCPart->start;
    } else if (m_pCPart->extraRevisions[0]) {
        rev = m_pCPart->extraRevisions[0];
    }
    if (!m_pCPart->m_SvnWrapper->makeList(m_pCPart->url[0],res,rev,svn::DepthInfinity)) {
        return;
    }
    for (long i = 0; i < res.count();++i) {
        QString d = svn::DateTime(res[i]->time()).toString(QString("yyyy-MM-dd hh:mm::ss"));
        m_pCPart->Stdout
            << (res[i]->kind()==svn_node_dir?"D":"F")<<" "
            << d << " "
            << res[i]->name()<<endl;
    }
}

void CommandExec::slotCmd_copy()
{
    QString target;
    if (m_pCPart->url.count()<2) {
        bool force_move,ok;
        target = CopyMoveView_impl::getMoveCopyTo(&ok,&force_move,false,
            m_pCPart->url[0],"",0,"move_name");
        if (!ok) {
            return;
        }
    } else {
        target = m_pCPart->url[1];
    }
    if (m_pCPart->extraRevisions.find(0)!=m_pCPart->extraRevisions.end()) {
        m_pCPart->rev_set=true;
        m_pCPart->start=m_pCPart->extraRevisions[0];
    } else {
        m_pCPart->end = svn::Revision::HEAD;
    }
    m_pCPart->m_SvnWrapper->makeCopy(m_pCPart->url[0],target,(m_pCPart->rev_set?m_pCPart->start:m_pCPart->end));
}

void CommandExec::slotCmd_move()
{
    bool force_move,ok;
    force_move = false;
    QString target;
    if (m_pCPart->url.count()<2) {
        target = CopyMoveView_impl::getMoveCopyTo(&ok,&force_move,true,
            m_pCPart->url[0],"",0,"move_name");
        if (!ok) {
            return;
        }
    } else {
        target = m_pCPart->url[1];
    }
    m_pCPart->m_SvnWrapper->makeMove(m_pCPart->url[0],target,force_move);
}

void CommandExec::slotCmd_delete()
{
    m_pCPart->m_SvnWrapper->makeDelete(m_pCPart->url);
}

void CommandExec::slotCmd_add()
{
    m_pCPart->m_SvnWrapper->addItems(m_pCPart->url,svn::DepthInfinity);
}

void CommandExec::slotCmd_revert()
{
    m_pCPart->m_SvnWrapper->slotRevertItems(m_pCPart->url,false);
}

void CommandExec::slotCmd_addnew()
{
    m_pCPart->m_SvnWrapper->checkAddItems(m_pCPart->url[0]);
}

/*!
    \fn CommandExec::scanRevision()
 */
bool CommandExec::scanRevision()
{
    QString revstring = m_pCPart->args->getOption("r");
    QStringList revl = revstring.split(':',QString::SkipEmptyParts);
    if (revl.count()==0) {
        return false;
    }
    m_pCPart->start = revl[0];
    if (revl.count()>1) {
        m_pCPart->end = revl[1];
    }
    m_pCPart->rev_set=true;
    return true;
}

void CommandExec::slotNotifyMessage(const QString&msg)
{
    m_pCPart->m_SvnWrapper->slotExtraLogMsg(msg);
    if (Kdesvnsettings::self()->cmdline_show_logwindow()) {
        ++m_lastMessagesLines;
        if (!m_lastMessages.isEmpty()) m_lastMessages.append("\n");
        m_lastMessages.append(msg);
    }
}

bool CommandExec::askRevision()
{
    QString _head = m_pCPart->cmd+" - Revision";
    KDialog dlg(0);
    dlg.setButtons(KDialog::Ok | KDialog::Cancel);
    KVBox *Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    Rangeinput_impl*rdlg;
    rdlg = new Rangeinput_impl(Dialog1Layout);
    dlg.resize( QSize(120,60).expandedTo(dlg.minimumSizeHint()));
    rdlg->setStartOnly(m_pCPart->single_revision);
    if (dlg.exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range range = rdlg->getRange();
        m_pCPart->start = range.first;
        m_pCPart->end = range.second;
        m_pCPart->rev_set = true;
        return true;
    }
    return false;
}


/*!
    \fn CommandExec::slotCmd_switch()
 */
void CommandExec::slotCmd_switch()
{
    QString base;
    if (m_pCPart->url.count()>1) {
        clientException(i18n("May only switch one url at time!"));
        return;
    }
    if (m_pCPart->baseUrls.find(0)==m_pCPart->baseUrls.end()) {
        clientException(i18n("Switch only on working copies!"));
        return;
    }
    base = m_pCPart->baseUrls[0];
    m_pCPart->m_SvnWrapper->makeSwitch(m_pCPart->url[0],base);
}

void CommandExec::slotCmd_lock()
{
//     m_pCPart->m_SvnWrapper->makeLock(m_pCPart->url[0],"",m_pCPart->force);
    m_pCPart->m_SvnWrapper->makeLock(m_pCPart->url,"",m_pCPart->force);
}

void CommandExec::slotCmd_unlock()
{
//     m_pCPart->m_SvnWrapper->makeUnlock(m_pCPart->url[0],m_pCPart->force);
    m_pCPart->m_SvnWrapper->makeUnlock(m_pCPart->url,m_pCPart->force);
}

#include "commandexec.moc"
