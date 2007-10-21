/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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
#include "blamedisplay_impl.h"
#include "src/ksvnwidgets/logmsg_impl.h"
#include "src/ksvnwidgets/diffbrowser.h"
#include "src/ksvnwidgets/encodingselector_impl.h"
#include "graphtree/revisiontree.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/client.hpp"
#include "src/svnqt/annotate_line.hpp"
#include "src/svnqt/context_listener.hpp"
#include "src/svnqt/dirent.hpp"
#include "src/svnqt/targets.hpp"
#include "src/svnqt/url.hpp"
#include "src/svnqt/wc.hpp"
#include "src/svnqt/svnqt_defines.hpp"
#include "helpers/sub2qt.h"
#include "fronthelpers/cursorstack.h"
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
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <krun.h>
#include <kstdguiitem.h>

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
#include <qguardedptr.h>


// wait not longer than 10 seconds for a thread
#define MAX_THREAD_WAITTIME 10000

class SvnActionsData:public svn::ref_count
{
    typedef svn::SharedPointer<svn::PathPropertiesMapList> sPPlist;
public:
    SvnActionsData():ref_count()
    {
        m_Svnclient = svn::Client::getobject(0,0);
        m_CurrentContext = 0;
    }

    virtual ~SvnActionsData()
    {
        if (m_DiffDialog) {
            m_DiffDialog->saveDialogSize(*(Kdesvnsettings::self()->config()),"diff_display",false);
            delete m_DiffDialog;
        }
        QMap<KProcess*,QStringList>::iterator it;
        for (it=m_tempfilelist.begin();it!=m_tempfilelist.end();++it) {
            for (QStringList::iterator it2 = (*it).begin();
                it2 != (*it).end();++it2) {
                ::unlink((*it2).ascii());
            }
        }
        for (it=m_tempdirlist.begin();it!=m_tempdirlist.end();++it) {
            for (QStringList::iterator it2 = (*it).begin();
                it2 != (*it).end();++it2) {
                KIO::NetAccess::del((*it2),0);
            }
        }
        delete m_Svnclient;
        m_Svnclient = 0L;
    }

    bool isExternalDiff()
    {
        if (Kdesvnsettings::use_external_diff()) {
            QString edisp = Kdesvnsettings::external_diff_display();
            QStringList wlist = QStringList::split(" ",edisp);
            if (wlist.count()>=3 && edisp.find("%1")!=-1 && edisp.find("%2")!=-1) {
                return true;
            }
        }
        return false;
    }

    void clearCaches()
    {
        m_PropertiesCache.clear();
        m_contextData.clear();
        m_InfoCache.clear();
    }

    ItemDisplay* m_ParentList;

    svn::smart_pointer<CContextListener> m_SvnContext;
    svn::ContextP m_CurrentContext;
    svn::Client*m_Svnclient;

    helpers::statusCache m_UpdateCache;
    helpers::statusCache m_Cache;
    helpers::statusCache m_conflictCache;
    helpers::statusCache m_repoLockCache;
    helpers::itemCache<svn::PathPropertiesMapListPtr> m_PropertiesCache;
    /// \todo as persistent cache (sqlite?)
    helpers::itemCache<svn::InfoEntry> m_InfoCache;

    QMap<KProcess*,QStringList> m_tempfilelist;
    QMap<KProcess*,QStringList> m_tempdirlist;

    QTimer m_ThreadCheckTimer;
    QTimer m_UpdateCheckTimer;
    QTime m_UpdateCheckTick;
    QGuardedPtr<DiffBrowser> m_DiffBrowserPtr;
    QGuardedPtr<KDialogBase> m_DiffDialog;

    QMap<QString,QString> m_contextData;

    bool runblocked;
};

#define EMIT_FINISHED emit sendNotify(i18n("Finished"))
#define EMIT_REFRESH emit sigRefreshAll()
#define DIALOGS_SIZES "display_dialogs_sizes"

SvnActions::SvnActions(ItemDisplay *parent, const char *name,bool processes_blocked)
    : QObject(parent?parent->realWidget():0, name),SimpleLogCb()
{
    m_CThread = 0;
    m_UThread = 0;
    m_Data = new SvnActionsData();
    m_Data->m_ParentList = parent;
    m_Data->m_SvnContext = new CContextListener(this);
    m_Data->runblocked = processes_blocked;
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
    m_Data->clearCaches();
    m_Data->m_CurrentContext = new svn::Context();
    m_Data->m_CurrentContext->setListener(m_Data->m_SvnContext);
    m_Data->m_Svnclient->setContext(m_Data->m_CurrentContext);
}

template<class T> KDialogBase* SvnActions::createDialog(T**ptr,const QString&_head,bool OkCancel,const char*name,bool showHelp,bool modal,const KGuiItem&u1)
{
    int buttons = KDialogBase::Ok;
    if (OkCancel) {
        buttons = buttons|KDialogBase::Cancel;
    }
    if (showHelp) {
        buttons = buttons|KDialogBase::Help;
    }
    if (!u1.text().isEmpty()) {
        buttons = buttons|KDialogBase::User1;
    }
    KDialogBase * dlg = new KDialogBase(
        modal?KApplication::activeModalWidget():0, // parent
        name, // name
        modal, // modal
        _head, // caption
        buttons, // buttonmask
        KDialogBase::Ok, // defaultButton
        false , // separator
        (u1.text().isEmpty()?KGuiItem():u1) //user1
                                       );

    if (!dlg) return dlg;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    *ptr = new T(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),name?name:DIALOGS_SIZES));
    return dlg;
}

/*!
    \fn SvnActions::makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k)
 */
void SvnActions::makeLog(const svn::Revision&start,const svn::Revision&end,SvnItem*k,bool list_files,int limit)
{
    if (!k)return;
    makeLog(start,end,k->fullName(),list_files,limit);
}

