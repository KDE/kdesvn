/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "svnactions.h"
#include "checkoutinfo_impl.h"
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "rangeinput_impl.h"
#include "propertiesdlg.h"
#include "ccontextlistener.h"
#include "svnlogdlgimp.h"
#include "stopdlg.h"
#include "logmsg_impl.h"
#include "svncpp/client.hpp"
#include "svncpp/annotate_line.hpp"
#include "svncpp/context_listener.hpp"
#include "svncpp/targets.hpp"
#include "helpers/dialog_template.h"

#include <qstring.h>
#include <qmap.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <kdialog.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <qvaluelist.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kdialogbase.h>

#if 0
#include <khtml_part.h>
#include <khtmlview.h>
#endif

SvnActions::SvnActions(QObject *parent, const char *name)
 : QObject(parent, name),m_ParentList(0),m_SvnContext(new CContextListener(this)),m_CurrentContext(0)
{
    connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
}

SvnActions::SvnActions(kdesvnfilelist *parent, const char *name)
 : QObject(parent, name),m_ParentList(parent),m_SvnContext(new CContextListener(this)),m_CurrentContext(0)
{
    connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
}

SvnActions::~SvnActions()
{
    if (m_CurrentContext) {
        delete m_CurrentContext;
        m_CurrentContext = 0;
    }
}

void SvnActions::slotNotifyMessage(const QString&aMsg)
{
    emit sendNotify(aMsg);
}

void SvnActions::reInitClient()
{
    if (m_CurrentContext) {
        delete m_CurrentContext;
        m_CurrentContext = 0;
    }
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(m_SvnContext);
    m_Svnclient.setContext(m_CurrentContext);
}

#include "svnactions.moc"

template<class T> KDialog* SvnActions::createDialog(T**ptr,const QString&_head,bool OkCancel)
{
    KDialog * dlg = new KDialog();
    if (!dlg) return dlg;
    dlg->setCaption(_head);
    QVBoxLayout* Dialog1Layout;

    QHBoxLayout* blayout;
    QSpacerItem* Spacer1;
    Dialog1Layout = new QVBoxLayout( dlg, 2, 2, "Dialog1Layout");
    *ptr = new T(dlg);

    Dialog1Layout->addWidget( *ptr );

    /* button */
    blayout = new QHBoxLayout( 0, 2, 2, "blayout");
    QPushButton*buttonOk = new QPushButton( dlg, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    blayout->addWidget( buttonOk );
    if (!OkCancel) {
        buttonOk->setText( i18n( "&Close" ) );
    } else {
        buttonOk->setText( i18n( "&Ok" ) );
        QPushButton*buttonCancel = new QPushButton( dlg, "buttonCancel" );
        buttonCancel->setText(i18n("&Cancel"));
        blayout->addWidget( buttonCancel );
        connect( buttonCancel, SIGNAL(clicked()), dlg, SLOT(reject()));
    }
    Spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    blayout->addItem( Spacer1 );
    Dialog1Layout->addLayout(blayout);
    QObject::connect( buttonOk, SIGNAL( clicked() ), dlg, SLOT( accept() ) );

    dlg->resize( QSize(320,240).expandedTo(dlg->minimumSizeHint()) );
    return dlg;
}


/*!
    \fn SvnActions::slotMakeRangeLog()
 */
void SvnActions::slotMakeRangeLog()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    Rangeinput_impl*rdlg;
    QDialog*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true);
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        makeLog(r.first,r.second,k);
    }
}

/*!
    \fn SvnActions::slotMakeLog()
 */
void SvnActions::slotMakeLog()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    makeLog(start,end,k);
}

/*!
    \fn SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
 */
void SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
{
    const svn::LogEntries * logs;
    QString ex;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Logs","Getting logs - hit cancel for abort");
        /// @fixme Last parameter should be user setable (follow nodes moving or not)
        logs = m_Svnclient.log(k->fullName().local8Bit(),start,end,true,false);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    if (!logs) {
        ex = i18n("Got no logs");
        emit clientException(ex);
        return;
    }
    SvnLogDlgImp disp;
    disp.dispLog(logs,k->fullName());
    connect(&disp,SIGNAL(makeDiff(const QString&,const svn::Revision&,const svn::Revision&)),
            this,SLOT(makeDiff(const QString&,const svn::Revision&,const svn::Revision&)));
    disp.exec();
    delete logs;
}


