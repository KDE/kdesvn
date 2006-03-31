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
#include "svnactions.h"
#include "checkoutinfo_impl.h"
#include "itemdisplay.h"
#include "svnitem.h"
#include "rangeinput_impl.h"
#include "propertiesdlg.h"
#include "ccontextlistener.h"
#include "modifiedthread.h"
#include "svnlogdlgimp.h"
#include "stopdlg.h"
#include "logmsg_impl.h"
#include "graphtree/revisiontree.h"
#include "fronthelpers/settings.h"
#include "svnqt/client.hpp"
#include "svnqt/annotate_line.hpp"
#include "svnqt/context_listener.hpp"
#include "svnqt/dirent.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/url.hpp"
#include "svnqt/wc.hpp"
#include "helpers/sub2qt.h"
#include "svnfrontend/fronthelpers/oimagescrollview.h"
#include "cacheentry.h"

#include <kdialog.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kprocess.h>
#include <ktempdir.h>
#include <ktempfile.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klistview.h>

#include <qstring.h>
#include <qmap.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvaluelist.h>
#include <qvbox.h>
#include <qstylesheet.h>
#include <qregexp.h>
#include <qimage.h>
#include <qthread.h>
#include <qtimer.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <sys/time.h>
#include <unistd.h>


// wait not longer than 10 seconds for a thread
#define MAX_THREAD_WAITTIME 10000

class SvnActionsData:public ref_count
{
public:
    SvnActionsData():ref_count()
    {
        m_Svnclient = svn::Client::getobject(0,0);
        m_CurrentContext = 0;
//        m_Cache.clear();
    }

    virtual ~SvnActionsData()
    {
        delete m_CurrentContext;
        m_CurrentContext = 0;
        QMap<KProcess*,QString>::iterator it;
        for (it=m_tempfilelist.begin();it!=m_tempfilelist.end();++it) {
            ::unlink((*it).ascii());
        }
        delete m_Svnclient;
        m_Svnclient = 0L;
    }

    ItemDisplay* m_ParentList;

    smart_pointer<CContextListener> m_SvnContext;
    svn::Context* m_CurrentContext;
    svn::Client*m_Svnclient;

    helpers::itemCache m_UpdateCache;
    helpers::itemCache m_Cache;
    helpers::itemCache m_conflictCache;

    QMap<KProcess*,QString> m_tempfilelist;

    QTimer m_ThreadCheckTimer;
    QTimer m_UpdateCheckTimer;
    QTime m_UpdateCheckTick;
};

#define EMIT_FINISHED emit sendNotify(i18n("Finished"))
#define EMIT_REFRESH emit sigRefreshAll()
#define DIALOGS_SIZES "display_dialogs_sizes"

SvnActions::SvnActions(ItemDisplay *parent, const char *name)
 : QObject(parent?parent->realWidget():0, name)
{
    m_CThread = 0;
    m_UThread = 0;
    kdDebug()<<"New SvnActionsData()"<<endl;
    m_Data = new SvnActionsData();
    kdDebug()<<"New SvnActionsData() finished"<<endl;
    m_Data->m_ParentList = parent;
    m_Data->m_SvnContext = new CContextListener(this);
    connect(m_Data->m_SvnContext,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(&(m_Data->m_ThreadCheckTimer),SIGNAL(timeout()),this,SLOT(checkModthread()));
    connect(&(m_Data->m_UpdateCheckTimer),SIGNAL(timeout()),this,SLOT(checkUpdateThread()));
}

svn::Client* SvnActions::svnclient()
{
    return m_Data->m_Svnclient;
}

SvnActions::~SvnActions()
{
    killallThreads();
}

void SvnActions::slotNotifyMessage(const QString&aMsg)
{
    emit sendNotify(aMsg);
}

void SvnActions::reInitClient()
{
    delete m_Data->m_CurrentContext;
    m_Data->m_CurrentContext = new svn::Context();
    m_Data->m_CurrentContext->setListener(m_Data->m_SvnContext);
    m_Data->m_Svnclient->setContext(m_Data->m_CurrentContext);
}

template<class T> KDialogBase* SvnActions::createDialog(T**ptr,const QString&_head,bool OkCancel,const char*name)
{
    KDialogBase * dlg = new KDialogBase(
        KApplication::activeModalWidget(),
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
    dlg->resize(dlg->configDialogSize(*(Settings::self()->config()),name?name:DIALOGS_SIZES));
    return dlg;
}

/*!
    \fn SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
 */
void SvnActions::makeLog(svn::Revision start,svn::Revision end,SvnItem*k,bool list_files,int limit)
{
    if (!k)return;
    makeLog(start,end,k->fullName(),list_files,limit);
}

const svn::LogEntries * SvnActions::getLog(svn::Revision start,svn::Revision end,const QString&which,bool list_files,int limit)
{
    const svn::LogEntries * logs = 0;
    QString ex;
    if (!m_Data->m_CurrentContext) return 0;

    bool follow = Settings::log_follows_nodes();

    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Logs","Getting logs - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        logs = m_Data->m_Svnclient->log(which,start,end,list_files,!follow,limit);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return 0;
    }
    if (!logs) {
        ex = i18n("Got no logs");
        emit clientException(ex);
        return 0;
    }
    return logs;
}

void SvnActions::makeTree(const QString&what,const svn::Revision&_rev)
{
    QString url;
    QString ex;
    svn::Revision rev = _rev;
    svn::Revision peg;
    if (!m_Data->m_CurrentContext) return;

    if (!svn::Url::isValid(what)) {
        // working copy
        // url = svn::Wc::getUrl(what);
        url = what;
        peg = svn::Revision::UNDEFINED;
    } else {
        KURL _uri = what;
        QString prot = svn::Url::transformProtokoll(_uri.protocol());
        _uri.setProtocol(prot);
        url = _uri.prettyURL();
        peg = _rev;
    }
    kdDebug()<<"Getting info " << url << endl;
    svn::InfoEntries e;

    try {
        e = (m_Data->m_Svnclient->info(url,false,_rev,peg));
    } catch (svn::ClientException ce) {
        kdDebug()<<ce.msg() << endl;
        emit clientException(ce.msg());
        return;
    }
    if (e.count()<1) {
        kdDebug()<<"No info found" << endl;
        emit clientException(i18n("Got no info."));
        return;
    }
    QString reposRoot = e[0].reposRoot();

    kdDebug()<<"Logs for "<<reposRoot<<endl;
    QWidget*disp;
    KDialogBase dlg(m_Data->m_ParentList->realWidget(),"historylist",true,i18n("History of %1").arg(e[0].url().mid(reposRoot.length())),
        KDialogBase::Ok,
        KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    RevisionTree rt(m_Data->m_Svnclient,m_Data->m_SvnContext,reposRoot,
            e[0].url().mid(reposRoot.length()),rev,Dialog1Layout,m_Data->m_ParentList->realWidget());
    if (rt.isValid()) {
        disp = rt.getView();
        if (disp) {
            connect(disp,SIGNAL(dispDiff(const QString&)),this,SLOT(dispDiff(const QString&)));
            dlg.resize(dlg.configDialogSize(*(Settings::self()->config()),"revisiontree_dlg"));
            dlg.exec();
            dlg.saveDialogSize(*(Settings::self()->config()),"revisiontree_dlg",false);
        }
    }
}

void SvnActions::makeLog(svn::Revision start,svn::Revision end,const QString&which,bool list_files,int limit)
{
    const svn::LogEntries * logs = getLog(start,end,which,list_files,limit);
    if (!logs) return;
    SvnLogDlgImp disp(this);
    disp.dispLog(logs,which);
    connect(&disp,SIGNAL(makeDiff(const QString&,const svn::Revision&,const svn::Revision&)),
            this,SLOT(makeDiff(const QString&,const svn::Revision&,const svn::Revision&)));
    disp.exec();
    disp.saveSize();
    delete logs;
    EMIT_FINISHED;
}

void SvnActions::makeBlame(svn::Revision start, svn::Revision end, SvnItem*k)
{
    if (k) makeBlame(start,end,k->fullName());
}

void SvnActions::makeBlame(svn::Revision start, svn::Revision end, const QString&k)
{
    if (!m_Data->m_CurrentContext) return;
    svn::AnnotatedFile blame;
    QString ex;
    svn::Path p(k);

    /// @todo make parameter for peg revision
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Annotate","Blaming - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->annotate(blame,p,start,end);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    } catch (...) {
        ex = i18n("Error getting blame");
        emit clientException(ex);
        return;
    }
    if (blame.count()==0) {
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
    for (bit=blame.begin();bit!=blame.end();++bit) {
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
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Blame %1")).arg(k),false,"blame_dlg");
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        dlg->saveDialogSize(*(Settings::self()->config()),"blame_dlg",false);
        delete dlg;
    }
}

QByteArray SvnActions::makeGet(const svn::Revision&start, const QString&what,const svn::Revision&peg)
{
    QByteArray content;
    if (!m_Data->m_CurrentContext) return content;
    QString ex;
    svn::Path p(what);
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),
            0,"Content cat","Getting content - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        content = m_Data->m_Svnclient->cat(p,start,peg);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
    } catch (...) {
        ex = i18n("Error getting content");
        emit clientException(ex);
    }
    return content;
}