svn::SharedPointer<svn::LogEntriesMap> SvnActions::getLog(const svn::Revision&start,const svn::Revision&end,const QString&which,bool list_files,int limit)
{
    svn::SharedPointer<svn::LogEntriesMap> logs = new svn::LogEntriesMap;
    QString ex;
    if (!m_Data->m_CurrentContext) return 0;

    bool follow = Kdesvnsettings::log_follows_nodes();

    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Logs",
            i18n("Getting logs - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->log(which,start,end,*logs,list_files,!follow,limit);
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

bool SvnActions::getSingleLog(svn::LogEntry&t,const svn::Revision&r,const QString&what,const svn::Revision&peg,QString&root)
{
    bool res = false;

    if (what.isEmpty()) {
        return res;
    }
    if (root.isEmpty()) {
        svn::InfoEntry inf;
        if (!singleInfo(what,peg,inf))
        {
            return res;
        }
        root = inf.reposRoot();
    }
    svn::SharedPointer<svn::LogEntriesMap> log = getLog(r,r,root,true,1);
    if (log) {
        if (log->find(r.revnum())!=log->end()) {
            t = (*log)[r.revnum()];
            res = true;
        }
    }
    return res;
}

bool SvnActions::singleInfo(const QString&what,const svn::Revision&_rev,svn::InfoEntry&target)
{
    QString url;
    QString ex;
    QString cacheKey;
    QTime d; d.start();
    svn::Revision rev = _rev;
    svn::Revision peg;
    if (!m_Data->m_CurrentContext) return false;
    if (!svn::Url::isValid(what)) {
        // working copy
        // url = svn::Wc::getUrl(what);
        url = what;
        if (url.find("@")!=-1) {
            url+="@BASE";
        }
        peg = svn::Revision::UNDEFINED;
        cacheKey=url;
    } else {
        KURL _uri = what;
        QString prot = svn::Url::transformProtokoll(_uri.protocol());
        _uri.setProtocol(prot);
        url = _uri.prettyURL();
        peg = _rev;
        cacheKey=_rev.toString()+"/"+url;
    }
    svn::InfoEntries e;

    if (cacheKey.isEmpty() || !m_Data->m_InfoCache.findSingleValid(cacheKey,target)) {
        try {
            e = (m_Data->m_Svnclient->info(url,false,_rev,peg));
        } catch (svn::ClientException ce) {
            kdDebug()<<ce.msg() << endl;
            emit clientException(ce.msg());
            return false;
        }
        if (e.count()<1||e[0].reposRoot().isEmpty()) {
            kdDebug()<<"No info found" << endl;
            emit clientException(i18n("Got no info."));
            return false;
        }
        target = e[0];
        if (!cacheKey.isEmpty()) {
            m_Data->m_InfoCache.insertKey(e[0],cacheKey);
        }
    }

    return true;
}

void SvnActions::makeTree(const QString&what,const svn::Revision&_rev,const svn::Revision&startr,const svn::Revision&endr)
{
    svn::InfoEntry info;
    if (!singleInfo(what,_rev,info)) {
        return;
    }
    QString reposRoot = info.reposRoot();

    kdDebug()<<"Logs for "<<reposRoot<<endl;
    QWidget*disp;
    KDialogBase dlg(m_Data->m_ParentList->realWidget(),"historylist",true,i18n("History of %1").arg(info.url().mid(reposRoot.length())),
        KDialogBase::Ok,
        KDialogBase::Ok,true);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();

    RevisionTree rt(m_Data->m_Svnclient,m_Data->m_SvnContext,reposRoot,
            startr,endr,
            info.prettyUrl().mid(reposRoot.length()),_rev,Dialog1Layout,m_Data->m_ParentList->realWidget());
    if (rt.isValid()) {
        disp = rt.getView();
        if (disp) {
            connect(
                disp,SIGNAL(makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
                this,SLOT(makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*))
            );
            connect(
                disp,SIGNAL(makeRecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
                this,SLOT(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*))
                   );
            connect(disp,SIGNAL(makeCat(const svn::Revision&, const QString&,const QString&,const svn::Revision&,QWidget*)),
                this,SLOT(slotMakeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*)));
            dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),"revisiontree_dlg"));
            dlg.exec();
            dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),"revisiontree_dlg",false);
        }
    }
}

void SvnActions::makeLog(const svn::Revision&start,const svn::Revision&end,const QString&which,bool list_files,int limit)
{

    svn::InfoEntry info;
    if (!singleInfo(which,start,info)) {
        return;
    }
    QString reposRoot = info.reposRoot();

    svn::SharedPointer<svn::LogEntriesMap> logs = getLog(start,end,which,list_files,limit);
    if (!logs) return;
    SvnLogDlgImp disp(this);
    disp.dispLog(logs,info.url().mid(reposRoot.length()),reposRoot);
    connect(&disp,SIGNAL(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)),
            this,SLOT(makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*)));
    connect(&disp,SIGNAL(makeCat(const svn::Revision&, const QString&,const QString&,const svn::Revision&,QWidget*)),
            this,SLOT(slotMakeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*)));
    disp.exec();
    disp.saveSize();
    EMIT_FINISHED;
}

void SvnActions::makeBlame(const svn::Revision&start, const svn::Revision&end, SvnItem*k)
{
    if (k) makeBlame(start,end,k->fullName(),m_Data->m_ParentList->realWidget());
}

