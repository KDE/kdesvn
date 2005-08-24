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
#include "itemdisplay.h"
#include "svnitem.h"
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
#include "helpers/sub2qt.h"
#include "helpers/stl2qt.h"
#include "kdesvn_part.h"

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
#include <ktempdir.h>
#include <kdialogbase.h>
#include <qvbox.h>
#include <kapplication.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kdebug.h>
#include <qstylesheet.h>
#include <kconfig.h>
#include <qregexp.h>

#if 0
#include <khtml_part.h>
#include <khtmlview.h>
#endif

#define EMIT_FINISHED emit sendNotify(i18n("Finished"))
#define EMIT_REFRESH emit sigRefreshAll()
#define DIALOGS_SIZES "display_dialogs_sizes"

SvnActions::SvnActions(ItemDisplay *parent, const char *name)
 : QObject(parent->realWidget(), name),m_ParentList(parent),m_SvnContext(new CContextListener(this)),m_CurrentContext(0)
{
    connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
}

SvnActions::~SvnActions()
{
    delete m_CurrentContext;
    m_CurrentContext = 0;
}

void SvnActions::slotNotifyMessage(const QString&aMsg)
{
    emit sendNotify(aMsg);
}

void SvnActions::reInitClient()
{
    delete m_CurrentContext;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(m_SvnContext);
    m_Svnclient.setContext(m_CurrentContext);
}

#include "svnactions.moc"

template<class T> KDialogBase* SvnActions::createDialog(T**ptr,const QString&_head,bool OkCancel,const char*name)
{
    KDialogBase * dlg = new KDialogBase(
        0,
        name,
        true,
        _head,
        (OkCancel?KDialogBase::Ok|KDialogBase::Cancel:KDialogBase::Ok)/*,
        (OkCancel?KDialogBase::Cancel:KDialogBase::Close),
        KDialogBase::Cancel,
        true*//*,(OkCancel?KStdGuiItem::ok():KStdGuiItem::close())*/);

    if (!dlg) return dlg;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    *ptr = new T(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(kdesvnPart::config()),name?name:DIALOGS_SIZES));
    return dlg;
}


/*!
    \fn SvnActions::slotMakeRangeLog()
 */
void SvnActions::slotMakeRangeLog()
{
    /// @todo remove reference to parentlist
    if (!m_ParentList) return;
    SvnItem*k = m_ParentList->Selected();
    if (!k) return;
    Rangeinput_impl*rdlg;
    KDialogBase*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true,"revisions_dlg");
    if (!dlg) {
        return;
    }
    int i = dlg->exec();
    if (i==QDialog::Accepted) {
        Rangeinput_impl::revision_range r = rdlg->getRange();
        makeLog(r.first,r.second,k);
    }
    dlg->saveDialogSize(*(kdesvnPart::config()),"revisions_dlg",false);
}

/*!
    \fn SvnActions::slotMakeLog()
 */
void SvnActions::slotMakeLog()
{
    /// @todo remove reference to parentlist
    if (!m_ParentList) return;
    SvnItem*k = m_ParentList->Selected();
    if (!k) return;
    svn::Revision start(svn::Revision::START);
    svn::Revision end(svn::Revision::HEAD);
    makeLog(start,end,k);
}

/*!
    \fn SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
 */