void SvnActions::makeCat(const svn::Revision&start, const QString&what, const QString&disp,const svn::Revision&peg)
{
    QByteArray content = makeGet(start,what,peg);
    if (content.size()==0) {
        emit clientException(i18n("Got no content"));
        return;
    }
    EMIT_FINISHED;
    // a bit fun must be ;)
    QImage img(content);
    if (img.isNull()) {
        KTextBrowser*ptr;
        KDialogBase*dlg = createDialog(&ptr,QString(i18n("Content of %1")).arg(disp),false,"cat_display_dlg");
        if (dlg) {
            ptr->setFont(KGlobalSettings::fixedFont());
            ptr->setWordWrap(QTextEdit::NoWrap);
            ptr->setText("<code>"+QStyleSheet::convertFromPlainText(content)+"</code>");
            dlg->exec();
            dlg->saveDialogSize(*(Settings::self()->config()),"cat_display_dlg",false);
            delete dlg;
        }
    } else {
        Opie::MM::OImageScrollView*ptr;
        KDialogBase*dlg = createDialog(&ptr,QString(i18n("Content of %1")).arg(disp),false,"cat_display_dlg");
        ptr->setAutoRotate(false);
        //ptr->setShowZoomer( true );
        ptr->setImage( img );
        dlg->exec();
        dlg->saveDialogSize(*(Settings::self()->config()),"cat_display_dlg",false);
        delete dlg;
    }
}

QString SvnActions::makeMkdir(const QString&parentDir)
{
    if (!m_Data->m_CurrentContext) return QString::null;
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
    if (!m_Data->m_ParentList->isWorkingCopy()) {
        bool ok;
        logMessage = Logmsg_impl::getLogmessage(&ok,0,m_Data->m_ParentList->realWidget(),"logmsg_impl");
        if (!ok) {
            return QString::null;
        }
    }

    try {
        m_Data->m_Svnclient->mkdir(target,logMessage);
    }catch (svn::ClientException e) {
        emit clientException(e.msg());
        return QString::null;
    }

    ex = target.path();
    return ex;
}

QString SvnActions::getInfo(QPtrList<SvnItem> lst,const svn::Revision&rev,const svn::Revision&peg,bool recursive,bool all)
{
    QStringList l;
    QString res = "";
    SvnItem*item;
    for (item=lst.first();item;item=lst.next()) {
        if (all) res+="<h4 align=\"center\">"+item->fullName()+"</h4>";
        res += getInfo(item->fullName(),rev,item->stat().entry().revision(),recursive,all);
    }
    return res;
}