void SvnActions::makeBlame(const svn::Revision&start, const svn::Revision&end,const QString&k,QWidget*_p,const svn::Revision&_peg,SimpleLogCb*_acb)
{
    if (!m_Data->m_CurrentContext) return;
    svn::AnnotatedFile blame;
    QString ex;
    svn::Path p(k);
    QWidget*_parent = _p?_p:m_Data->m_ParentList->realWidget();
    svn::Revision peg = _peg==svn::Revision::UNDEFINED?end:_peg;

    try {
        CursorStack a(Qt::BusyCursor);
        StopDlg sdlg(m_Data->m_SvnContext,_parent,0,"Annotate",i18n("Annotate lines - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->annotate(blame,p,start,end,peg);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    if (blame.count()==0) {
        ex = i18n("Got no annotate");
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
    BlameDisplay_impl::displayBlame(_acb?_acb:this,k,blame,_p,"blame_dlg");
}

bool SvnActions::makeGet(const svn::Revision&start, const QString&what, const QString&target,
    const svn::Revision&peg,QWidget*_dlgparent)
{
    if (!m_Data->m_CurrentContext) return false;
    CursorStack a(Qt::BusyCursor);
    QWidget*dlgp=_dlgparent?_dlgparent:m_Data->m_ParentList->realWidget();
    QString ex;
    svn::Path p(what);
    try {
        StopDlg sdlg(m_Data->m_SvnContext,dlgp,
            0,"Content get",i18n("Getting content - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->get(p,target,start,peg);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    } catch (...) {
        ex = i18n("Error getting content");
        emit clientException(ex);
        return false;
    }
    return true;
}

void SvnActions::slotMakeCat(const svn::Revision&start, const QString&what, const QString&disp,const svn::Revision&peg,QWidget*_dlgparent)
{
    KTempFile content;
    content.setAutoDelete(true);
    if (!makeGet(start,what,content.name(),peg,_dlgparent)) {
        return;
    }
    EMIT_FINISHED;
    KMimeType::Ptr mptr;
    mptr = KMimeType::findByFileContent(content.name());
    KTrader::OfferList offers = KTrader::self()->query(mptr->name(), "Type == 'Application' or (exist Exec)");
    if (offers.count()==0 || offers.first()->exec().isEmpty()) {
        offers = KTrader::self()->query(mptr->name(), "Type == 'Application'");
    }
    KTrader::OfferList::ConstIterator it = offers.begin();
    for( ; it != offers.end(); ++it ) {
        if ((*it)->noDisplay())
            continue;
        break;
    }

    if (it!=offers.end()) {
        content.setAutoDelete(false);
        KRun::run(**it,KURL(content.name()),true);
        return;
    }
    KTextBrowser*ptr;
    QFile file(content.name());
    file.open( IO_ReadOnly );
    QByteArray co = file.readAll();

    if (co.size()) {
        KDialogBase*dlg = createDialog(&ptr,QString(i18n("Content of %1")).arg(disp),false,"cat_display_dlg");
        if (dlg) {
            ptr->setFont(KGlobalSettings::fixedFont());
            ptr->setWordWrap(QTextEdit::NoWrap);
            ptr->setText(QString::FROMUTF8(co,co.size()));
            dlg->exec();
            dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"cat_display_dlg",false);
            delete dlg;
        }
    } else {
        KMessageBox::information(_dlgparent?_dlgparent:m_Data->m_ParentList->realWidget(),
                                 i18n("Got no content."));
    }
}

bool SvnActions::makeMkdir(const QStringList&which,const QString&logMessage)
{
    if (!m_Data->m_CurrentContext||which.count()<1) return false;
    svn::Targets targets(which);
    try {
        m_Data->m_Svnclient->mkdir(targets,logMessage);
    }catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
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

    QString logMessage=QString::null;
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
        res += getInfo(item->fullName(),rev,peg,recursive,all);
    }
    return res;
}

QString SvnActions::getInfo(const QString& _what,const svn::Revision&rev,const svn::Revision&peg,bool recursive,bool all)
{
    if (!m_Data->m_CurrentContext) return QString::null;
    QString ex;
    svn::InfoEntries entries;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Details",
            i18n("Retrieving infos - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        svn::InfoEntries e;
        entries = (m_Data->m_Svnclient->info(_what+
                    (_what.find("@")>-1&&!svn::Url::isValid(_what)?"@BASE":""),recursive,rev,peg));
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
        if ((*it).cmtDate()>0) {
            text+=rb+i18n("Last committed")+cs+helpers::sub2qt::DateTime2qtString((*it).cmtDate())+re;
        }
        text+=rb+i18n("Last revision")+cs+QString("%1").arg((*it).cmtRev())+re;
        if ((*it).textTime()>0) {
            text+=rb+i18n("Content last changed")+cs+helpers::sub2qt::DateTime2qtString((*it).textTime())+re;
        }
        if (all) {
            if ((*it).propTime()>0) {
                text+=rb+i18n("Property last changed")+cs+helpers::sub2qt::DateTime2qtString((*it).propTime())+re;
            }
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
                    helpers::sub2qt::DateTime2qtString((*it).lockEntry().Date())+
                    re;
                text+=rb+i18n("Lock comment")+cs+
                    (*it).lockEntry().Comment()+re;
            } else {
                svn::SharedPointer<svn::Status> d;
                if (checkReposLockCache(_what,d)&& d && d->lockEntry().Locked()) {
                    text+=rb+i18n("Lock token")+cs+(d->lockEntry().Token())+re;
                    text+=rb+i18n("Owner")+cs+(d->lockEntry().Owner())+re;
                    text+=rb+i18n("Locked on")+cs+
                            helpers::sub2qt::DateTime2qtString(d->lockEntry().Date())+
                        re;
                    text+=rb+i18n("Lock comment")+cs+
                            d->lockEntry().Comment()+re;
                }
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
        QString text = getInfo(item->fullName(),rev,peg,recursive,true);
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
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"info_dialog",false);
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
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"info_dialog",false);
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
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()), "properties_dlg"));
    if (dlg.exec()!=QDialog::Accepted) {
        return;
    }
    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),"properties_dlg",false);
    QString ex;
    svn::PropertiesMap setList;
    QValueList<QString> delList;
    dlg.changedItems(setList,delList);
    changeProperties(setList,delList,k->fullName());
    k->refreshStatus();
    EMIT_FINISHED;
}

bool SvnActions::changeProperties(const svn::PropertiesMap&setList,const QValueList<QString>&delList,const QString&path)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Applying properties","<center>Applying<br>hit cancel for abort</center>");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        unsigned int pos;
        for (pos = 0; pos<delList.size();++pos) {
            m_Data->m_Svnclient->propdel(delList[pos],svn::Path(path),svn::Revision::WORKING);
        }
        svn::PropertiesMap::ConstIterator it;
        for (it=setList.begin(); it!=setList.end();++it) {
            m_Data->m_Svnclient->propset(it.key(),it.data(),svn::Path(path),svn::Revision::WORKING);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
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
        targets.push_back(svn::Path("."));
    } else {
        while ( (cur=liter.current())!=0) {
            ++liter;
            targets.push_back(svn::Path(
                    m_Data->m_ParentList->relativePath(cur)
                                       ));
        }
    }
    makeCommit(targets);
}