void SvnActions::makeLog(svn::Revision start,svn::Revision end,SvnItem*k)
{
    const svn::LogEntries * logs;
    QString ex;
    if (!m_CurrentContext) return;
    KConfigGroup cs(kdesvnPart::config(), "general_items");
    bool follow = cs.readBoolEntry("toggle_log_follows",true);

    try {
        StopDlg sdlg(m_SvnContext,0,0,"Logs","Getting logs - hit cancel for abort");
        /// @todo second last parameter user settable (printing infos about copy/move etc) moment false so traffic reduced
        logs = m_Svnclient.log(k->fullName(),start,end,false,!follow);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
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
    disp.saveSize();
    delete logs;
    EMIT_FINISHED;
}

/*!
    \fn SvnActions::makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k)
 */
void SvnActions::makeBlame(svn::Revision start, svn::Revision end, SvnItem*k)
{
    if (!m_CurrentContext) return;
    svn::AnnotatedFile * blame;
    QString ex;
    svn::Path p(k->fullName());

    try {
        StopDlg sdlg(m_SvnContext,0,0,"Annotate","Blaming - hit cancel for abort");
        blame = m_Svnclient.annotate(p,start,end);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
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
    EMIT_FINISHED;
    svn::AnnotatedFile::const_iterator bit;
    static QString rowb = "<tr><td>";
    static QString rowc = "<tr bgcolor=\"#EEEEEE\"><td>";
    static QString rows = "</td><td>";
    static QString rowe = "</td></tr>\n";
    QString text = "<html><table>"+rowb+"Rev"+rows+i18n("Author")+rows+i18n("Line")+rows+"&nbsp;"+rowe;
    bool second = false;
    QString codetext = "";
    for (
        bit=blame->begin();
        bit!=blame->end();
        ++bit) {
        codetext = (*bit).line();
        codetext.replace(" ","&nbsp;");
        codetext.replace("\"","&quot;");
        codetext.replace("<","&lt;");
        codetext.replace(">","&gt;");
        codetext.replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
        text+=(second?rowb:rowc)+QString("%1").arg((*bit).revision())+
            rows+((*bit).author().length()?(*bit).author():i18n("Unknown"))+
            rows+QString("%1").arg((*bit).lineNumber())+rows+
            QString("<code>%1</code>").arg(codetext)+rowe;
            second = !second;
    }
    text += "</table></html>";
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Blame %1")).arg(k->shortName()),false,"blame_dlg");
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        dlg->saveDialogSize(*(kdesvnPart::config()),"blame_dlg",false);
        delete dlg;
    }
    delete blame;
}

void SvnActions::makeCat(svn::Revision start, const QString&what, const QString&disp)
{
    if (!m_CurrentContext) return;
    QString content;
    QString ex;
    svn::Path p(what);
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Content cat","Getting content - hit cancel for abort");
        content = m_Svnclient.cat(p,start);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    } catch (...) {
        ex = i18n("Error getting content");
        emit clientException(ex);
        return;
    }
    if (content.length()==0) {
        emit clientException(i18n("Got no content"));
        return;
    }
    EMIT_FINISHED;
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Content of %1")).arg(disp),false,"cat_display_dlg");
    if (dlg) {
        ptr->setFont(KGlobalSettings::fixedFont());
        ptr->setWordWrap(QTextEdit::NoWrap);
        ptr->setText("<code>"+QStyleSheet::convertFromPlainText(content)+"</code>");
        dlg->exec();
        dlg->saveDialogSize(*(kdesvnPart::config()),"cat_display_dlg",false);
        delete dlg;
    }
}

QString SvnActions::makeMkdir(const QString&parentDir)
{
    if (!m_CurrentContext) return QString::null;
    QString ex;
    bool isOk=false;
    ex = KInputDialog::getText(i18n("New folder"),i18n("Enter folder name:"),QString::null,&isOk);
    if (!isOk) {
        return QString::null;
    }
    svn::Path target(parentDir);
    target.addComponent(ex);
    ex = "";

    QString logMessage="";
    if (!m_ParentList->isLocal()) {
        bool ok;
        logMessage = Logmsg_impl::getLogmessage(&ok,0,m_ParentList->realWidget(),"logmsg_impl");
        if (!ok) {
            return QString::null;
        }
    }

    try {
        m_Svnclient.mkdir(target,logMessage);
    }catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return QString::null;
    }

    ex = target.path();
    return ex;
}