QString SvnActions::getInfo(const QString& _what,const svn::Revision&rev,const svn::Revision&peg,bool recursive,bool all)
{
    if (!m_Data->m_CurrentContext) return QString::null;
    QString ex;
    svn::InfoEntries entries;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Details","Retrieving infos - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        svn::InfoEntries e;
        entries = (m_Data->m_Svnclient->info(_what,recursive,rev,peg));
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return QString::null;
    }
    //if (!all) EMIT_FINISHED;
    QString text = "";
    svn::InfoEntries::const_iterator it;
    static QString rb = "<tr><td><nobr><font color=\"black\">";
    static QString re = "</font></nobr></td></tr>\n";
    static QString cs = "</font></nobr>:</td><td><nobr><font color=\"black\">";
    unsigned int val = 0;
    for (it=entries.begin();it!=entries.end();++it) {
        if (val>0) {
            text+="<hline>";
        }
#if 0
        if (all && lst.count()>=val) {
            text+="<h4 align=\"center\">"+lst[val]+"</h4>";
            ++val;
        }
#endif
        text+="<p align=\"center\">";
        text+="<table cellspacing=0 cellpadding=0>";
        if ((*it).Name().length()) {
            text+=rb+i18n("Name")+cs+((*it).Name())+re;
        }
        if (all) {
            text+=rb+i18n("URL")+cs+((*it).url())+re;
            if ((*it).reposRoot().length()) {
                text+=rb+i18n("Canonical repository url")+cs+((*it).reposRoot())+re;
            }
            if ((*it).checksum().length()) {
                text+=rb+i18n("Checksum")+cs+((*it).checksum())+re;
            }
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
        if (all) {
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
        }
        text+=rb+i18n("Last author")+cs+((*it).cmtAuthor())+re;
        text+=rb+i18n("Last committed")+cs+helpers::sub2qt::apr_time2qtString((*it).cmtDate())+re;
        text+=rb+i18n("Last revision")+cs+QString("%1").arg((*it).cmtRev())+re;
        text+=rb+i18n("Content last changed")+cs+helpers::sub2qt::apr_time2qtString((*it).textTime())+re;
        if (all) {
            text+=rb+i18n("Property last changed")+cs+helpers::sub2qt::apr_time2qtString((*it).propTime())+re;
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
                    helpers::sub2qt::apr_time2qtString((*it).lockEntry().Date())+
                    re;
                text+=rb+i18n("Lock comment")+cs+
                    (*it).lockEntry().Comment()+re;
            }
        }
        text+="</table></p>\n";
    }
    return text;
}

void SvnActions::makeInfo(QPtrList<SvnItem> lst,const svn::Revision&rev,const svn::Revision&peg,bool recursive)
{
    QStringList l;
    QString res = "<html><head></head><body>";
    SvnItem*item;
    for (item=lst.first();item;item=lst.next()) {
        QString text = getInfo(item->fullName(),rev,item->stat().entry().revision(),recursive,true);
        if (!text.isEmpty()) {
            res+="<h4 align=\"center\">"+item->fullName()+"</h4>";
            res+=text;
        }
    }
    res+="</body></html>";
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Infolist")),false,"info_dialog");
    if (dlg) {
        ptr->setText(res);
        dlg->exec();
        dlg->saveDialogSize(*(Settings::self()->config()),"info_dialog",false);
        delete dlg;
    }
}

void SvnActions::makeInfo(const QStringList&lst,const svn::Revision&rev,const svn::Revision&peg,bool recursive)
{
    QString text = "";
    for (unsigned int i=0; i < lst.count();++i) {
        QString res = getInfo(lst[i],rev,peg,recursive,true);
        if (!res.isEmpty()) {
            text+="<h4 align=\"center\">"+lst[i]+"</h4>";
            text+=res;
        }
    }
    text = "<html><head></head><body>"+text+"</body></html>";
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Infolist")),false,"info_dialog");
    if (dlg) {
        ptr->setText(text);
        dlg->exec();
        dlg->saveDialogSize(*(Settings::self()->config()),"info_dialog",false);
        delete dlg;
    }
}


/*!
    \fn SvnActions::slotProperties()
 */