bool SvnActions::makeCommit(const svn::Targets&targets)
{
    bool ok,rec,keeplocks;
    svn::Revision nnum;
    svn::Targets _targets;
    svn::Pathes _deldir;
    bool review = Kdesvnsettings::review_commit();
    QString msg,_p;

    if (!review) {
        msg = Logmsg_impl::getLogmessage(&ok,&rec,&keeplocks,
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
                _p = _Cache[i]->path();
                if (_Cache[i]->isRealVersioned()&& (
                    _Cache[i]->textStatus()==svn_wc_status_modified||
                    _Cache[i]->textStatus()==svn_wc_status_added||
                    _Cache[i]->textStatus()==svn_wc_status_replaced||
                    _Cache[i]->textStatus()==svn_wc_status_deleted||
                    _Cache[i]->propStatus()==svn_wc_status_modified
                ) ) {
                    if (_Cache[i]->textStatus()==svn_wc_status_deleted) {
                        _check.append(Logmsg_impl::logActionEntry(_p,i18n("Delete"),Logmsg_impl::logActionEntry::DELETE));
                    } else {
                        _check.append(Logmsg_impl::logActionEntry(_p,i18n("Commit"),Logmsg_impl::logActionEntry::COMMIT));
                    }
                } else if (_Cache[i]->textStatus()==svn_wc_status_missing) {
                    _uncheck.append(Logmsg_impl::logActionEntry(_p,i18n("Delete and Commit"),Logmsg_impl::logActionEntry::MISSING_DELETE));
                } else if (!_Cache[i]->isVersioned()) {
                    _uncheck.append(Logmsg_impl::logActionEntry(_p,i18n("Add and Commit"),Logmsg_impl::logActionEntry::ADD_COMMIT));
                }
            }
        }
        msg = Logmsg_impl::getLogmessage(_check,_uncheck,this,_result,&ok,&keeplocks,
            m_Data->m_ParentList->realWidget(),"logmsg_impl");
        if (!ok||_result.count()==0) {
            return false;
        }
        svn::Pathes _add,_commit,_delete;
        for (unsigned int i=0; i < _result.count();++i) {
            if (_result[i]._kind==Logmsg_impl::logActionEntry::DELETE) {
                QFileInfo fi(_result[i]._name);
                if (fi.isDir()) {
                    rec = true;
                }
            }
            _commit.append(_result[i]._name);
            if (_result[i]._kind==Logmsg_impl::logActionEntry::ADD_COMMIT) {
                _add.append(_result[i]._name);
            } else if (_result[i]._kind==Logmsg_impl::logActionEntry::MISSING_DELETE) {
                _delete.append(_result[i]._name);
            }
        }
        if (_add.count()>0) {
            if (!addItems(_add,false)) {
                return false;
            }
        }
        if (_delete.count()>0) {
            makeDelete(_delete);
        }
        _targets = svn::Targets(_commit);
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Commiting"),
            i18n("Commiting - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        nnum = m_Data->m_Svnclient->commit(_targets,msg,rec,keeplocks);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    emit sendNotify(i18n("Committed revision %1.").arg(nnum.toString()));
    return true;
}

/*!
    \fn SvnActions::wroteStdin(KProcess*)
 */
void SvnActions::wroteStdin(KProcess*proc)
{
    if (!proc) return;
    kdDebug()<<"void SvnActions::wroteStdin(KProcess*proc)"<<endl;
    proc->closeStdin();
}

void SvnActions::receivedStderr(KProcess*proc,char*buff,int len)
{
    if (!proc || !buff || len == 0) {
        return;
    }
    QString msg(QCString(buff,len));
    emit sendNotify(msg);
}

void SvnActions::procClosed(KProcess*proc)
{
    if (!proc) return;
    QMap<KProcess*,QStringList>::iterator it;
    if ( (it=m_Data->m_tempfilelist.find(proc))!=m_Data->m_tempfilelist.end()) {
        for (QStringList::iterator it2 = (*it).begin();
            it2 != (*it).end();++it2) {
                ::unlink((*it2).ascii());
        }
        m_Data->m_tempfilelist.erase(it);
    }
    if ( (it=m_Data->m_tempdirlist.find(proc))!=m_Data->m_tempdirlist.end()) {
        for (QStringList::iterator it2 = (*it).begin();
            it2 != (*it).end();++it2) {
                KIO::NetAccess::del((*it2),0);
        }
        m_Data->m_tempdirlist.erase(it);
    }
    delete proc;
}

bool SvnActions::get(const QString&what,const QString& to,const svn::Revision&rev,const svn::Revision&peg,QWidget*p)
{
    kdDebug()<<"Downloading "<<what<<endl;
    svn::Revision _peg = peg;
    if (_peg == svn::Revision::UNDEFINED) {
        _peg = rev;
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContext,p?p:m_Data->m_ParentList->realWidget(),0,"Downloading",
        i18n("Download - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->get(svn::Path(what),
            to,rev,_peg);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

/*!
    \fn SvnActions::makeDiff(const QString&,const svn::Revision&start,const svn::Revision&end)
 */
void SvnActions::makeDiff(const QString&what,const svn::Revision&start,const svn::Revision&end,bool isDir)
{
    makeDiff(what,start,what,end,isDir,m_Data->m_ParentList->realWidget());
}

void SvnActions::makeDiff(const QString&p1,const svn::Revision&start,const QString&p2,const svn::Revision&end)
{
    makeDiff(p1,start,p2,end,(QWidget*)0);
}

void SvnActions::makeDiff(const QString&p1,const svn::Revision&start,const QString&p2,const svn::Revision&end,QWidget*p)
{
    if (m_Data->isExternalDiff()) {
        kdDebug()<<"External diff..."<<endl;
        svn::InfoEntry info;
        if (singleInfo(p1,start,info)) {
            makeDiff(p1,start,p2,end,info.isDir(),p);
        }
        return;
    }
    makeDiffinternal(p1,start,p2,end,p);
}

void SvnActions::makeDiffExternal(const QString&p1,const svn::Revision&start,const QString&p2,const svn::Revision&end,bool isDir,QWidget*p,bool rec)
{
    QString edisp = Kdesvnsettings::external_diff_display();
    QStringList wlist = QStringList::split(" ",edisp);
    QFileInfo f1(p1);
    QFileInfo f2(p2);
    KTempFile tfile(QString::null,f1.fileName()+"-"+start.toString()),tfile2(QString::null,f2.fileName()+"-"+end.toString());
    QString s1 = f1.fileName()+"-"+start.toString();
    QString s2 = f2.fileName()+"-"+end.toString();
    KTempDir tdir1;
    tdir1.setAutoDelete(true);
    tfile.setAutoDelete(true);
    tfile2.setAutoDelete(true);
    QString first,second;

    if (start != svn::Revision::WORKING) {
        first = isDir?tdir1.name()+"/"+s1:tfile.name();
    } else {
        first = p1;
    }
    if (end!=svn::Revision::WORKING) {
        second = isDir?tdir1.name()+"/"+s2:tfile2.name();
    } else {
        second = p2;
    }
    if (second == first) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("Both entries seems to be the same, can not diff."));
        return;
    }

    if (start != svn::Revision::WORKING) {
        if (!isDir) {
            if (!get(p1,tfile.name(),start,svn::Revision::UNDEFINED,p)) {
                return;
            }
        } else {
            if (!makeCheckout(p1,first,start,true,true,false,false,rec,p)) {
                return;
            }
        }
    }
    if (end!=svn::Revision::WORKING) {
        if (!isDir) {
            if (!get(p2,tfile2.name(),end,svn::Revision::UNDEFINED,p)) {
                return;
            }
        } else {
            if (!makeCheckout(p2,second,end,true,true,false,false,rec,p)) {
                return;
            }
        }
    }
    KProcess*proc = new KProcess();
    for ( QStringList::Iterator it = wlist.begin();it!=wlist.end();++it) {
        if (*it=="%1") {
            *proc<<first;
        } else if (*it=="%2") {
            *proc<<second;
        } else {
            *proc << *it;
        }
    }
    connect(proc,SIGNAL(processExited(KProcess*)),this,SLOT(procClosed(KProcess*)));
    connect(proc,SIGNAL(receivedStderr(KProcess*,char*,int)),this,SLOT(receivedStderr(KProcess*,char*,int)));
    connect(proc,SIGNAL(receivedStdout(KProcess*,char*,int)),this,SLOT(receivedStderr(KProcess*,char*,int)));
    if (proc->start(m_Data->runblocked?KProcess::Block:KProcess::NotifyOnExit,KProcess::All)) {
        if (!m_Data->runblocked) {
            if (!isDir) {
                tfile2.setAutoDelete(false);
                tfile.setAutoDelete(false);
                m_Data->m_tempfilelist[proc].append(tfile.name());
                m_Data->m_tempfilelist[proc].append(tfile2.name());
            } else {
                tdir1.setAutoDelete(false);
                m_Data->m_tempdirlist[proc].append(tdir1.name());
            }
        }
        return;
    } else {
        emit sendNotify(i18n("Diff-process could not started, check command."));
    }
    delete proc;
    return;
}

void SvnActions::makeDiff(const QString&p1,const svn::Revision&start,const QString&p2,const svn::Revision&end,bool isDir,QWidget*p)
{
    if (m_Data->isExternalDiff()) {
        kdDebug()<<"External diff 2..."<<endl;
        makeDiffExternal(p1,start,p2,end,isDir,p);
    } else {
        makeDiffinternal(p1,start,p2,end,p);
    }
}

void SvnActions::makeDiffinternal(const QString&p1,const svn::Revision&r1,const QString&p2,const svn::Revision&r2,QWidget*p)
{
    if (!m_Data->m_CurrentContext) return;
    QByteArray ex;
    KTempDir tdir;
    tdir.setAutoDelete(true);
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    bool ignore_content = Kdesvnsettings::diff_ignore_content();
    QWidget*parent = p?p:m_Data->m_ParentList->realWidget();
    QStringList extraOptions;
    if (Kdesvnsettings::diff_ignore_spaces())
    {
        extraOptions.append("-b");
    }
    if (Kdesvnsettings::diff_ignore_all_white_spaces())
    {
        extraOptions.append("-w");
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContext,parent,0,"Diffing",
            i18n("Diffing - hit cancel for abort"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        ex = m_Data->m_Svnclient->diff(svn::Path(tn),
            svn::Path(p1),svn::Path(p2),
            r1, r2,
            true,false,false,ignore_content,extraOptions);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    if (ex.isEmpty()) {
        emit clientException(i18n("No difference to display"));
        return;
    }
    dispDiff(ex);
}

void SvnActions::makeNorecDiff(const QString&p1,const svn::Revision&r1,const QString&p2,const svn::Revision&r2,QWidget*_p)
{
    if (!m_Data->m_CurrentContext) return;
    if (m_Data->isExternalDiff()) {
        svn::InfoEntry info;
        if (singleInfo(p1,r1,info)) {
            makeDiffExternal(p1,r1,p2,r2,info.isDir(),_p,false);
        }
        return;
    }
    QStringList extraOptions;
    if (Kdesvnsettings::diff_ignore_spaces())
    {
        extraOptions.append("-b");
    }
    if (Kdesvnsettings::diff_ignore_all_white_spaces())
    {
        extraOptions.append("-w");
    }
    QByteArray ex;
    KTempDir tdir;
    tdir.setAutoDelete(true);
    kdDebug()<<"Non recourse diff"<<endl;
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    bool ignore_content = Kdesvnsettings::diff_ignore_content();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,_p?_p:m_Data->m_ParentList->realWidget(),0,"Diffing","Diffing - hit cancel for abort");
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        ex = m_Data->m_Svnclient->diff(svn::Path(tn),
            svn::Path(p1),svn::Path(p2),
            r1, r2,
            false,false,false,ignore_content,extraOptions);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    if (ex.isEmpty()) {
        emit clientException(i18n("No difference to display"));
        return;
    }

    dispDiff(ex);
}

void SvnActions::dispDiff(const QByteArray&ex)
{
    QString what = Kdesvnsettings::external_diff_display();
    int r = KProcess::Stdin|KProcess::Stderr;

    if (Kdesvnsettings::use_external_diff() && (what.find("%1")==-1 || what.find("%2")==-1)) {
        QStringList wlist = QStringList::split(" ",what);
        KProcess*proc = new KProcess();
        bool fname_used = false;
        KTempFile tfile;
        tfile.setAutoDelete(false);

        for ( QStringList::Iterator it = wlist.begin();it!=wlist.end();++it) {
            if (*it=="%f") {
                fname_used = true;
                QDataStream*ds = tfile.dataStream();
                ds->writeRawBytes(ex,ex.size());
                tfile.close();
                *proc<<tfile.name();
            } else {
                *proc << *it;
            }
        }

        connect(proc,SIGNAL(processExited(KProcess*)),this,SLOT(procClosed(KProcess*)));
        connect(proc,SIGNAL(receivedStderr(KProcess*,char*,int)),this,SLOT(receivedStderr(KProcess*,char*,int)));
        if (!fname_used) {
            connect(proc,SIGNAL(wroteStdin(KProcess*)),this,SLOT(wroteStdin(KProcess*)));
        }
        if (proc->start(KProcess::NotifyOnExit,fname_used?KProcess::Stderr:(KProcess::Communication)r)) {
            if (!fname_used) proc->writeStdin(ex,ex.size());
            else m_Data->m_tempfilelist[proc].append(tfile.name());
            return;
        } else {
            emit sendNotify(i18n("Display-process could not started, check command."));
        }
        delete proc;
    }
    bool need_modal = m_Data->runblocked||KApplication::activeModalWidget()!=0;
    if (need_modal||!m_Data->m_DiffBrowserPtr||!m_Data->m_DiffDialog) {
        DiffBrowser*ptr;

        if (!need_modal && m_Data->m_DiffBrowserPtr) {
            delete m_Data->m_DiffBrowserPtr;
        }
        KDialogBase*dlg = createDialog(&ptr,QString(i18n("Diff display")),false,
                                        "diff_display",false,need_modal,
                                      KStdGuiItem::saveAs());
        if (dlg) {
            QWidget*wd = dlg->mainWidget();
            if (wd) {
                EncodingSelector_impl * ls = new EncodingSelector_impl("",wd);
                QObject::connect(ls,SIGNAL(TextCodecChanged(const QString&)),
                                 ptr,SLOT(slotTextCodecChanged(const QString&)));
            }
            QObject::connect(dlg,SIGNAL(user1Clicked()),ptr,SLOT(saveDiff()));
            ptr->setText(ex);
            if (need_modal) {
                ptr->setFocus();
                dlg->exec();
                dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"diff_display",false);
                delete dlg;
                return;
            } else {
                m_Data->m_DiffBrowserPtr=ptr;
                m_Data->m_DiffDialog=dlg;
            }
        }
    } else {
        m_Data->m_DiffBrowserPtr->setText(ex);
        m_Data->m_DiffBrowserPtr->setFocus();
    }
    if (m_Data->m_DiffDialog) {
        m_Data->m_DiffDialog->show();
        m_Data->m_DiffDialog->raise();
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
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,"Making update",
            i18n("Making update - hit cancel for abort"));
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

bool SvnActions::makeDelete(const QStringList&w)
{
    int answer = KMessageBox::questionYesNoList(0,i18n("Really delete these entries?"),w,i18n("Delete from repository"));
    if (answer!=KMessageBox::Yes) {
        return false;
    }
    svn::Pathes items;
    for (unsigned int i = 0; i<w.count();++i) {
        items.push_back(w[i]);
    }
    return makeDelete(items);
}

/*!
    \fn SvnActions::makeDelete()
 */
bool SvnActions::makeDelete(const svn::Pathes&items)
{
    if (!m_Data->m_CurrentContext) return false;
    QString ex;
    try {
        svn::Targets target(items);
        m_Data->m_Svnclient->remove(target,false);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_FINISHED;
    return true;
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
            bool ignoreExternal=ptr->ignoreExternals();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt(),_exp,openit,ignoreExternal);
        }
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"checkout_export_dialog",false);
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
            bool ignoreExternal = ptr->ignoreExternals();
            makeCheckout(ptr->reposURL(),ptr->targetDir(),r,ptr->forceIt(),_exp,openIt,ignoreExternal);
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

bool SvnActions::makeCheckout(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool force_recurse,bool _exp,bool openIt,bool ignoreExternal,bool exp_rec, QWidget*_p)
{
    QString fUrl = rUrl;
    QString ex;
    while (fUrl.endsWith("/")) {
        fUrl.truncate(fUrl.length()-1);
    }
    svn::Path p(tPath);
    svn::Revision peg = svn::Revision::UNDEFINED;
    if (r!=svn::Revision::BASE && r!=svn::Revision::WORKING) {
        peg = r;
    }
    if (!_exp||!m_Data->m_CurrentContext) reInitClient();
    try {
        StopDlg sdlg(m_Data->m_SvnContext,_p?_p:m_Data->m_ParentList->realWidget(),0,_exp?i18n("Export"):i18n("Checkout"),_exp?i18n("Exporting"):i18n("Checking out"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        if (_exp) {
            /// @todo setup parameter for export operation
            m_Data->m_Svnclient->doExport(svn::Path(fUrl),p,r,peg,force_recurse,QString::null,ignoreExternal,exp_rec);
        } else {
            m_Data->m_Svnclient->checkout(fUrl,p,r,peg,force_recurse,ignoreExternal);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    if (openIt) {
        if (!_exp) emit sigGotourl(tPath);
        else kapp->invokeBrowser(tPath);
    }
    EMIT_FINISHED;
    return true;
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

    bool checkboxres = true;

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
//        m_Data->m_Cache.dump_tree();
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
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"switch_url_dlg",false);
        delete dlg;
    }
    return done;
}

bool SvnActions::makeCleanup(const QString&path)
{
    if (!m_Data->m_CurrentContext) return false;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Cleanup"),i18n("Cleaning up folder"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->cleanup(svn::Path(path));
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
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

void SvnActions::slotMergeExternal(const QString&_src1,const QString&_src2, const QString&_target,
    const svn::Revision&rev1,const svn::Revision&rev2,bool rec)
{
    KTempDir tdir1;
    tdir1.setAutoDelete(true);
    QString src1 = _src1;
    QString src2 = _src2;
    QString target = _target;
    bool singleMerge = false;

    if (rev1 == rev2 && (src2.isEmpty() || src1==src2) ) {
        singleMerge = true;
    }
    if (src1.isEmpty()) {
        emit clientException(i18n("Nothing to merge."));
        return;
    }
    if (target.isEmpty()) {
        emit clientException(i18n("No destination to merge."));
        return;
    }

    KURL url(target);
    if (!url.isLocalFile()) {
        emit clientException(i18n("Target for merge must be local!"));
        return;
    }

    QFileInfo f1(src1);
    QFileInfo f2(src2);
    bool isDir = true;

    svn::InfoEntry i1,i2;

    if (!singleInfo(src1,rev1,i1)) {
        return;
    }
    isDir = i1.isDir();
    if (!singleMerge && src1 != src2) {
        if (!singleInfo(src2,rev2,i2)) {
            return;
        }
        if (i2.isDir()!=isDir) {
            emit clientException(i18n("Both sources must be same type!"));
            return;
        }
    }

    QFileInfo ti(target);

    if (ti.isDir()!=isDir) {
        emit clientException(i18n("Target for merge must same type like sources!"));
        return;
    }

    QString s1 = f1.fileName()+"-"+rev1.toString();
    QString s2 = f2.fileName()+"-"+rev2.toString();
    QString first,second,out;
    if (rev1 != svn::Revision::WORKING) {
        first = tdir1.name()+"/"+s1;
    } else {
        first = src1;
    }
    if (!singleMerge) {
        if (rev2!=svn::Revision::WORKING) {
            second = tdir1.name()+"/"+s2;
        } else {
            second = src2;
        }
    } else {
        // only two-way  merge
        second = QString::null;
    }
    if (second == first) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(),i18n("Both entries seems to be the same, won't do a merge."));
        return;
    }

    if (rev1 != svn::Revision::WORKING) {
        if (isDir) {
            if (!makeCheckout(src1,first,rev1,true,true,false,false,rec)) {
                return;
            }
        } else {
            if (!get(src1,first,rev1,svn::Revision::UNDEFINED,m_Data->m_ParentList->realWidget())) {
                return;
            }
        }
    }

    if (!singleMerge) {
        if (rev2!=svn::Revision::WORKING) {
            if (isDir) {
                if (!makeCheckout(src2,second,rev2,true,true,false,false,rec)) {
                    return;
                }
            } else {
                if (!get(src2,second,rev2,svn::Revision::UNDEFINED,m_Data->m_ParentList->realWidget())) {
                    return;
                }
            }
        }
    }
    QString edisp = Kdesvnsettings::external_merge_program();
    QStringList wlist = QStringList::split(" ",edisp);
    KProcess*proc = new KProcess();
    for ( QStringList::Iterator it = wlist.begin();it!=wlist.end();++it) {
        if (*it=="%s1") {
            *proc<<first;
        } else if (*it=="%s2") {
            if (!second.isEmpty()) *proc<<second;
        } else if (*it=="%t") {
            *proc<<target;
        } else {
            *proc << *it;
        }
    }
    connect(proc,SIGNAL(processExited(KProcess*)),this,SLOT(procClosed(KProcess*)));
    connect(proc,SIGNAL(receivedStderr(KProcess*,char*,int)),this,SLOT(receivedStderr(KProcess*,char*,int)));
    if (proc->start(m_Data->runblocked?KProcess::Block:KProcess::NotifyOnExit,KProcess::Stderr)) {
        if (!m_Data->runblocked) {
            tdir1.setAutoDelete(false);
            m_Data->m_tempdirlist[proc].append(tdir1.name());
        }
        return;
    } else {
        emit sendNotify(i18n("Merge-process could not started, check command."));
    }
    delete proc;
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
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Merge"),i18n("Merging items"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
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
bool SvnActions::makeMove(const QString&Old,const QString&New,bool force)
{
    if (!m_Data->m_CurrentContext) return false;
    kdDebug()<<"Force: "<<force << endl;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Move"),i18n("Moving/Rename item "));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->move(svn::Path(Old),svn::Path(New),force);
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    return true;
}

bool SvnActions::makeMove(const KURL::List&Old,const QString&New,bool force)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Move"),i18n("Moving entries"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        KURL::List::ConstIterator it = Old.begin();
        bool local = false;
        if ((*it).protocol().isEmpty()) {
            local = true;
        }
        for (;it!=Old.end();++it) {
            svn::Path NPath(New);
            NPath.addComponent((*it).fileName());
            m_Data->m_Svnclient->move((local?(*it).path():(*it).url()),NPath,force);
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

bool SvnActions::makeCopy(const QString&Old,const QString&New,const svn::Revision&rev)
{
    if (!m_Data->m_CurrentContext) return false;
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Copy / Move"),i18n("Copy or Moving entries"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        m_Data->m_Svnclient->copy(svn::Path(Old),rev,svn::Path(New));
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    return true;
}

bool SvnActions::makeCopy(const KURL::List&Old,const QString&New,const svn::Revision&rev)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContext,m_Data->m_ParentList->realWidget(),0,i18n("Copy / Move"),i18n("Copy or Moving entries"));
        connect(this,SIGNAL(sigExtraLogMsg(const QString&)),&sdlg,SLOT(slotExtraMessage(const QString&)));
        KURL::List::ConstIterator it = Old.begin();
        for (;it!=Old.end();++it) {
            m_Data->m_Svnclient->copy(svn::Path((*it). pathOrURL()),rev,svn::Path(New));
        }
    } catch (svn::ClientException e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
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
    for (unsigned int i = 0; i<what.count();++i) {
        m_Data->m_repoLockCache.deleteKey(*(what.at(i)),true);
    }
//    m_Data->m_repoLockCache.dump_tree();
}


/*!
    \fn SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist)
 */
bool SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where,bool rec,bool all)
{
    bool display_ignores = Kdesvnsettings::display_ignored_files();
    return makeStatus(what,dlist,where,rec,all,display_ignores);
}

bool SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where,bool rec,bool all,bool display_ignores,bool updates)
{
    bool disp_remote_details = Kdesvnsettings::details_on_remote_listing();
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
        if (!dlist[i]->isVersioned()) {
            rlist.append(dlist[i]);
            displist.append(dlist[i]->path());
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
        dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"add_items_dlg",false);
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
        svn::StatusPtr ptr = m_CThread->getList()[i];
        if (m_CThread->getList()[i]->isRealVersioned()&& (
            m_CThread->getList()[i]->textStatus()==svn_wc_status_modified||
            m_CThread->getList()[i]->textStatus()==svn_wc_status_added||
            m_CThread->getList()[i]->textStatus()==svn_wc_status_deleted||
            m_CThread->getList()[i]->textStatus()==svn_wc_status_replaced||
            m_CThread->getList()[i]->propStatus()==svn_wc_status_modified
         ) ) {
            m_Data->m_Cache.insertKey(ptr,ptr->path());
        } else if (m_CThread->getList()[i]->textStatus()==svn_wc_status_conflicted) {
            m_Data->m_conflictCache.insertKey(ptr,ptr->path());
        }
    }
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
        svn::StatusPtr ptr = m_UThread->getList()[i];
        if (ptr->validReposStatus()) {
            m_Data->m_UpdateCache.insertKey(ptr,ptr->path());
            ptr->textStatus();
            ptr->propStatus();
            if (!(ptr->validLocalStatus())) {
                newer = true;
            }
        }
        if (ptr->isLocked() &&
            !(ptr->entry().lockEntry().Locked())) {
            m_Data->m_repoLockCache.insertKey(ptr,ptr->path());
        }
    }
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