void SvnActions::slotInfo()
{
    /// @todo remove reference to parentlist
    if (!m_CurrentContext) return;
    if (!m_ParentList) return;
    QPtrList<SvnItem> lst;
    m_ParentList->SelectionList(&lst);
    svn::InfoEntries entries;
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    if (!m_ParentList->isLocal()) {
        rev = svn::Revision::HEAD;
    }
    QString ex;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Details","Retrieving infos - hit cancel for abort");
        if (lst.count()==0) {
            entries = m_Svnclient.info2(m_ParentList->baseUri(),true,rev,peg);
        } else {
            SvnItem*item;
            svn::InfoEntries e;
            for (item=lst.first();item;item=lst.next()) {
                e = (m_Svnclient.info2(item->fullName(),true,rev,peg));
                // stl like - hold it for qt4?
                //entries.insert(entries.end(),e.begin(),e.end());
                entries+=e;
            }
        }
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
    QString text = "<html>";
    svn::InfoEntries::const_iterator it;
    static QString rb = "<tr><td align=\"right\"><b>";
    static QString re = "</td></tr>\n";
    static QString cs = "</b>:</td><td>";
    for (it=entries.begin();it!=entries.end();++it) {
        text+="<p><table>";
        if ((*it).Name().length()) {
            text+=rb+i18n("Name")+cs+((*it).Name())+re;
        }
        text+=rb+i18n("URL")+cs+((*it).url())+re;
        if ((*it).reposRoot().length()) {
            text+=rb+i18n("Canonical repository url")+cs+((*it).reposRoot())+re;
        }
        if ((*it).checksum().length()) {
            text+=rb+i18n("Checksum")+cs+((*it).checksum())+re;
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
            text+=i18n("Folder");
            break;
        case svn_node_unknown:
        default:
            text+=i18n("Unknown");
            break;
        }
        text+=re;
        text+=rb+i18n("Schedule")+cs;
        switch ((*it).Schedule()) {
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
            text+=i18n("Unknown");
            break;
        }
        text+=re;
        text+=rb+i18n("UUID")+cs+((*it).uuid())+re;
        text+=rb+i18n("Last author")+cs+((*it).cmtAuthor())+re;
        text+=rb+i18n("Last committed")+cs+helpers::sub2qt::apr_time2qt((*it).cmtDate()).toString(Qt::LocalDate)+re;
        text+=rb+i18n("Last revision")+cs+QString("%1").arg((*it).cmtRev())+re;
        text+=rb+i18n("Content last changed")+cs+helpers::sub2qt::apr_time2qt((*it).textTime()).toString(Qt::LocalDate)+re;
        text+=rb+i18n("Property last changed")+cs+helpers::sub2qt::apr_time2qt((*it).propTime()).toString(Qt::LocalDate)+re;
        if ((*it).conflictNew().length()) {
            text+=rb+i18n("New version of conflicted file")+cs+((*it).conflictNew())+re;
        }
        if ((*it).conflictOld().length()) {
            text+=rb+i18n("Old version of conflicted file")+cs+((*it).conflictOld())+re;
        }
        if ((*it).conflictWrk().length()) {
            text+=rb+i18n("Working version of conflicted file")+
                cs+((*it).conflictWrk())+re;
        }
        if ((*it).prejfile().length()) {
            text+=rb+i18n("Property reject file")+
                cs+((*it).prejfile())+re;
        }

        if ((*it).copyfromUrl().length()) {
            text+=rb+i18n("Copy from URL")+cs+((*it).copyfromUrl())+re;
        }
        if ((*it).lockEntry().Locked()) {
            text+=rb+i18n("Lock token")+cs+((*it).lockEntry().Token())+re;
            text+=rb+i18n("Owner")+cs+((*it).lockEntry().Owner())+re;
            text+=rb+i18n("Locked on")+cs+
                helpers::sub2qt::apr_time2qt((*it).lockEntry().Date()).toString(Qt::LocalDate)+
                re;
            text+=rb+i18n("Lock comment")+cs+
                (*it).lockEntry().Comment()+re;
        }
        text+="</table></p>\n";
    }
    text+="</html>";
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Infolist")),false,"info_dialog");
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        dlg->saveDialogSize(*(kdesvnPart::config()),"info_dialog",false);
        delete dlg;
    }
}


/*!
    \fn SvnActions::slotProperties()
 */
