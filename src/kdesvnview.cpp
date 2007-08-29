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


#include "kdesvnview.h"
#include "svnfrontend/kdesvnfilelist.h"
#include "svnfrontend/createrepo_impl.h"
#include "svnfrontend/dumprepo_impl.h"
#include "svnfrontend/hotcopydlg_impl.h"
#include "svnfrontend/loaddmpdlg_impl.h"
#include "svnfrontend/stopdlg.h"
#include "svnfrontend/leftpane.h"
#include "svnfrontend/fronthelpers/propertylist.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/url.hpp"
#include "src/svnqt/repository.hpp"
#include "src/svnqt/version_check.hpp"
#include "src/svnqt/svnqttypes.hpp"

#include <qpainter.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qvbox.h>

#include <kurl.h>
#include <ktrader.h>
#include <kapplication.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <klocale.h>
#include <ktextbrowser.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kdialog.h>
#include <kdialogbase.h>

kdesvnView::kdesvnView(KActionCollection*aCollection,QWidget *parent,const char*name)
    : QWidget(parent,name),svn::repository::RepositoryListener(),m_Collection(aCollection),
      m_currentURL("")
{
    setupActions();
    QVBoxLayout *top_layout = new QVBoxLayout(this);

    m_Splitter = new QSplitter( this, "m_Splitter" );
    m_Splitter->setOrientation( QSplitter::Vertical );

#if 1

#if 0
    m_treeSplitter = new QSplitter(m_Splitter);
    m_treeSplitter->setOrientation( QSplitter::Horizontal );
    m_treeSplitter->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 1, m_treeSplitter->sizePolicy().hasHeightForWidth() ) );

    // just for testing - get filled with a real widget
    leftpane * _leftpane = new leftpane(m_treeSplitter);
    m_flist=new kdesvnfilelist(m_Collection,m_treeSplitter);
#else
    m_treeSplitter=0;
    m_flist=new kdesvnfilelist(m_Collection,m_Splitter);
#endif

    m_infoSplitter = new QSplitter(m_Splitter);
    m_infoSplitter->setOrientation( QSplitter::Horizontal );
    m_infoSplitter->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 1, m_infoSplitter->sizePolicy().hasHeightForWidth() ) );
    m_LogWindow=new KTextBrowser(m_infoSplitter);
    Propertylist*pl = new Propertylist(m_infoSplitter);
    pl->setCommitchanges(true);
    pl->addCallback(m_flist);
    connect(m_flist,SIGNAL(sigProplist(const svn::PathPropertiesMapListPtr&,bool,const QString&)),
            pl,SLOT(displayList(const svn::PathPropertiesMapListPtr&,bool,const QString&)));
#else
    m_treeSplitter=0;
    m_infoSplitter=0;
    m_flist=new kdesvnfilelist(m_Collection,m_Splitter);
    m_LogWindow=new KTextBrowser(m_Splitter);
#endif
    m_flist->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 1, m_flist->sizePolicy().hasHeightForWidth() ) );
#if 0
    m_treeSplitter->setCollapsible(m_flist,false);
    m_Splitter->setCollapsible(m_treeSplitter,false);
#endif

    top_layout->addWidget(m_Splitter);
    connect(m_flist,SIGNAL(sigLogMessage(const QString&)),this,SLOT(slotAppendLog(const QString&)));
    connect(m_flist,SIGNAL(changeCaption(const QString&)),this,SLOT(slotSetTitle(const QString&)));
    connect(m_flist,SIGNAL(sigShowPopup(const QString&,QWidget**)),this,SLOT(slotDispPopup(const QString&,QWidget**)));
    connect(m_flist,SIGNAL(sigUrlOpend(bool)),parent,SLOT(slotUrlOpened(bool)));
    connect(m_flist,SIGNAL(sigSwitchUrl(const KURL&)),this,SIGNAL(sigSwitchUrl(const KURL&)));
    connect(m_flist,SIGNAL(sigUrlChanged( const QString& )),this,SLOT(slotUrlChanged(const QString&)));
    connect(this,SIGNAL(sigMakeBaseDirs()),m_flist,SLOT(slotMkBaseDirs()));
    KConfigGroup cs(Kdesvnsettings::self()->config(),"kdesvn-mainlayout");
    QString t1;
    t1 = cs.readEntry("split1",QString::null);
    if (!t1.isEmpty()) {
        QTextStream st1(&t1,IO_ReadOnly);
        st1 >> *m_Splitter;
    }
    if (m_treeSplitter) {
        t1 = cs.readEntry("split2",QString::null);
        if (!t1.isEmpty()) {
            QTextStream st2(&t1,IO_ReadOnly);
            st2 >> *m_treeSplitter;
        }
    }
    if (m_infoSplitter) {
        t1 = cs.readEntry("infosplit",QString::null);
        if (!t1.isEmpty()) {
            QTextStream st2(&t1,IO_ReadOnly);
            st2 >> *m_infoSplitter;
        }
    }
}