void SvnActions::addModifiedCache(const svn::StatusPtr&what)
{
    if (what->textStatus()==svn_wc_status_conflicted) {
        m_Data->m_conflictCache.insertKey(what,what->path());
    } else {
        m_Data->m_Cache.insertKey(what,what->path());
//        m_Data->m_Cache.dump_tree();
    }
}

void SvnActions::deleteFromModifiedCache(const QString&what)
{
    m_Data->m_Cache.deleteKey(what,true);
    m_Data->m_conflictCache.deleteKey(what,true);
    //m_Data->m_Cache.dump_tree();
}

bool SvnActions::checkModifiedCache(const QString&path)
{
    return m_Data->m_Cache.find(path);
}

bool SvnActions::checkReposLockCache(const QString&path)
{
    return m_Data->m_repoLockCache.findSingleValid(path,false);
}

bool SvnActions::checkReposLockCache(const QString&path,svn::SharedPointer<svn::Status>&t)
{
    /// @todo create a method where svn::Status* will be a parameter so no copy is needed but just reading content
    return m_Data->m_repoLockCache.findSingleValid(path,t);
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
    m_Data->m_repoLockCache.clear();
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
    svn::SharedPointer<svn::Status> d;
    return m_Data->m_UpdateCache.findSingleValid(path,d);
}