void SvnActions::slotProperties()
{
    /// @todo remove reference to parentlist
    if (!m_CurrentContext) return;
    if (!m_ParentList) return;
    SvnItem*k = m_ParentList->Selected();
    if (!k) return;
    PropertiesDlg dlg(k->fullName(),svnclient(),
        m_ParentList->isLocal()?svn::Revision::WORKING:svn::Revision::HEAD);
    connect(&dlg,SIGNAL(clientException(const QString&)),m_ParentList->realWidget(),SLOT(slotClientException(const QString&)));
    dlg.resize(dlg.configDialogSize("kdesvn_properties_dlg"));
    if (dlg.exec()!=QDialog::Accepted) {
        return;
    }
    dlg.saveDialogSize(*(kdesvnPart::config()),"kdesvn_properties_dlg",false);
    QString ex;
    PropertiesDlg::tPropEntries setList;
    QValueList<QString> delList;
    dlg.changedItems(setList,delList);
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Applying properties","<center>Applying<br>hit cancel for abort</center>");
        unsigned int pos;
        for (pos = 0; pos<delList.size();++pos) {
            m_Svnclient.propdel(delList[pos],svn::Path(k->fullName()),svn::Revision::HEAD);
        }
        PropertiesDlg::tPropEntries::Iterator it;
        for (it=setList.begin(); it!=setList.end();++it) {
            m_Svnclient.propset(it.key(),it.data(),svn::Path(k->fullName()),svn::Revision::HEAD);
        }
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    k->refreshStatus();
    EMIT_FINISHED;
}


/*!
    \fn SvnActions::slotCommit()
 */
void SvnActions::slotCommit()
{
    /// @todo remove reference to parentlist
    if (!m_CurrentContext) return;
    if (!m_ParentList->isLocal()) return;
    bool ok,rec;
    QString msg = Logmsg_impl::getLogmessage(&ok,&rec,m_ParentList->realWidget(),"logmsg_impl");
    if (!ok) {
        return;
    }
    QPtrList<SvnItem> which;
    m_ParentList->SelectionList(&which);
    SvnItem*cur;
    QPtrListIterator<SvnItem> liter(which);

    QValueList<svn::Path> targets;
    kdDebug()<<"Commit baseuri" << m_ParentList->baseUri()<<endl;
    if (which.count()==0) {
        targets.push_back(svn::Path(m_ParentList->baseUri()));
    } else {
        while ( (cur=liter.current())!=0) {
            ++liter;
            kdDebug()<<"Commiting " << cur->fullName()<<endl;
            targets.push_back(svn::Path(cur->fullName()));
        }
    }
    svn_revnum_t nnum;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Commiting","Commiting - hit cancel for abort");
        nnum = m_Svnclient.commit(svn::Targets(targets),msg,rec);
    } catch (svn::ClientException e) {
        QString ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_REFRESH;
    EMIT_FINISHED;
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
    if (!m_CurrentContext) return;
    QString ex;
    KTempDir tdir;
    tdir.setAutoDelete(true);
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Diffing","Diffing - hit cancel for abort");
        ex = m_Svnclient.diff(svn::Path(tdir.name()),
            svn::Path(what),
            start, end,
            true,false,false);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
    KConfigGroup cs(kdesvnPart::config(), "general_items");
    if (cs.readBoolEntry("use_kompare_for_diff",true)) {
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
    }
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Diff display")),false,"diff_display");
    if (dlg) {
        ptr->setText("<code>"+QStyleSheet::convertFromPlainText(ex)+"</code>");
        dlg->exec();
        dlg->saveDialogSize(*(kdesvnPart::config()),"diff_display",false);
        delete dlg;
    }
}


/*!
    \fn SvnActions::makeUpdate(const QString&what,const svn::Revision&rev,bool recurse)
 */