/*!
    \fn SvnActions::slotBlame()
 */
void SvnActions::slotBlame()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    makeBlame(start,end,k);
}

/*!
    \fn SvnActions::makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k)
 */
void SvnActions::makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k)
{
    svn::AnnotatedFile * blame;
    QString ex;
    svn::Path p(k->fullName().local8Bit());

    try {
        StopDlg sdlg(m_SvnContext,0,0,"Annotate","Blaming - hit cancel for abort");
        blame = m_Svnclient.annotate(p,start,end);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    } catch (...) {
        ex = i18n("Error getting blame");
        emit clientException(ex);
        return;
    }
    if (!blame) {
        ex = i18n("Got no annotate");
        emit clientException(ex);
        return;
    }
    svn::AnnotatedFile::const_iterator bit;
    static QString rowb = "<tr><td>";
    static QString rowc = "<tr bgcolor=\"#EEEEEE\"><td>";
    static QString rows = "</td><td>";
    static QString rowe = "</td></tr>\n";
    QString text = "<html><table>"+rowb+"Rev"+rows+i18n("Author")+rows+i18n("Line")+rows+"&nbsp;"+rowe;
    bool second = false;
    QString codetext = "";
    for (bit=blame->begin();bit!=blame->end();++bit) {
        codetext = bit->line().c_str();
        codetext.replace(" ","&nbsp;");
        codetext.replace("\"","&quot;");
        codetext.replace("<","&lt;");
        codetext.replace(">","&gt;");
        codetext.replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
        text+=(second?rowb:rowc)+QString("%1").arg(bit->revision())+
            rows+QString("%1").arg(bit->author().c_str())+rows+
            QString("%1").arg(bit->lineNumber())+rows+
            QString("<code>%1</code>").arg(codetext)+rowe;
            second = !second;
    }
    text += "</table></html>";
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(i18n("Blame %1")).arg(k->text(0)));
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        delete dlg;
    }
    delete blame;
}

QDateTime SvnActions::apr2qttime(apr_time_t atime)
{
    QDateTime t;t.setTime_t(atime/(1000*1000),Qt::UTC);
    return t;
}

/*!
    \fn SvnActions::slotMakeRangeBlame()
 */
void SvnActions::slotRangeBlame()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    Rangeinput_impl*rdlg;
    QDialog*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true);
    if (!dlg) {
        return;
    }
    if (dlg->exec()==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        makeBlame(r.first,r.second,k);
    }
    delete dlg;
}

void SvnActions::makeCat(svn::Revision start, FileListViewItem*k)
{
    std::string content;
    QString ex;
    svn::Path p(k->fullName().local8Bit());
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Content cat","Getting content - hit cancel for abort");
        content = m_Svnclient.cat(p,start);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    } catch (...) {
        ex = i18n("Error getting content");
        emit clientException(ex);
        return;
    }
    if (content.size()==0) {
        emit clientException(i18n("Got no content"));
        return;
    }
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(i18n("Content of %1")).arg(k->text(0)));
    if (dlg) {
        ptr->setFont(KGlobalSettings::fixedFont());
        ptr->setWordWrap(QTextEdit::NoWrap);
        ptr->setText(QString::fromLocal8Bit(content.c_str()));
        dlg->exec();
        delete dlg;
    }
}

void SvnActions::slotCat()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    makeCat(svn::Revision::HEAD, k);
}

void SvnActions::slotMkdir()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    QString parentDir,ex;
    if (k) {
        if (!k->isDir()) {
            KMessageBox::sorry(0,i18n("May not make subdirs of a file"));
            return;
        }
        parentDir=k->fullName();
    } else {
        parentDir=m_ParentList->baseUri();
    }
    bool isOk=false;
    ex = KInputDialog::getText(i18n("New Dir"),i18n("Enter (sub-)directory name:"),QString::null,&isOk);
    if (!isOk) {
        return;
    }
    svn::Path target(parentDir.local8Bit());
    target.addComponent(ex.local8Bit());
    ex = "";
    std::string message;

    QString logMessage="";
    if (!m_ParentList->isLocal()) {
        isOk = false;
        logMessage = KInputDialog::getMultiLineText(i18n("Logmessage"),i18n("Enter a logmessage"),QString::null,&isOk);
        if (!isOk) {
            return;
        }
    }

    try {
        m_Svnclient.mkdir(target,logMessage.local8Bit());
    }catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }

    ex = target.path().c_str();
    emit dirAdded(ex,k);
}