bool SvnActions::getUpdated(const QString&path,svn::SharedPointer<svn::Status>&d)const
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
        svn::PropertiesMap&mp = pm[0].second;
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

svn::PathPropertiesMapListPtr SvnActions::propList(const QString&which,const svn::Revision&where,bool cacheOnly)
{
    svn::PathPropertiesMapListPtr pm;
    if (!which.isEmpty()) {
        QString ex;
        svn::Path p(which);
        QString fk=where.toString()+"/"+which;

        if (where != svn::Revision::WORKING)
        {
            m_Data->m_PropertiesCache.findSingleValid(fk,pm);
        }
        if (!pm && !cacheOnly)
        {
            try {
                pm = m_Data->m_Svnclient->proplist(p,where,where);
            } catch (const svn::ClientException&e) {
                /* no messagebox needed */
                sendNotify(e.msg());
            }
            if (where != svn::Revision::WORKING && pm) {
                m_Data->m_PropertiesCache.insertKey(pm,fk);
            }
        }
    }
    return pm;
}

bool SvnActions::isLockNeeded(SvnItem*which,const svn::Revision&where)
{
    if (!which) return false;
    QString ex;
    svn::Path p(which->fullName());
    svn::PathPropertiesMapList pm;
    try {
        pm = m_Data->m_Svnclient->propget("svn:needs-lock",p,where,where);
    } catch (const svn::ClientException&e) {
        /* no messagebox needed */
        //emit clientException(e.msg());
        return false;
    }
    if (pm.size()>0) {
        svn::PropertiesMap&mp = pm[0].second;
        if (mp.find("svn:needs-lock")!=mp.end()) {
            return true;
        }
    }
    return false;
}