void SvnActions::makeUpdate(const QStringList&what,const svn::Revision&rev,bool recurse)
{
    if (!m_CurrentContext) return;
    QString ex;
    svn_revnum_t ret;
    try {
        StopDlg sdlg(m_SvnContext,0,0,"Making update","Making update - hit cancel for abort");
        for (unsigned int i = 0; i<what.count();++i) {
            ret = m_Svnclient.update(svn::Path(what[i]),
                rev, recurse);
            kapp->processEvents();
            if (sdlg.cancelld()) {
                break;
            }
        }
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_REFRESH;
    EMIT_FINISHED;
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
    SvnItemList k;
    m_ParentList->SelectionList(&k);

    QStringList what;
    if (k.count()==0) {
        what.append(m_ParentList->baseUri());
    } else {
        SvnItemListIterator liter(k);
        SvnItem*cur;
        while ((cur=liter.current())!=0){
            ++liter;
            what.append(cur->fullName());
        }
    }
    svn::Revision r(svn::Revision::HEAD);
    if (ask) {
        Rangeinput_impl*rdlg;
        KDialog*dlg = createDialog(&rdlg,QString(i18n("Revisions")),true);
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
    if (!m_CurrentContext) return;
    if (!m_ParentList) return;
    QPtrList<SvnItem> lst;
    m_ParentList->SelectionList(&lst);
    if (lst.count()==0) {
        KMessageBox::error(m_ParentList->realWidget(),i18n("Which files or directories should I add?"));
        return;
    }
    QValueList<svn::Path> items;
    SvnItemListIterator liter(lst);
    SvnItem*cur;
    while ((cur=liter.current())!=0){
        ++liter;
        if (cur->isVersioned()) {
            KMessageBox::error(m_ParentList->realWidget(),i18n("<center>The entry<br>%1<br>is versioned - break.</center>")
                .arg(cur->fullName()));
            return;
        }
        items.push_back(svn::Path(cur->fullName()));
    }
    addItems(items);
    liter.toFirst();
    while ((cur=liter.current())!=0){
        ++liter;
        cur->refreshStatus();
        //emit sigRefreshCurrent(static_cast<FileListViewItem*>(cur->parent()));
    }
}

void SvnActions::addItems(const QValueList<svn::Path> &items,bool rec)
{
    QString ex;
    try {
        QValueList<svn::Path>::const_iterator piter;
        for (piter=items.begin();piter!=items.end();++piter) {
            m_Svnclient.add((*piter),rec);
        }
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
}

/*!
    \fn SvnActions::makeDelete()
 */
void SvnActions::makeDelete(const QValueList<svn::Path>&items)
{
    if (!m_CurrentContext) return;
    QString ex;
    try {
        svn::Targets target(items);
        m_Svnclient.remove(target,false);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
}

/*!
    \fn kdesvnfilelist::slotCheckout()
 */
void SvnActions::CheckoutExport(bool _exp)
{
    CheckoutInfo_impl*ptr;
    KDialogBase * dlg = createDialog(&ptr,(_exp?i18n("Export repository"):i18n("Checkout a repository")),true,"checkout_export_dialog");
    if (dlg) {
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            bool openit = ptr->openAfterJob();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt(),_exp,openit);
        }
        dlg->saveDialogSize(*(kdesvnPart::config()),"checkout_export_dialog",false);
        delete dlg;
    }
}

void SvnActions::slotCheckout()
{
    CheckoutExport(false);
}

void SvnActions::slotExport()
{
    CheckoutExport(true);
}

void SvnActions::CheckoutExportCurrent(bool _exp)
{
    if (!m_ParentList|| !_exp&&m_ParentList->isLocal()) return;
    SvnItem*k = m_ParentList->Selected();
    if (k && !k->isDir()) {
        KMessageBox::error(m_ParentList->realWidget(),_exp?i18n("Exporting a file?"):i18n("Checking out a file?"));
        return;
    }
    QString what;
    if (!k) {
        what = m_ParentList->baseUri();
    } else {
        what = k->fullName();
    }
    CheckoutInfo_impl*ptr;
    KDialog * dlg = createDialog(&ptr,_exp?i18n("Export a repository"):i18n("Checkout a repository"),true);
    if (dlg) {
        ptr->setStartUrl(what);
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            bool openIt = ptr->openAfterJob();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt(),_exp,openIt);
        }
        delete dlg;
    }
}

void SvnActions::slotCheckoutCurrent()
{
    CheckoutExportCurrent(false);
}

void SvnActions::slotExportCurrent()
{
    CheckoutExportCurrent(true);
}

void SvnActions::makeCheckout(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool force,bool _exp,bool openIt)
{
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath);
    if (!_exp||!m_CurrentContext) reInitClient();
    try {
        StopDlg sdlg(m_SvnContext,0,0,_exp?i18n("Export"):i18n("Checkout"),_exp?i18n("Exporting"):i18n("Checking out"));
        if (_exp) {
            m_Svnclient.doExport(svn::Path(fUrl),p,r,force);
        } else {
            m_Svnclient.checkout(fUrl,p,r,force);
        }
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    if (openIt) {
        if (!_exp) m_ParentList->openURL(tPath,true);
        else kapp->invokeBrowser(tPath);
    }
    EMIT_FINISHED;
}