void SvnActions::slotProperties()
{
    /// @todo remove reference to parentlist
    if (!m_Data->m_CurrentContext) return;
    if (!m_Data->m_ParentList) return;
    SvnItem*k = m_Data->m_ParentList->Selected();
    if (!k) return;
    PropertiesDlg dlg(k,svnclient(),
        m_Data->m_ParentList->isWorkingCopy()?svn::Revision::WORKING:svn::Revision::HEAD);
    connect(&dlg,SIGNAL(clientException(const QString&)),m_Data->m_ParentList->realWidget(),SLOT(slotClientException(const QString&)));
    dlg.resize(dlg.configDialogSize(*(Settings::self()->config()), "properties_dlg"));
    if (dlg.exec()!=QDialog::Accepted) {
        return;
    }
    dlg.saveDialogSize(*(Settings::self()->config()),"properties_dlg",false);
    QString ex;
    PropertiesDlg::tPropEntries setList;
    QValueList<QString> delList;
    dlg.changedItems(setList,delList);
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Applying properties","<center>Applying<br>hit cancel for abort</center>");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        unsigned int pos;
        for (pos = 0; pos<delList.size();++pos) {
            m_Data->m_Svnclient->propdel(delList[pos],svn::Path(k->fullName()),svn::Revision::HEAD);
        }
        PropertiesDlg::tPropEntries::Iterator it;
        for (it=setList.begin(); it!=setList.end();++it) {
            m_Data->m_Svnclient->propset(it.key(),it.data(),svn::Path(k->fullName()),svn::Revision::HEAD);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
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
    if (!m_Data->m_CurrentContext) return;
    if (!m_Data->m_ParentList->isWorkingCopy()) return;
    QPtrList<SvnItem> which;
    m_Data->m_ParentList->SelectionList(&which);
    SvnItem*cur;
    QPtrListIterator<SvnItem> liter(which);

    svn::Pathes targets;
    if (which.count()==0) {
        targets.push_back(svn::Path(m_Data->m_ParentList->baseUri()));
    } else {
        while ( (cur=liter.current())!=0) {
            ++liter;
            kdDebug()<<"Commiting " << cur->fullName()<<endl;
            targets.push_back(svn::Path(cur->fullName()));
        }
    }
    makeCommit(targets);
}

bool SvnActions::makeCommit(const svn::Targets&targets)
{
    bool ok,rec;
    svn_revnum_t nnum;
    svn::Targets _targets;
    svn::Pathes _deldir;
    bool review = Settings::review_commit();
    QString msg;

    if (!review) {
        msg = Logmsg_impl::getLogmessage(&ok,&rec,
            m_Data->m_ParentList->realWidget(),"logmsg_impl");
        if (!ok) {
            return false;
        }
        _targets = targets;
    } else {
        Logmsg_impl::logActionEntries _check,_uncheck,_result;
        svn::StatusEntries _Cache;
        rec = false;
        /// @todo filter out double entries
        for (unsigned j = 0; j < targets.size(); ++j) {
            svn::Revision where = svn::Revision::HEAD;
            try {
                StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Status / List"),i18n("Creating list / check status"));
                _Cache = m_Data->m_Svnclient->status(targets.target(j).path(),true,false,false,false,where);
            } catch (svn::ClientException e) {
                emit clientException(e.msg());
                return false;
            }
            for (unsigned int i = 0; i < _Cache.count();++i) {
                if (_Cache[i].isRealVersioned()&& (
                    _Cache[i].textStatus()==svn_wc_status_modified||
                    _Cache[i].textStatus()==svn_wc_status_added||
                    _Cache[i].textStatus()==svn_wc_status_deleted||
                    _Cache[i].propStatus()==svn_wc_status_modified
                ) ) {
                    if (_Cache[i].textStatus()==svn_wc_status_deleted) {
                        _check.append(Logmsg_impl::logActionEntry(_Cache[i].path(),i18n("Commit"),2));
                    } else {
                        _check.append(Logmsg_impl::logActionEntry(_Cache[i].path(),i18n("Commit")));
                    }
                } else if (!_Cache[i].isVersioned()) {
                    _uncheck.append(Logmsg_impl::logActionEntry(_Cache[i].path(),i18n("Add and Commit"),1));
                }
            }
        }
        msg = Logmsg_impl::getLogmessage(_check,_uncheck,_result,&ok,
            m_Data->m_ParentList->realWidget(),"logmsg_impl");
        if (!ok||_result.count()==0) {
            return false;
        }
        svn::Pathes _add,_commit;
        for (unsigned int i=0; i < _result.count();++i) {
            if (_result[i]._kind==2) {
                QFileInfo fi(_result[i]._name);
                if (fi.isDir()) {
                    rec = true;
                }
            }
            _commit.append(_result[i]._name);
            if (_result[i]._kind==1) {
                _add.append(_result[i]._name);
            }
        }
        if (_add.count()>0) {
            if (!addItems(_add,false)) {
                return false;
            }
        }
        _targets = svn::Targets(_commit);
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Commiting","Commiting - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        nnum = m_Data->m_Svnclient->commit(_targets,msg,rec);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    EMIT_FINISHED;
    return true;
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
    QMap<KProcess*,QString>::iterator it;
    if ( (it=m_Data->m_tempfilelist.find(proc))!=m_Data->m_tempfilelist.end()) {
        ::unlink((*it).ascii());
        m_Data->m_tempfilelist.erase(it);
    }
    delete proc;
}


void SvnActions::makeDiff(const QStringList&which,const svn::Revision&start,const svn::Revision&end)
{
    if (!m_Data->m_CurrentContext) return;
    QString ex = "";
    KTempDir tdir;
    tdir.setAutoDelete(true);
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    svn::Path tmpPath(tn);
    bool ignore_content = Settings::diff_ignore_content();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Diffing","Diffing - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        for (unsigned int i = 0; i < which.count();++i) {
            ex += m_Data->m_Svnclient->diff(tmpPath,
                svn::Path(which[i]),
                start, end,
                true,false,false,ignore_content);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    dispDiff(ex);
}

/*!
    \fn SvnActions::makeDiff(const QString&,const svn::Revision&start,const svn::Revision&end)
 */
void SvnActions::makeDiff(const QString&what,const svn::Revision&start,const svn::Revision&end)
{
    QStringList w; w << what;
    makeDiff(w,start,end);
}

void SvnActions::makeDiff(const QString&p1,const svn::Revision&r1,const QString&p2,const svn::Revision&r2)
{
    if (!m_Data->m_CurrentContext) return;
    QString ex = "";
    KTempDir tdir;
    tdir.setAutoDelete(true);
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    bool ignore_content = Settings::diff_ignore_content();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Diffing","Diffing - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        ex = m_Data->m_Svnclient->diff(svn::Path(tn),
            svn::Path(p1),svn::Path(p2),
            r1, r2,
            true,false,false,ignore_content);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    dispDiff(ex);
}

void SvnActions::makeNorecDiff(const QString&p1,const svn::Revision&r1,const QString&p2,const svn::Revision&r2)
{
    if (!m_Data->m_CurrentContext) return;
    QString ex = "";
    KTempDir tdir;
    tdir.setAutoDelete(true);
    kdDebug()<<"Non recourse diff"<<endl;
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    bool ignore_content = Settings::diff_ignore_content();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Diffing","Diffing - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        ex = m_Data->m_Svnclient->diff(svn::Path(tn),
            svn::Path(p1),svn::Path(p2),
            r1, r2,
            false,false,false,ignore_content);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    dispDiff(ex);
}

void SvnActions::dispDiff(const QString&ex)
{
    int disp = Settings::use_kompare_for_diff();
    if (disp==1) {
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
    } else if (disp>1) {
        QString what = Settings::external_diff_display();
        QStringList wlist = QStringList::split(" ",what);
        KProcess*proc = new KProcess();
        bool fname_used = false;
        KTempFile tfile;
        tfile.setAutoDelete(false);

        for ( QStringList::Iterator it = wlist.begin();it!=wlist.end();++it) {
            if (*it=="%f") {
                fname_used = true;
                QByteArray ut = ex.utf8();
                QDataStream*ds = tfile.dataStream();
                ds->writeRawBytes(ut,ut.size());
                tfile.close();
                *proc<<tfile.name();
            } else {
                *proc << *it;
            }
        }
        connect(proc,SIGNAL(processExited(KProcess*)),this,SLOT(procClosed(KProcess*)));
        if (!fname_used) {
            connect(proc,SIGNAL(wroteStdin(KProcess*)),this,SLOT(wroteStdin(KProcess*)));
        }

        if (proc->start(KProcess::NotifyOnExit,fname_used?KProcess::NoCommunication:KProcess::Stdin)) {
            if (!fname_used) proc->writeStdin(ex.ascii(),ex.length());
            else m_Data->m_tempfilelist[proc]=tfile.name();
            return;
        }
        delete proc;
    }
    KTextBrowser*ptr;
    KDialogBase*dlg = createDialog(&ptr,QString(i18n("Diff display")),false,"diff_display");
    if (dlg) {
        ptr->setText("<code>"+QStyleSheet::convertFromPlainText(ex)+"</code>");
        dlg->exec();
        dlg->saveDialogSize(*(Settings::self()->config()),"diff_display",false);
        delete dlg;
    }
}


/*!
    \fn SvnActions::makeUpdate(const QString&what,const svn::Revision&rev,bool recurse)
 */
void SvnActions::makeUpdate(const QStringList&what,const svn::Revision&rev,bool recurse)
{
    if (!m_Data->m_CurrentContext) return;
    QString ex;
    svn::Revisions ret;
    stopCheckUpdateThread();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Making update","Making update - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        svn::Targets pathes(what);
        // always update externals, too. (last parameter)
        ret = m_Data->m_Svnclient->update(pathes,rev, recurse,false);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    removeFromUpdateCache(what,!recurse);
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
    if (!m_Data->m_ParentList||!m_Data->m_ParentList->isWorkingCopy()) return;
    SvnItemList k;
    m_Data->m_ParentList->SelectionList(&k);

    QStringList what;
    if (k.count()==0) {
        what.append(m_Data->m_ParentList->baseUri());
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
    makeAdd(false);
}

void SvnActions::slotAddRec()
{
    makeAdd(true);
}

void SvnActions::makeAdd(bool rec)
{
    if (!m_Data->m_CurrentContext) return;
    if (!m_Data->m_ParentList) return;
    QPtrList<SvnItem> lst;
    m_Data->m_ParentList->SelectionList(&lst);
    if (lst.count()==0) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("Which files or directories should I add?"));
        return;
    }
    QValueList<svn::Path> items;
    SvnItemListIterator liter(lst);
    SvnItem*cur;
    while ((cur=liter.current())!=0){
        ++liter;
        if (cur->isVersioned()) {
            KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("<center>The entry<br>%1<br>is versioned - break.</center>")
                .arg(cur->fullName()));
            return;
        }
        items.push_back(svn::Path(cur->fullName()));
    }
    addItems(items,rec);
    liter.toFirst();
#if 0
    while ((cur=liter.current())!=0){
        ++liter;
        //cur->refreshStatus();

        //emit sigRefreshCurrent(static_cast<FileListViewItem*>(cur->parent()));
    }
#else
    emit sigRefreshCurrent(0);
#endif
}

bool SvnActions::addItems(const QStringList&w,bool rec)
{
    QValueList<svn::Path> items;
    for (unsigned int i = 0; i<w.count();++i) {
        items.push_back(w[i]);
    }
    return addItems(items,rec);
}

bool SvnActions::addItems(const QValueList<svn::Path> &items,bool rec)
{
    QString ex;
    try {
        QValueList<svn::Path>::const_iterator piter;
        for (piter=items.begin();piter!=items.end();++piter) {
            m_Data->m_Svnclient->add((*piter),rec);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

void SvnActions::makeDelete(const QStringList&w)
{
    int answer = KMessageBox::questionYesNoList(0,i18n("Really delete these entries?"),w,i18n("Delete from repository"));
    if (answer!=KMessageBox::Yes) {
        return;
    }
    QValueList<svn::Path> items;
    for (unsigned int i = 0; i<w.count();++i) {
        items.push_back(w[i]);
    }
    makeDelete(items);
}

/*!
    \fn SvnActions::makeDelete()
 */
void SvnActions::makeDelete(const QValueList<svn::Path>&items)
{
    if (!m_Data->m_CurrentContext) return;
    QString ex;
    try {
        svn::Targets target(items);
        m_Data->m_Svnclient->remove(target,false);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
}

void SvnActions::CheckoutExport(bool _exp)
{
    CheckoutInfo_impl*ptr;
    KDialogBase * dlg = createDialog(&ptr,(_exp?i18n("Export repository"):i18n("Checkout a repository")),true,"checkout_export_dialog");
    if (dlg) {
        ptr->forceAsRecursive(!_exp);
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            bool openit = ptr->openAfterJob();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt(),_exp,openit);
        }
        dlg->saveDialogSize(*(Settings::self()->config()),"checkout_export_dialog",false);
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
    if (!m_Data->m_ParentList || !_exp&&m_Data->m_ParentList->isWorkingCopy()) return;
    SvnItem*k = m_Data->m_ParentList->Selected();
    if (k && !k->isDir()) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(),_exp?i18n("Exporting a file?"):i18n("Checking out a file?"));
        return;
    }
    QString what;
    if (!k) {
        what = m_Data->m_ParentList->baseUri();
    } else {
        what = k->fullName();
    }
    CheckoutExport(what,_exp);
}

void SvnActions::CheckoutExport(const QString&what,bool _exp,bool urlisTarget)
{
    CheckoutInfo_impl*ptr;
    KDialog * dlg = createDialog(&ptr,_exp?i18n("Export a repository"):i18n("Checkout a repository"),true);
    if (dlg) {
        if (!urlisTarget) {
            ptr->setStartUrl(what);
        } else {
            ptr->setTargetUrl(what);
        }
        ptr->forceAsRecursive(!_exp);
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

void SvnActions::makeCheckout(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool force_recurse,bool _exp,bool openIt)
{
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath);
    svn::Revision peg = svn::Revision::UNDEFINED;
    if (!_exp||!m_Data->m_CurrentContext) reInitClient();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,_exp?i18n("Export"):i18n("Checkout"),_exp?i18n("Exporting"):i18n("Checking out"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        if (_exp) {
            /// @todo setup parameter for export operation
            m_Data->m_Svnclient->doExport(svn::Path(fUrl),p,r,peg,force_recurse,QString::null,false,true);
        } else {
            m_Data->m_Svnclient->checkout(fUrl,p,r,peg,force_recurse,false);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    if (openIt) {
        if (!_exp) emit sigGotourl(tPath);
        else kapp->invokeBrowser(tPath);
    }
    EMIT_FINISHED;
}

void SvnActions::slotRevert()
{
    if (!m_Data->m_ParentList||!m_Data->m_ParentList->isWorkingCopy()) return;
    QPtrList<SvnItem> lst;
    m_Data->m_ParentList->SelectionList(&lst);
    QStringList displist;
    SvnItemListIterator liter(lst);
    SvnItem*cur;
    if (lst.count()>0) {
        while ((cur=liter.current())!=0){
            if (!cur->isVersioned()) {
                KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("<center>The entry<br>%1<br>is not versioned - break.</center>")
                    .arg(cur->fullName()));
                return;
            }
            displist.append(cur->fullName());
            ++liter;
        }
    } else {
        displist.push_back(m_Data->m_ParentList->baseUri());
    }
    slotRevertItems(displist);
#if 0
    liter.toFirst();
    while ((cur=liter.current())!=0){
        ++liter;
        cur->refreshStatus(true,&lst,false);
        cur->refreshStatus(false,&lst,true);
    }
#else
    EMIT_REFRESH;
#endif
}

void SvnActions::slotRevertItems(const QStringList&displist)
{
    if (!m_Data->m_CurrentContext) return;
    if (displist.count()==0) {
        return;
    }
    KDialogBase*dialog = new KDialogBase(
                i18n("Revert entries"),
                KDialogBase::Yes | KDialogBase::No,
                KDialogBase::No, KDialogBase::No,
                m_Data->m_ParentList->realWidget(),"warningRevert",true,true);

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
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Revert"),i18n("Reverting items"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        svn::Targets target(items);
        m_Data->m_Svnclient->revert(target,checkboxres);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    // remove them from cache
    for (unsigned int j = 0; j<items.count();++j) {
        m_Data->m_Cache.deleteKey(items[j].path(),!checkboxres);
        m_Data->m_Cache.dump_tree();
    }
    EMIT_FINISHED;
}

bool SvnActions::makeSwitch(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool rec)
{
    if (!m_Data->m_CurrentContext) return false;
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath);
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Switch url"),i18n("Switching url"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->doSwitch(p,fUrl,r,rec);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_FINISHED;
    return true;
}

bool SvnActions::makeRelocate(const QString&fUrl,const QString&tUrl,const QString&path,bool rec)
{
    if (!m_Data->m_CurrentContext) return false;
    QString _f = fUrl;
    QString _t = tUrl;
    QString ex;
    while (_f.endsWith("/")) {
        _f.truncate(_f.length()-1);
    }
    while (_t.endsWith("/")) {
        _t.truncate(_t.length()-1);
    }
    svn::Path p(path);
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Relocate url"),i18n("Relocate repository to new URL"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->relocate(p,_f,_t,rec);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_FINISHED;
    return true;

}

void SvnActions::slotSwitch()
{
    if (!m_Data->m_CurrentContext) return;
    if (!m_Data->m_ParentList||!m_Data->m_ParentList->isWorkingCopy()) return;

    QPtrList<SvnItem> lst;
    m_Data->m_ParentList->SelectionList(&lst);

    if (lst.count()>1) {
        KMessageBox::error(0,i18n("Can only switch one item at time"));
        return;
    }
    SvnItem*k;

    k = m_Data->m_ParentList->SelectedOrMain();
    if (!k) {
        KMessageBox::error(0,i18n("Error getting entry to switch"));
        return;
    }
    QString path,what;
    path = k->fullName();
    what = k->Url();
    if (makeSwitch(path,what)) {
        emit reinitItem(k);
    }
}

bool SvnActions::makeSwitch(const QString&path,const QString&what)
{
    CheckoutInfo_impl*ptr;
    KDialogBase * dlg = createDialog(&ptr,i18n("Switch url"),true,"switch_url_dlg");
    bool done = false;
    if (dlg) {
        ptr->setStartUrl(what);
        ptr->forceAsRecursive(true);
        ptr->disableAppend(true);
        ptr->disableTargetDir(true);
        ptr->disableOpen(true);
        if (dlg->exec()==QDialog::Accepted) {
            svn::Revision r = ptr->toRevision();
            done = makeSwitch(ptr->reposURL(),path,r,ptr->forceIt());
        }
        dlg->saveDialogSize(*(Settings::self()->config()),"switch_url_dlg",false);
        delete dlg;
    }
    return done;
}

void SvnActions::slotCleanup(const QString&path)
{
    if (!m_Data->m_CurrentContext) return;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Cleanup"),i18n("Cleaning up folder"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->cleanup(svn::Path(path));
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
}

void SvnActions::slotResolved(const QString&path)
{
    if (!m_Data->m_CurrentContext) return;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Resolve"),i18n("Marking resolved"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->resolved(svn::Path(path),true);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    m_Data->m_conflictCache.deleteKey(path,false);
}

void SvnActions::slotImport(const QString&path,const QString&target,const QString&message,bool rec)
{
    if (!m_Data->m_CurrentContext) return;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Import"),i18n("Importing items"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->import(svn::Path(path),target,message,rec);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
}

void SvnActions::slotMergeWcRevisions(const QString&_entry,const svn::Revision&rev1,
                    const svn::Revision&rev2,bool rec,bool ancestry,bool forceIt,bool dry)
{
    slotMerge(_entry,_entry,_entry,rev1,rev2,rec,ancestry,forceIt,dry);
}

void SvnActions::slotMerge(const QString&src1,const QString&src2, const QString&target,
        const svn::Revision&rev1,const svn::Revision&rev2,bool rec,bool ancestry,bool forceIt,bool dry)
{
    if (!m_Data->m_CurrentContext) return;
    QString s2;
    if (src2.isEmpty()) {
        s2 = src1;
    } else {
        s2 = src2;
    }
    try {
        m_Data->m_Svnclient->merge(svn::Path(src1),
            rev1,
            svn::Path(s2),
            rev2,
            svn::Path(target),
            forceIt,rec,ancestry,dry);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
}

/*!
    \fn SvnActions::slotCopyMove(bool,const QString&,const QString&)
 */
void SvnActions::slotCopyMove(bool move,const QString&Old,const QString&New,bool force)
{
    if (!m_Data->m_CurrentContext) return;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Copy / Move"),i18n("Copy or Moving entries"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        if (move) {
            m_Data->m_Svnclient->move(svn::Path(Old),svn::Path(New),force);
        } else {
            m_Data->m_Svnclient->copy(svn::Path(Old),svn::Revision::HEAD,
                svn::Path(New));
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_REFRESH;
}

void SvnActions::slotCopyMove(bool move,const KURL::List&Old,const QString&New,bool force)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Copy / Move"),i18n("Copy or Moving entries"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        KURL::List::ConstIterator it = Old.begin();
        for (;it!=Old.end();++it) {
            if (move) {
                m_Data->m_Svnclient->move(svn::Path((*it).url()),svn::Path(New),force);
            } else {
                m_Data->m_Svnclient->copy(svn::Path((*it).url()),svn::Revision::HEAD,
                     svn::Path(New));
            }
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
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
    if (!m_Data->m_CurrentContext) return;
    try {
        m_Data->m_Svnclient->lock(svn::Targets(targets),_msg,breakit);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
}


/*!
    \fn SvnActions::makeUnlock(const QStringList&)
 */
void SvnActions::makeUnlock(const QStringList&what,bool breakit)
{
    QValueList<svn::Path> targets;
    if (!m_Data->m_CurrentContext) return;
    for (unsigned int i = 0; i<what.count();++i) {
        targets.push_back(svn::Path((*(what.at(i)))));
    }

    try {
        m_Data->m_Svnclient->unlock(svn::Targets(targets),breakit);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
}


/*!
    \fn SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist)
 */
bool SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where,bool rec,bool all)
{
    bool display_ignores = Settings::display_ignored_files();
    return makeStatus(what,dlist,where,rec,all,display_ignores);
}

bool SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where,bool rec,bool all,bool display_ignores,bool updates)
{
    bool disp_remote_details = Settings::details_on_remote_listing();
    QString ex;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Status / List"),i18n("Creating list / check status"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        //                                      rec all  up     noign
        dlist = m_Data->m_Svnclient->status(what,rec,all,updates,display_ignores,where,disp_remote_details,false);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

void SvnActions::checkAddItems(const QString&path,bool print_error_box)
{
    svn::StatusEntries dlist;
    svn::StatusEntries rlist;
    QStringList displist;
    svn::Revision where = svn::Revision::HEAD;
    if (!makeStatus(path,dlist,where,true,true,false,false)) {
        return;
    }
    for (unsigned int i = 0; i<dlist.size();++i) {
        if (!dlist[i].isVersioned()) {
            rlist.append(dlist[i]);
            displist.append(dlist[i].path());
        }
    }
    if (rlist.size()==0) {
        if (print_error_box) KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("No unversioned items found."));
    } else {
        KListView*ptr;
        KDialogBase * dlg = createDialog(&ptr,i18n("Add unversioned items"),true,"add_items_dlg");
        ptr->addColumn("Item");
        for (unsigned j = 0; j<displist.size();++j) {
            QCheckListItem * n = new QCheckListItem(ptr,displist[j],QCheckListItem::CheckBox);
            n->setOn(true);
        }
        if (dlg->exec()==QDialog::Accepted) {
            QListViewItemIterator it(ptr);
            displist.clear();
            while(it.current()) {
                QCheckListItem*t = (QCheckListItem*)it.current();
                if (t->isOn()) {
                    displist.append(t->text());
                }
                ++it;
            }
            if (displist.count()>0) {
                addItems(displist,false);
            }
        }
        dlg->saveDialogSize(*(Settings::self()->config()),"add_items_dlg",false);
        delete dlg;
    }
}

void SvnActions::stopCheckModThread()
{
    m_Data->m_ThreadCheckTimer.stop();
    if (m_CThread) {
        m_CThread->cancelMe();
        if (!m_CThread->wait(MAX_THREAD_WAITTIME)) {
            m_CThread->terminate();
            m_CThread->wait(MAX_THREAD_WAITTIME);
        }
        delete m_CThread;
        m_CThread=0;
    }
}

void SvnActions::stopCheckUpdateThread()
{
    m_Data->m_UpdateCheckTimer.stop();
    if (m_UThread) {
        m_UThread->cancelMe();
        if (!m_UThread->wait(MAX_THREAD_WAITTIME)) {
            m_UThread->terminate();
            m_UThread->wait(MAX_THREAD_WAITTIME);
        }
        delete m_UThread;
        m_UThread=0;
    }
}

void SvnActions::killallThreads()
{
    stopCheckModThread();
    stopCheckUpdateThread();
}

bool SvnActions::createModifiedCache(const QString&what)
{
    stopCheckModThread();
    m_Data->m_Cache.clear();
    m_Data->m_conflictCache.clear();
    kdDebug()<<"Create cache for " << what << endl;
    m_CThread = new CheckModifiedThread(this,what);
    m_CThread->start();
    m_Data->m_ThreadCheckTimer.start(100,true);
    return true;
}

void SvnActions::checkModthread()
{
    if (!m_CThread)return;
    if (m_CThread->running()) {
        m_Data->m_ThreadCheckTimer.start(100,true);
        return;
    }
    kdDebug()<<"ModifiedThread seems stopped"<<endl;
    for (unsigned int i = 0; i < m_CThread->getList().count();++i) {
        if (m_CThread->getList()[i].isRealVersioned()&& (
            m_CThread->getList()[i].textStatus()==svn_wc_status_modified||
            m_CThread->getList()[i].textStatus()==svn_wc_status_added||
            m_CThread->getList()[i].textStatus()==svn_wc_status_deleted||
            m_CThread->getList()[i].propStatus()==svn_wc_status_modified
         ) ) {
            m_Data->m_Cache.insertKey(m_CThread->getList()[i]);
        } else if (m_CThread->getList()[i].textStatus()==svn_wc_status_conflicted) {
            m_Data->m_conflictCache.insertKey(m_CThread->getList()[i]);
        }
    }
    kdDebug()<<"Modified Cache"<<endl;
    m_Data->m_Cache.dump_tree();
    kdDebug()<<"Modified Cache end"<<endl;
    delete m_CThread;
    m_CThread = 0;
    emit sigRefreshIcons(false);
}

void SvnActions::checkUpdateThread()
{
    if (!m_UThread)return;
    if (m_UThread->running()) {
        if (m_Data->m_UpdateCheckTick.elapsed()>2500) {
            m_Data->m_UpdateCheckTick.restart();
            emit sendNotify(i18n("Still checking for updates"));
        }
        m_Data->m_UpdateCheckTimer.start(100,true);
        return;
    }
    kdDebug()<<"Updates Thread seems stopped"<<endl;

    bool newer=false;
    for (unsigned int i = 0; i < m_UThread->getList().count();++i) {
        if (m_UThread->getList()[i].validReposStatus()) {
            m_Data->m_UpdateCache.insertKey(m_UThread->getList()[i]);
            if (!m_UThread->getList()[i].validLocalStatus()) {
                newer = true;
            }
        }
    }
    m_Data->m_UpdateCache.dump_tree();
    emit sigRefreshIcons(newer);
    emit sendNotify(i18n("Checking for updates finished"));
    if (newer) {
        emit sendNotify(i18n("There are new items in repository"));
    }
    delete m_UThread;
    m_UThread = 0;
}

void SvnActions::getaddedItems(const QString&path,svn::StatusEntries&target)
{
    helpers::ValidRemoteOnly vro;
    m_Data->m_UpdateCache.listsubs_if(path,vro);
    target=vro.liste();
}

bool SvnActions::checkUpdatesRunning()
{
    return m_UThread && m_UThread->running();
}

void SvnActions::addModifiedCache(const svn::Status&what)
{
    if (what.textStatus()==svn_wc_status_conflicted) {
        m_Data->m_conflictCache.insertKey(what);
    } else {
        m_Data->m_Cache.insertKey(what);
        m_Data->m_Cache.dump_tree();
    }
}

void SvnActions::deleteFromModifiedCache(const QString&what)
{
    kdDebug()<<"deleteFromModifiedCache"<<endl;
    m_Data->m_Cache.deleteKey(what,true);
    m_Data->m_conflictCache.deleteKey(what,true);
    m_Data->m_Cache.dump_tree();
}

void SvnActions::getModifiedCache(const QString&path,svn::StatusEntries&dlist)
{
    m_Data->m_Cache.find(path,dlist);
}

bool SvnActions::checkModifiedCache(const QString&path)
{
    return m_Data->m_Cache.find(path);
}

bool SvnActions::checkConflictedCache(const QString&path)
{
    return m_Data->m_conflictCache.find(path);
}

/*!
    \fn SvnActions::createUpdateCache(const QString&what)
 */
bool SvnActions::createUpdateCache(const QString&what)
{
    clearUpdateCache();
    stopCheckUpdateThread();
    m_UThread = new CheckModifiedThread(this,what,true);
    m_UThread->start();
    m_Data->m_UpdateCheckTimer.start(100,true);
    emit sendNotify(i18n("Checking for updates started in background"));
    m_Data->m_UpdateCheckTick.start();
    return true;
}

bool SvnActions::checkUpdateCache(const QString&path)const
{
    return m_Data->m_UpdateCache.find(path);
}

void SvnActions::removeFromUpdateCache(const QStringList&what,bool exact_only)
{
    for (unsigned int i = 0; i < what.count(); ++i) {
        m_Data->m_UpdateCache.deleteKey(what[i],exact_only);
    }
}

bool SvnActions::isUpdated(const QString&path)const
{
    svn::Status d;
    return m_Data->m_UpdateCache.findSingleValid(path,d);
}

bool SvnActions::getUpdated(const QString&path,svn::Status&d)const
{
    return m_Data->m_UpdateCache.findSingleValid(path,d);
}

void SvnActions::clearUpdateCache()
{
    m_Data->m_UpdateCache.clear();
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
        pm = m_Data->m_Svnclient->propget("svn:ignore",p,r,r);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    QString data = "";
    if (pm.size()>0) {
        svn::PropertiesMap mp = pm[0].second;
        data = mp["svn:ignore"];
    }
    bool result = false;
    QStringList lst = QStringList::split("\n",data);
    QStringList::iterator it = lst.find(name);
    if (it != lst.end()) {
        if (unignore) {
            lst.erase(it);
            result = true;
        }
    } else {
        if (!unignore) {
            lst.append(name);
            result = true;
        }
    }
    if (result) {
        data = lst.join("\n");
        try {
            m_Data->m_Svnclient->propset("svn:ignore",data,p,r);
        } catch (svn::ClientException e) {
            emit clientException(e.msg());
            return false;
        }
    }
    return result;
}

bool SvnActions::makeList(const QString&url,svn::DirEntries&dlist,svn::Revision&where,bool rec)
{
    if (!m_Data->m_CurrentContext) return false;
    QString ex;
    try {
        dlist = m_Data->m_Svnclient->list(url,where,where,rec,false);
    } catch (svn::ClientException e) {
            emit clientException(e.msg());
            return false;
    }
    return true;
}

/*!
    \fn SvnActions::isLocalWorkingCopy(const KURL&url)
 */
bool SvnActions::isLocalWorkingCopy(const KURL&url,QString&_baseUri)
{
    if (url.isEmpty()||!url.isLocalFile()) return false;
    QString cleanpath = url.path();
    while (cleanpath.endsWith("/")) {
        cleanpath.truncate(cleanpath.length()-1);
    }
    _baseUri="";
    kdDebug()<<"Url: " << url << " - path: " << cleanpath << endl;
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Data->m_Svnclient->info(cleanpath,false,rev,peg);
    } catch (svn::ClientException e) {
        kdDebug()<< e.msg()<<endl;
        return false;
    }
    _baseUri=e[0].url();
    return true;
}

void SvnActions::slotExtraLogMsg(const QString&msg)
{
    emit sigExtraLogMsg(msg);
}

#include "svnactions.moc"