QString SvnActions::searchProperty(QString&Store, const QString&property, const QString&start,const svn::Revision&where,bool up)
{
    svn::Path pa(start);
    while(pa.length()>0) {
        svn::PathPropertiesMapListPtr pm = propList(pa,where,false);
        if (!pm) {
            return QString::null;
        }
        if (pm->size()>0) {
            svn::PropertiesMap&mp = (*pm)[0].second;
            if (mp.find(property)!=mp.end()) {
                Store=mp[property];
                return pa;
            }
        }
        if (up) {
            pa.removeLast();
        } else {
            return QString::null;
        }
    }
    return QString::null;
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
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Data->m_Svnclient->info(cleanpath,false,rev,peg);
    } catch (svn::ClientException e) {
        kdDebug()<< e.msg()<< " " << endl;
        if (SVN_ERR_WC_NOT_DIRECTORY==e.apr_err())
        {
            return false;
        }
        return true;
    }
    _baseUri=e[0].url();
    return true;
}

void SvnActions::slotExtraLogMsg(const QString&msg)
{
    emit sigExtraLogMsg(msg);
}

void SvnActions::slotCancel(bool how)
{
    if (!m_Data->m_CurrentContext) return;
    m_Data->m_SvnContext->setCanceled(how);
}

void SvnActions::setContextData(const QString&aKey,const QString&aValue)
{
    if (aValue.isNull()) {
        QMap<QString,QString>::iterator it = m_Data->m_contextData.find(aKey);
        if (it!=m_Data->m_contextData.end()) {
            m_Data->m_contextData.remove(it);
        }
    } else {
        m_Data->m_contextData[aKey]=aValue;
    }
}

void SvnActions::clearContextData()
{
    m_Data->m_contextData.clear();
}

QString SvnActions::getContextData(const QString&aKey)const
{
    if (m_Data->m_contextData.find(aKey)!=m_Data->m_contextData.end()) {
        return m_Data->m_contextData[aKey];
    }
    return QString::null;
}

#include "svnactions.moc"