void SvnActions::slotInfo()
{
    if (!m_ParentList) return;
    QPtrList<FileListViewItem> lst = m_ParentList->allSelected();
    QValueList<svn::Entry> entries;
    QString ex;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Details","Retrieving infos - hit cancel for abort");
        if (lst.count()==0) {
            entries.push_back(m_Svnclient.info(m_ParentList->baseUri().local8Bit()));
        } else {
            FileListViewItem*item;
            for (item=lst.first();item;item=lst.next()) {
                entries.push_back(m_Svnclient.info(item->fullName().local8Bit()));
            }
        }
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    QString text = "<html>";
    QValueList<svn::Entry>::const_iterator it;
    QString rb = "<tr><td align=\"right\"><b>";
    QString re = "</td></tr>\n";
    QString cs = "</b>:</td><td>";
    for (it=entries.begin();it!=entries.end();++it) {
        text+="<p><table>";
        if ((*it).name()&&strlen((*it).name())) {
            text+=rb+i18n("Name")+cs+QString((*it).name())+re;
        }
        text+=rb+i18n("URL")+cs+QString((*it).url())+re;
        if ((*it).repos()&&strlen((*it).repos())) {
            text+=rb+i18n("Canonical repository url")+cs+QString((*it).repos())+re;
        }
        text+=rb+i18n("Type")+cs;
        switch ((*it).kind()) {
        case svn_node_none:
            text+=i18n("Absent");
            break;
        case svn_node_file:
            text+=i18n("File");
            break;
        case svn_node_dir:
            text+=i18n("Directory");
            break;
        case svn_node_unknown:
        default:
            text+=i18n("Unknown");
            break;
        }
        text+=re;
        text+=rb+i18n("Schedule")+cs;
        switch ((*it).schedule()) {
        case svn_wc_schedule_normal:
            text+=i18n("Normal");
            break;
        case svn_wc_schedule_add:
            text+=i18n("Addition");
            break;
        case svn_wc_schedule_delete:
            text+=i18n("Deletion");
            break;
        case svn_wc_schedule_replace:
            text+=i18n("Replace");
            break;
        default:
            text+=i18n("Unknow");
            break;
        }
        text+=re;
        text+=rb+i18n("UUID")+cs+QString((*it).uuid())+re;
        text+=rb+i18n("Last author")+cs+QString((*it).cmtAuthor())+re;
        text+=rb+i18n("Last changed")+cs+apr2qttime((*it).cmtDate()).toString(Qt::LocalDate)+re;
        text+=rb+i18n("Property last changed")+cs+apr2qttime((*it).propTime()).toString(Qt::LocalDate)+re;
        text+=rb+i18n("Last revision")+cs+QString("%1").arg((*it).cmtRev())+re;
        if ((*it).conflictNew()&&strlen((*it).conflictNew())) {
            text+=rb+i18n("New version of conflicted file")+cs+QString((*it).conflictNew())+re;
        }
        if ((*it).conflictOld()&&strlen((*it).conflictOld())) {
            text+=rb+i18n("Old version of conflicted file")+cs+QString((*it).conflictOld())+re;
        }
        if ((*it).conflictWrk()&&strlen((*it).conflictWrk())) {
            text+=rb+i18n("Working version of conflicted file")+cs+QString((*it).conflictWrk())+re;
        }
        if ((*it).copyfromUrl()&&strlen((*it).copyfromUrl())) {
            text+=rb+i18n("Copy from URL")+cs+QString((*it).copyfromUrl())+re;
        }

        text+="</table></p>\n";
    }
    text+="</html>";
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(i18n("Infolist")));
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        delete dlg;
    }
}


/*!
    \fn SvnActions::slotProperties()
 */