void SvnActions::slotRevert()
{
    if (!m_ParentList||!m_ParentList->isLocal()) return;
    QPtrList<SvnItem> lst;
    m_ParentList->SelectionList(&lst);
    QStringList displist;
    SvnItemListIterator liter(lst);
    SvnItem*cur;
    if (lst.count()>0) {
        while ((cur=liter.current())!=0){
            if (!cur->isVersioned()) {
                KMessageBox::error(m_ParentList->realWidget(),i18n("<center>The entry<br>%1<br>is not versioned - break.</center>")
                    .arg(cur->fullName()));
                return;
            }
            displist.append(cur->fullName());
            ++liter;
        }
    } else {
        displist.push_back(m_ParentList->baseUri());
    }
    slotRevertItems(displist);
    liter.toFirst();
    while ((cur=liter.current())!=0){
        ++liter;
        cur->refreshStatus(true,&lst,false);
        cur->refreshStatus(false,&lst,true);
    }
}

void SvnActions::slotRevertItems(const QStringList&displist)
{
    if (!m_CurrentContext) return;
    if (displist.count()==0) {
        return;
    }
    KDialogBase*dialog = new KDialogBase(
                i18n("Revert entries"),
                KDialogBase::Yes | KDialogBase::No,
                KDialogBase::No, KDialogBase::No,
                m_ParentList->realWidget(),"warningRevert",true,true);

    bool checkboxres = false;

    int result = KMessageBox::createKMessageBox(dialog,QMessageBox::Warning,
        i18n("Really revert these entries to pristine state?"), displist, i18n("Recursive"),
        &checkboxres,
        KMessageBox::Dangerous);
    if (result != KDialogBase::Yes) {
        return;
    }

    QValueList<svn::Path> items;
    for (unsigned j = 0; j<displist.count();++j) {
        items.push_back(svn::Path((*(displist.at(j)))));
    }
    QString ex;

    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Revert"),i18n("Reverting items"));
        svn::Targets target(items);
        m_Svnclient.revert(target,checkboxres);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
}

void SvnActions::makeSwitch(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool rec)
{
    if (!m_CurrentContext) return;
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath);
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Switch url"),i18n("Switching url"));
        m_Svnclient.doSwitch(p,fUrl,r,rec);
    } catch (svn::ClientException e) {
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
}

void SvnActions::slotSwitch()
{
    if (!m_CurrentContext) return;
    if (!m_ParentList||!m_ParentList->isLocal()) return;

    QPtrList<SvnItem> lst;
    m_ParentList->SelectionList(&lst);

    if (lst.count()>1) {
        KMessageBox::error(0,i18n("Can only switch one item at time"));
        return;
    }
    SvnItem*k;

    k = m_ParentList->SelectedOrMain();
#if 0
    if (lst->count()==0) {
        k=static_cast<FileListViewItem*>(m_ParentList->firstChild());
    } else {
        k = lst->at(0);
    }
#endif
    if (!k) {
        KMessageBox::error(0,i18n("Error getting entry to switch"));
        return;
    }
    QString path,what;
    path = k->fullName();
    what = k->Url();

    CheckoutInfo_impl*ptr;
    KDialog * dlg = createDialog(&ptr,i18n("Switch url"),true);
    if (dlg) {
        ptr->setStartUrl(what);
        ptr->forceAsRecursive(true);
        ptr->disableTargetDir(true);
        bool done = false;
        if (dlg->exec()==QDialog::Accepted) {
            done = true;
            svn::Revision r = ptr->toRevision();
            makeSwitch(ptr->reposURL(),path,r,ptr->forceIt());
        }
        delete dlg;
        if (!done) return;
    }
    k->refreshMe();
//    emit reinitItem(k);
}

void SvnActions::slotCleanup(const QString&path)
{
    if (!m_CurrentContext) return;
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Cleanup"),i18n("Cleaning up folder"));
        m_Svnclient.cleanup(svn::Path(path));
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}

void SvnActions::slotResolved(const QString&path)
{
    if (!m_CurrentContext) return;
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Resolve"),i18n("Marking resolved"));
        m_Svnclient.resolved(svn::Path(path),true);
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}

void SvnActions::slotImport(const QString&path,const QString&target,const QString&message,bool rec)
{
    if (!m_CurrentContext) return;
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Import"),i18n("Importing items"));
        m_Svnclient.import(svn::Path(path),target,message,rec);
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}