void kdesvnView::slotAppendLog(const QString& text)
{
    m_LogWindow->append(text);
}

kdesvnView::~kdesvnView()
{
    KConfigGroup cs(Kdesvnsettings::self()->config(),"kdesvn-mainlayout");
    QString t1,t2;
    QTextStream st1(&t1,IO_WriteOnly);
    st1 << *m_Splitter;
    cs.writeEntry("split1",t1);

    if (m_treeSplitter) {
        QTextStream st2(&t2,IO_WriteOnly);
        st2 << *m_treeSplitter;
        cs.writeEntry("split2",t2);
    }
    if (m_infoSplitter) {
        t2="";
        QTextStream st2(&t2,IO_WriteOnly);
        st2 << *m_infoSplitter;
        cs.writeEntry("infosplit",t2);
    }
}

void kdesvnView::slotUrlChanged(const QString&url)
{
    m_currentURL=url;
    slotSetTitle(url);
    emit sigUrlChanged(url);
    slotOnURL(i18n("Repository opened"));
}

QString kdesvnView::currentURL()
{
    return m_currentURL;
}

bool kdesvnView::openURL(QString url)
{
    return openURL(KURL(url));
}

bool kdesvnView::openURL(const KURL& url)
{
    /* transform of url must be done in part! otherwise we will run into different troubles! */
    m_currentURL = "";
    KURL _url;
    bool open = false;
    _url = url;
    if (_url.isLocalFile()) {
        QString query = _url.query();
        _url.setQuery("");
        QString _f = _url.path();
        QFileInfo f(_f);
        if (!f.isDir()) {
            m_currentURL="";
            return open;
        }
        if (query.length()>1) {
            _url.setQuery(query);
        }
    } else {
        if (!svn::Url::isValid(url.protocol())) {
            return open;
        }
    }
    m_LogWindow->setText("");
    slotSetTitle(url.prettyURL());
    if (m_flist->openURL(url)) {
        slotOnURL(i18n("Repository opened"));
        m_currentURL=url.url();
        open = true;
    } else {
        QString t = m_flist->lastError();
        if (t.isEmpty()) {
            t = i18n("Could not open repository");
        }
        slotOnURL(t);
    }
    return open;
}

void kdesvnView::slotOnURL(const QString& url)
{
    emit signalChangeStatusbar(url);
}

void kdesvnView::slotSetTitle(const QString& title)
{
    //emit signalChangeCaption(title);
    emit setWindowCaption(title);
}


/*!
    \fn kdesvnView::closeMe()
 */
void kdesvnView::closeMe()
{
    m_flist->closeMe();
    m_LogWindow->setText("");
    slotOnURL(i18n("No repository open"));
}

void kdesvnView::slotDispPopup(const QString&item,QWidget**target)
{
    emit sigShowPopup(item,target);
}


/*!
    \fn kdesvnView::refreshCurrentTree()
 */
void kdesvnView::refreshCurrentTree()
{
    m_flist->refreshCurrentTree();
}


/*!
    \fn kdesvnView::slotSettingsChanged()
 */
void kdesvnView::slotSettingsChanged()
{
    m_flist->slotSettingsChanged();
}

/*!
    \fn kdesvnView::slotCreateRepo()
 */
void kdesvnView::slotCreateRepo()
{
    KDialogBase * dlg = new KDialogBase(
        KApplication::activeModalWidget(),
        "create_repository",
        true,
        i18n("Create new repository"),
        KDialogBase::Ok|KDialogBase::Cancel);
    if (!dlg) return;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    bool compatneeded = svn::Version::version_major()>1||svn::Version::version_minor()>3;
    Createrepo_impl*ptr = new Createrepo_impl(compatneeded,Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"create_repo_size"));
    int i = dlg->exec();
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"create_repo_size",false);

    if (i!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::repository::Repository*_rep = new svn::repository::Repository(this);
    bool ok = true;
    bool createdirs;
    QString path = ptr->targetDir();
    closeMe();
    kdDebug()<<"Creating "<<path << endl;
    try {
        _rep->CreateOpen(path,ptr->fsType(),ptr->disableFsync(),
            !ptr->keepLogs(),ptr->compat13());
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Creating "<<path << " failed "<<e.msg() << endl;
        ok = false;
    }
    kdDebug()<<"Creating "<<path << " done " << endl;
    createdirs = ptr->createMain();
    delete dlg;
    delete _rep;
    if (!ok) {
        return;
    }
    openURL(path);
    if (createdirs) {
        emit sigMakeBaseDirs();
    }
}