void SvnActions::slotProperties()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (!k) return;
    PropertiesDlg dlg(k->fullName(),svnclient(),
        m_ParentList->isLocal()?svn::Revision::WORKING:svn::Revision::HEAD);
    connect(&dlg,SIGNAL(clientException(const QString&)),m_ParentList,SLOT(slotClientException(const QString&)));
    if (dlg.exec()!=QDialog::Accepted) {
        return;
    }
    QString ex;
    PropertiesDlg::tPropEntries setList;
    QValueList<QString> delList;
    dlg.changedItems(setList,delList);
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Applying properties","<center>Applying<br>hit cancel for abort</center>");
        unsigned int pos;
        for (pos = 0; pos<delList.size();++pos) {
            m_Svnclient.propdel(delList[pos].local8Bit(),svn::Path(k->fullName().local8Bit()),svn::Revision::HEAD);
        }
        PropertiesDlg::tPropEntries::Iterator it;
        for (it=setList.begin(); it!=setList.end();++it) {
            m_Svnclient.propset(it.key().local8Bit(),it.data().local8Bit(),svn::Path(k->fullName().local8Bit()),svn::Revision::HEAD);
        }
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    k->refreshStatus();
}


/*!
    \fn SvnActions::slotCommit()
 */
void SvnActions::slotCommit()
{
    Logmsg_impl*ptr;
    if (!m_ParentList->firstChild()) return;
    QDialog*dlg = createDialog(&ptr,QString(i18n("Commit log")),true);
    if (!dlg) return;
    ptr->initHistory();
    if (dlg->exec()!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    QString msg = ptr->getMessage();
    bool rec = ptr->isRecursive();
    ptr->saveHistory();
    QPtrList<FileListViewItem> which = m_ParentList->allSelected();
    FileListViewItem*cur;
    std::vector<svn::Path> targets;
    if (which.count()==0) {
        targets.push_back(svn::Path(m_ParentList->baseUri().local8Bit()));
    } else {
        for (cur=which.first();cur;cur=which.next()) {
            targets.push_back(svn::Path(cur->fullName().local8Bit()));
        }
    }
    svn_revnum_t nnum;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Commiting","Commiting - hit cancel for abort");
        nnum = m_Svnclient.commit(svn::Targets(targets),msg.local8Bit(),rec);
    } catch (svn::ClientException e) {
        QString ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    if (which.count()==0 && m_ParentList->firstChild()) {
        which.append(static_cast<FileListViewItem*>(m_ParentList->firstChild()));
    }
    for (unsigned j = 0; j<which.count();++j) {
        if (rec) {
            which.at(j)->refreshStatus(true,&which,false);
            which.at(j)->refreshStatus(false,&which,true);
        } else {
            which.at(j)->refreshStatus(false,&which,false);
        }
    }
}

/*!
    \fn SvnActions::slotSimpleDiffBase
    /// @error sinnlos da es nicht funktioniert :(
 */
void SvnActions::slotSimpleDiffBase()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    QString what;
    if (k) {
        what=k->fullName();
    } else {
        if (m_ParentList->isLocal()) {
            what=m_ParentList->baseUri();
        } else {
            KMessageBox::error(m_ParentList,i18n("On remote browsing you must select an item!"));
            return;
        }
    }
    makeDiff(what,svn::Revision::HEAD,svn::Revision::BASE);
}

/*!
    \fn SvnActions::slotSimpleDiff()
 */
void SvnActions::slotSimpleDiff()
{
    if (!m_ParentList) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    QString what;
    if (!k) {
        what=m_ParentList->baseUri();
    }else{
        what=k->fullName();
    }
    makeDiff(what,svn::Revision::WORKING,svn::Revision::HEAD);
}


/*!
    \fn SvnActions::wroteStdin(KProcess*)
 */
void SvnActions::wroteStdin(KProcess*proc)
{
    if (!proc) return;
    proc->closeStdin();
}

void SvnActions::procClosed(KProcess*proc)
{
    if (!proc) return;
    delete proc;
}


/*!
    \fn SvnActions::makeDiff(const QString&,const svn::Revision&start,const svn::Revision&end)
 */