void SvnActions::slotMergeWcRevisions(const QString&_entry,const svn::Revision&rev1,
                    const svn::Revision&rev2,bool rec,bool ancestry,bool forceIt,bool dry)
{
    if (!m_CurrentContext) return;
    try {
        m_Svnclient.merge(svn::Path(_entry),
            rev1,
            svn::Path(_entry),
            rev2,
            svn::Path(_entry),
            forceIt,rec,ancestry,dry);
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}

/*!
    \fn SvnActions::slotCopyMove(bool,const QString&,const QString&)
 */
void SvnActions::slotCopyMove(bool move,const QString&Old,const QString&New,bool force)
{
    if (!m_CurrentContext) return;
    try {
        StopDlg sdlg(m_SvnContext,0,0,i18n("Copy / Move"),i18n("Copy or Moving entries"));
        if (move) {
            m_Svnclient.move(svn::Path(Old),svn::Revision::HEAD,
                svn::Path(New),force);
        } else {
            m_Svnclient.copy(svn::Path(Old),svn::Revision::HEAD,
                svn::Path(New));
        }
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
    EMIT_REFRESH;
}


/*!
    \fn SvnActions::makeLock(const QStringList&)
 */
void SvnActions::makeLock(const QStringList&what,const QString&_msg,bool breakit)
{
    QValueList<svn::Path> targets;
    for (unsigned int i = 0; i<what.count();++i) {
        targets.push_back(svn::Path((*(what.at(i)))));
    }
    if (!m_CurrentContext) return;
    try {
        m_Svnclient.lock(svn::Targets(targets),_msg,breakit);
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}


/*!
    \fn SvnActions::makeUnlock(const QStringList&)
 */
void SvnActions::makeUnlock(const QStringList&what,bool breakit)
{
    QValueList<svn::Path> targets;
    if (!m_CurrentContext) return;
    for (unsigned int i = 0; i<what.count();++i) {
        targets.push_back(svn::Path((*(what.at(i)))));
    }

    try {
        m_Svnclient.unlock(svn::Targets(targets),breakit);
    } catch (svn::ClientException e) {
        emit clientException(QString::fromUtf8(e.message()));
        return;
    }
}


/*!
    \fn SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist)
 */
bool SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist,bool rec,bool all)
{
    KConfigGroup cs(kdesvnPart::config(), "subversion");
    bool display_ignores = cs.readBoolEntry("display_ignored_files",true);
    //bool display_unknown = cs.readBoolEntry("display_unknown_files",true);

    QString ex;
    try {
        //                              rec all  up     noign
        dlist = m_Svnclient.status(what,rec,all,false,display_ignores);
    } catch (svn::ClientException e) {
        //Message box!
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return false;
    }
    return true;
}


/*!
    \fn SvnActions::makeIgnoreEntry(const QString&which)
 */
bool SvnActions::makeIgnoreEntry(SvnItem*which,bool unignore)
{
    if (!which) return false;
    QString parentName = which->getParentDir();
    if (parentName.isEmpty()) return false;
    QString name = which->shortName();
    QString ex;
    svn::Path p(parentName);
    svn::Revision r(svn_opt_revision_unspecified);
    svn::PathPropertiesMapList pm;
    try {
        pm = m_Svnclient.propget("svn:ignore",p,r);
    } catch (svn::ClientException e) {
        //Message box!
        ex = QString::fromUtf8(e.message());
        emit clientException(ex);
        return false;
    }
    QString data = "";
    if (pm.size()>0) {
        svn::PropertiesMap mp = pm[0].second;
        data = mp["svn:ignore"];
    }
    bool result = false;
    QRegExp reg("\\b"+QRegExp::escape(name)+"\\n");
    if (reg.search(data)!=-1) {
        if (unignore) {
            data.replace(reg,"");
            result = true;
        }
    } else {
        data = data.stripWhiteSpace();
        if (!unignore) {
            if (!data.isEmpty())
                data+="\n";
            data+=name;
            result = true;
        }
    }
    if (result) {

        try {
            m_Svnclient.propset("svn:ignore",data,p,r);
        } catch (svn::ClientException e) {
            //Message box!
            ex = QString::fromUtf8(e.message());
            emit clientException(ex);
            return false;
        }
    }
    return result;
}