void kdesvnView::slotHotcopy()
{
    KDialogBase * dlg = new KDialogBase(
        KApplication::activeModalWidget(),
        "hotcopy_repository",
        true,
        i18n("Hotcopy a repository"),
        KDialogBase::Ok|KDialogBase::Cancel);
    if (!dlg) return;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    HotcopyDlg_impl * ptr = new HotcopyDlg_impl(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"hotcopy_repo_size"));
    int i = dlg->exec();
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"hotcopy_repo_size",false);

    if (i!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    bool cleanlogs = ptr->cleanLogs();
    QString src = ptr->srcPath();
    QString dest = ptr->destPath();
    delete dlg;
    if (src.isEmpty()||dest.isEmpty()) {
        return;
    }
    try {
        svn::repository::Repository::hotcopy( src,dest,cleanlogs);
        slotAppendLog(i18n("Hotcopy finished."));
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Hotcopy of "<< src << " failed "<<e.msg() << endl;
    }
}

void kdesvnView::slotLoaddump()
{
    KDialogBase dlg(
        KApplication::activeModalWidget(),
        "hotcopy_repository",
        true,
        i18n("Hotcopy a repository"),
        KDialogBase::Ok|KDialogBase::Cancel);
    QWidget* Dialog1Layout = dlg.makeVBoxMainWidget();
    LoadDmpDlg_impl * ptr = new LoadDmpDlg_impl(Dialog1Layout);
    dlg.resize(dlg.configDialogSize(*(Kdesvnsettings::self()->config()),"loaddump_repo_size"));
    int i = dlg.exec();
    dlg.saveDialogSize(*(Kdesvnsettings::self()->config()),"loaddump_repo_size",false);
    if (i!=QDialog::Accepted) {
        return;
    }
    svn::repository::Repository _rep(this);
    m_ReposCancel = false;

    try {
        _rep.Open(ptr->repository());
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Open "<<ptr->repository() << " failed "<<e.msg() << endl;
        return ;
    }

    svn::repository::Repository::LOAD_UUID _act;
    switch (ptr->uuidAction()) {
    case 1:
        _act = svn::repository::Repository::UUID_IGNORE_ACTION;
        break;
    case 2:
        _act = svn::repository::Repository::UUID_FORCE_ACTION;
        break;
    case 0:
    default:
        _act = svn::repository::Repository::UUID_DEFAULT_ACTION;
        break;
    }
    try {
        StopDlg sdlg(this,this,0,"Load Dump",i18n("Loading a dump into a repository."));
        _rep.loaddump(ptr->dumpFile(),_act,ptr->parentPath(),ptr->usePre(),ptr->usePost());
        slotAppendLog(i18n("Loading dump finished."));
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Load dump into "<<ptr->repository() << " failed "<<e.msg() << endl;
    }
}

void kdesvnView::slotDumpRepo()
{
    KDialogBase * dlg = new KDialogBase(
        KApplication::activeModalWidget(),
        "dump_repository",
        true,
        i18n("Dump a repository"),
        KDialogBase::Ok|KDialogBase::Cancel);
    if (!dlg) return;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    DumpRepo_impl*ptr = new DumpRepo_impl(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"dump_repo_size"));
    int i = dlg->exec();
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"dump_repo_size",false);

    if (i!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::repository::Repository*_rep = new svn::repository::Repository(this);
    QString re,out;
    bool incr,diffs;
    re = ptr->reposPath();
    out = ptr->targetFile();
    incr = ptr->incremental();
    diffs = ptr->use_deltas();
    int s = ptr->startNumber();
    int e = ptr->endNumber();

    delete dlg;

    m_ReposCancel = false;

    try {
        _rep->Open(re);
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Open "<<re << " failed "<<e.msg() << endl;
        delete _rep;
        return ;
    }

    try {
        StopDlg sdlg(this,this,0,"Dump",i18n("Dumping a repository"));
        _rep->dump(out,s,e,incr,diffs);
        slotAppendLog(i18n("Dump finished."));
    } catch(svn::ClientException e) {
        slotAppendLog(e.msg());
        kdDebug()<<"Dump "<<out << " failed "<<e.msg() << endl;
    }
    delete _rep;
}

/*!
    \fn kdesvnView::setupActions()
 */
void kdesvnView::setupActions()
{
}

void kdesvnView::sendWarning(const QString&aMsg)
{
    slotAppendLog(aMsg);
}

void kdesvnView::sendError(const QString&aMsg)
{
    slotAppendLog(aMsg);
}

bool kdesvnView::isCanceld()
{
    if (!m_ReposCancel) {
        emit tickProgress();
        return false;
    }
    return true;
}

void kdesvnView::setCanceled(bool how)
{
    m_ReposCancel = how;
}

#include "kdesvnview.moc"