void SvnActions::makeDiff(const QString&what,const svn::Revision&start,const svn::Revision&end)
{
    QString ex;
    KTempFile tfile;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Diffing","Diffing - hit cancel for abort");
        ex = m_Svnclient.diff(svn::Path(tfile.name().local8Bit()),
            svn::Path(what.local8Bit()),
            start, end,
            true,false,false);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
#if 1
    /// @todo make it setable
    KProcess *proc = new KProcess();
    *proc << "kompare";
    *proc << "-";
    connect(proc,SIGNAL(wroteStdin(KProcess*)),this,SLOT(wroteStdin(KProcess*)));
    connect(proc,SIGNAL(processExited(KProcess*)),this,SLOT(procClosed(KProcess*)));
    if (proc->start(KProcess::NotifyOnExit,KProcess::Stdin)) {
        proc->writeStdin(ex.ascii(),ex.length());
        return;
    }
    delete proc;
#endif
    KTextBrowser*ptr;
    QDialog*dlg = createDialog(&ptr,QString(i18n("Diff display")));
    if (dlg) {
        ptr->setText(ex);
        dlg->exec();
        delete dlg;
    }
}


/*!
    \fn SvnActions::makeUpdate(const QString&what,const svn::Revision&rev,bool recurse)
 */
void SvnActions::makeUpdate(const QString&what,const svn::Revision&rev,bool recurse)
{
    QString ex;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Making update","Making update - hit cancel for abort");
        ex = m_Svnclient.update(svn::Path(what.local8Bit()),
            rev, recurse);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
}

/*!
    \fn SvnActions::slotUpdateHeadRec()
 */
void SvnActions::slotUpdateHeadRec()
{
    prepareUpdate(false);
}


/*!
    \fn SvnActions::prepareUpdate(bool ask)
 */
void SvnActions::prepareUpdate(bool ask)
{
    if (!m_ParentList||!m_ParentList->isLocal()) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    QString what;
    if (!k) {
        if (m_ParentList->allSelected().count()>0) {
            KMessageBox::error(m_ParentList,i18n("Cannot make multiple updates"));
            return;
        } else {
            what=m_ParentList->baseUri();
            k=static_cast<FileListViewItem*>(m_ParentList->firstChild());
        }
    } else {
        what=k->fullName();
    }
    svn::Revision r(svn::Revision::HEAD);
    if (ask) {
        Rangeinput_impl*rdlg;
        QDialog*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true);
        if (!dlg) {
            return;
        }
        rdlg->setStartOnly(true);
        /* just here cause layout has changed meanwhile */
        dlg->resize( QSize(120,60).expandedTo(dlg->minimumSizeHint()) );
        int result;
        if ((result=dlg->exec())==QDialog::Accepted) {
            Rangeinput_impl::revision_range range = rdlg->getRange();
            r=range.first;
        }
        delete dlg;
        if (result!=QDialog::Accepted) return;
    }
    makeUpdate(what,r,true);
    if (k) {
        k->removeChilds();
        m_ParentList->checkDirs(what,k);
        k->refreshMe();
        k->refreshStatus();
    }
}


/*!
    \fn SvnActions::slotUpdateTo()
 */
void SvnActions::slotUpdateTo()
{
    prepareUpdate(true);
}


/*!
    \fn SvnActions::slotAdd()
 */
void SvnActions::slotAdd()
{
    if (!m_ParentList) return;
    QPtrList<FileListViewItem> lst = m_ParentList->allSelected();
    if (lst.count()==0) {
        KMessageBox::error(m_ParentList,i18n("Which files or directories should I add?"));
        return;
    }
    QValueList<svn::Path> items;
    for (unsigned j = 0; j<lst.count();++j){
        if (lst.at(j)->svnStatus().isVersioned()) {
            KMessageBox::error(m_ParentList,i18n("<center>The entry<br>%1<br>is versioned - break.</center>")
                .arg(lst.at(j)->svnStatus().path()));
            return;
        }
        items.push_back(lst.at(j)->svnStatus().path());
    }
    QString ex;
    try {
        QValueList<svn::Path>::iterator piter;
        for (piter=items.begin();piter!=items.end();++piter) {
            m_Svnclient.add((*piter),false);
        }
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    for (unsigned j = 0; j<lst.count();++j){
        lst.at(j)->refreshStatus();
    }
}

/*!
    \fn SvnActions::slotDelete()
 */
void SvnActions::slotDelete()
{
    if (!m_ParentList) return;
    QPtrList<FileListViewItem> lst = m_ParentList->allSelected();
    if (lst.count()==0) {
        KMessageBox::error(m_ParentList,i18n("Nothing selected for delete"));
        return;
    }
    std::vector<svn::Path> items;
    QStringList displist;
        for (unsigned j = 0; j<lst.count();++j){
        if (!lst.at(j)->svnStatus().isVersioned()) {
            KMessageBox::error(m_ParentList,i18n("<center>The entry<br>%1<br>is not versioned - break.</center>")
                .arg(lst.at(j)->svnStatus().path()));
            return;
        }
        items.push_back(lst.at(j)->svnStatus().path());
        displist.append(lst.at(j)->svnStatus().path());
    }
    int answer = KMessageBox::questionYesNoList(m_ParentList,i18n("Realy delete that entries?"),displist,"Delete from repository");
    if (answer!=KMessageBox::Yes) {
        return;
    }
    QString ex;
    try {
        svn::Targets target(items);
        m_Svnclient.remove(target,false);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    for (unsigned j = 0; j<lst.count();++j){
        lst.at(j)->refreshStatus();
    }
}

/*!
    \fn kdesvnfilelist::slotCheckout()
 */
void SvnActions::slotCheckout()
{
    CheckoutInfo_impl*ptr;
    KDialog * dlg = createDialog(&ptr,i18n("Checkout a repository"),true);
    if (dlg) {
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt());
        }
        delete dlg;
    }
}

void SvnActions::slotCheckoutCurrent()
{
    if (!m_ParentList||m_ParentList->isLocal()) return;
    FileListViewItem*k = m_ParentList->singleSelected();
    if (k && !k->isDir()) {
        KMessageBox::error(m_ParentList,i18n("Checking out a file?"));
        return;
    }
    QString what;
    if (!k) {
        what = m_ParentList->baseUri();
    } else {
        what = k->fullName();
    }
    CheckoutInfo_impl*ptr;
    KDialog * dlg = createDialog(&ptr,"Checkout a repository",true);
    if (dlg) {
        ptr->setStartUrl(what);
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt());
        }
        delete dlg;
    }
}

void SvnActions::makeCheckout(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool force)
{
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath.local8Bit());
    reInitClient();
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Checkout"),i18n("Checking out - hit cancel for abort"));
        m_Svnclient.checkout(fUrl.local8Bit(),p,r,force);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
        return;
    }
    m_ParentList->openURL(tPath,true);
}

void SvnActions::slotRevert()
{
    qDebug("slotRevert");
    if (!m_ParentList||!m_ParentList->isLocal()) return;
    QPtrList<FileListViewItem> lst = m_ParentList->allSelected();
    qDebug("Selected items");
    QStringList displist;
    if (lst.count()>0) {
        for (unsigned j = 0; j<lst.count();++j){
            if (!lst.at(j)->svnStatus().isVersioned()) {
                KMessageBox::error(m_ParentList,i18n("<center>The entry<br>%1<br>is not versioned - break.</center>")
                    .arg(lst.at(j)->svnStatus().path()));
                return;
            }
            displist.append(lst.at(j)->svnStatus().path());
        }
    } else {
        displist.push_back(m_ParentList->baseUri());
    }
    slotRevertItems(displist);
    for (unsigned j = 0; j<lst.count();++j){
        lst.at(j)->refreshStatus(true,&lst,false);
        lst.at(j)->refreshStatus(false,&lst,true);
    }
}

void SvnActions::slotRevertItems(const QStringList&displist)
{
    if (displist.count()==0) {
        return;
    }
    KDialogBase*dialog = new KDialogBase(
                i18n("Revert entries"),
                KDialogBase::Yes | KDialogBase::No,
                KDialogBase::No, KDialogBase::No,
                m_ParentList,"warningRevert",true,true);

    bool checkboxres = false;

    int result = KMessageBox::createKMessageBox(dialog,QMessageBox::Warning,
        i18n("Realy revert that entries to pristine state?"), displist, i18n("Recursive"),
        &checkboxres,
        KMessageBox::Dangerous);
    if (result != KDialogBase::Yes) {
        return;
    }

    std::vector<svn::Path> items;
    for (unsigned j = 0; j<displist.count();++j) {
        items.push_back( (*(displist.at(j))).latin1());
    }
    QString ex;

    try {
        svn::Targets target(items);
        m_Svnclient.revert(target,checkboxres);
    } catch (svn::ClientException e) {
        ex = QString::fromLocal8Bit(e.message());
        emit clientException(ex);
    }
}
