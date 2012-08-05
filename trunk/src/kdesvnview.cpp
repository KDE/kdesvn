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


#include "kdesvnview.h"
#include "svnfrontend/maintreewidget.h"
#include "svnfrontend/createrepo_impl.h"
#include "svnfrontend/dumprepo_impl.h"
#include "svnfrontend/hotcopydlg_impl.h"
#include "svnfrontend/loaddmpdlg_impl.h"
#include "svnfrontend/stopdlg.h"
#include "svnfrontend/fronthelpers/propertylist.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/url.h"
#include "src/svnqt/repository.h"
#include "src/svnqt/version_check.h"
#include "src/svnqt/svnqttypes.h"

#include <QPainter>
#include <QLayout>
#include <QFileInfo>
#include <QSplitter>

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
#include <kprogressdialog.h>
#include <kvbox.h>
#include <kio/netaccess.h>

kdesvnView::kdesvnView(KActionCollection*aCollection,QWidget *parent,bool full)
    : QWidget(parent),svn::repository::RepositoryListener(),m_Collection(aCollection),
      m_currentUrl("")
{
    Q_UNUSED(full);
    setFocusPolicy(Qt::StrongFocus);
    setupActions();
    m_CacheProgressBar=0;

    m_topLayout = new QVBoxLayout(this);

    m_Splitter = new QSplitter( this);
    m_Splitter->setOrientation( Qt::Vertical );

    //m_TreeWidget=new kdesvnfilelist(m_Collection,m_Splitter);
    m_TreeWidget = new MainTreeWidget(m_Collection,m_Splitter);

    m_infoSplitter = new QSplitter(m_Splitter);
    m_infoSplitter->setOrientation( Qt::Horizontal );
    m_infoSplitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_LogWindow=new KTextBrowser(m_infoSplitter);
    Propertylist*pl = new Propertylist(m_infoSplitter);
    pl->setCommitchanges(true);
    pl->addCallback(m_TreeWidget);
    connect(m_TreeWidget,SIGNAL(sigProplist(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&)),
            pl,SLOT(displayList(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&)));
    connect(m_TreeWidget,SIGNAL(sigProplist(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&)),
            pl,SLOT(displayList(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&)));

    m_TreeWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    m_topLayout->addWidget(m_Splitter);
    connect(m_TreeWidget,SIGNAL(sigLogMessage(const QString&)),this,SLOT(slotAppendLog(const QString&)));
    connect(m_TreeWidget,SIGNAL(changeCaption(const QString&)),this,SLOT(slotSetTitle(const QString&)));
    connect(m_TreeWidget,SIGNAL(sigShowPopup(const QString&,QWidget**)),this,SLOT(slotDispPopup(const QString&,QWidget**)));
    connect(m_TreeWidget,SIGNAL(sigUrlOpend(bool)),parent,SLOT(slotUrlOpened(bool)));
    connect(m_TreeWidget,SIGNAL(sigSwitchUrl(const KUrl&)),this,SIGNAL(sigSwitchUrl(const KUrl&)));
    connect(m_TreeWidget,SIGNAL(sigUrlChanged( const QString& )),this,SLOT(slotUrlChanged(const QString&)));
    connect(m_TreeWidget,SIGNAL(sigCacheStatus(qlonglong,qlonglong)),this,SLOT(fillCacheStatus(qlonglong,qlonglong)));
    connect(m_TreeWidget,SIGNAL(sigExtraStatusMessage(const QString&)),this,SIGNAL(sigExtraStatusMessage(const QString&)));
    
    connect(this,SIGNAL(sigMakeBaseDirs()),m_TreeWidget,SLOT(slotMkBaseDirs()));

    KConfigGroup cs(Kdesvnsettings::self()->config(),"kdesvn-mainlayout");
    QByteArray t1 = cs.readEntry("split1",QByteArray());
    if (!t1.isEmpty()) {
        m_Splitter->restoreState(t1);
    }
    if (m_infoSplitter) {
        t1 = cs.readEntry("infosplit",QByteArray());
        if (!t1.isEmpty()) {
            m_infoSplitter->restoreState(t1);
        }
    }
}

void kdesvnView::slotAppendLog(const QString& text)
{
    m_LogWindow->append(text);
}

kdesvnView::~kdesvnView()
{
}

void kdesvnView::slotSavestate()
{
    KConfigGroup cs(Kdesvnsettings::self()->config(),"kdesvn-mainlayout");
    cs.writeEntry("split1",m_Splitter->saveState());
    if (m_infoSplitter) {
        cs.writeEntry("infosplit",m_infoSplitter->saveState());
    }
}

void kdesvnView::slotUrlChanged(const QString&url)
{
    m_currentUrl=url;
    slotSetTitle(url);
    emit sigUrlChanged(url);
    slotOnURL(i18n("Repository opened"));
}

QString kdesvnView::currentUrl()
{
    return m_currentUrl;
}

bool kdesvnView::openUrl(QString url)
{
    return openUrl(KUrl(url));
}

bool kdesvnView::openUrl(const KUrl& url)
{
    /* transform of url must be done in part! otherwise we will run into different troubles! */
    m_currentUrl = "";
    KUrl _url;
    bool open = false;
    _url = url;
    if (_url.isLocalFile()) {
        QString query = _url.query();
        _url.setQuery("");
        QString _f = _url.path();
        QFileInfo f(_f);
        if (!f.isDir()) {
            m_currentUrl="";
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
    slotSetTitle(url.prettyUrl());
    if (m_TreeWidget->openUrl(url)) {
        slotOnURL(i18n("Repository opened"));
        m_currentUrl=url.url();
        open = true;
    } else {
        QString t = m_TreeWidget->lastError();
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
    m_TreeWidget->closeMe();
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
    m_TreeWidget->refreshCurrentTree();
}


/*!
    \fn kdesvnView::slotSettingsChanged()
 */
void kdesvnView::slotSettingsChanged()
{
    m_TreeWidget->slotSettingsChanged();
}

/*!
    \fn kdesvnView::slotCreateRepo()
 */
void kdesvnView::slotCreateRepo()
{
    KDialog * dlg = new KDialog(KApplication::activeModalWidget());
    if (!dlg) {
        return;
    }
    dlg->setObjectName("create_repository");
    dlg->setModal(true);
    dlg->setCaption(i18n("Create new repository"));
    dlg->setButtons(KDialog::Ok|KDialog::Cancel);

    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);
    //dlg->makeVBoxMainWidget();
    Createrepo_impl*ptr = new Createrepo_impl(Dialog1Layout);
    KConfigGroup _kc(Kdesvnsettings::self()->config(),"create_repo_size");
    dlg->restoreDialogSize(_kc);
    int i = dlg->exec();
    dlg->saveDialogSize(_kc,KConfigGroup::Normal);

    if (i!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::repository::Repository*_rep = new svn::repository::Repository(this);
    bool ok = true;
    bool createdirs;
    QString path = ptr->targetDir();
    closeMe();
    try {
        _rep->CreateOpen(ptr->parameter());
    } catch(const svn::ClientException&e) {
        slotAppendLog(e.msg());
        ok = false;
    }
    createdirs = ptr->createMain();
    delete dlg;
    delete _rep;
    if (!ok) {
        return;
    }
    openUrl(path);
    if (createdirs) {
        emit sigMakeBaseDirs();
    }
}

void kdesvnView::slotHotcopy()
{
    KDialog* dlg = new KDialog(KApplication::activeModalWidget());
    if (!dlg) return;
    dlg->setObjectName("hotcopy_repository");
    dlg->setModal(true);
    dlg->setCaption(i18n("Hotcopy a repository"));
    dlg->setButtons(KDialog::Ok|KDialog::Cancel);

    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);
    HotcopyDlg_impl * ptr = new HotcopyDlg_impl(Dialog1Layout);
    KConfigGroup _kc(Kdesvnsettings::self()->config(),"hotcopy_repo_size");
    dlg->restoreDialogSize(_kc);
    int i = dlg->exec();
    dlg->saveDialogSize(_kc,KConfigGroup::Normal);

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
    } catch(const svn::ClientException&e) {
        slotAppendLog(e.msg());
    }
}

void kdesvnView::slotLoaddump()
{
    KDialog dlg(KApplication::activeModalWidget());
    dlg.setObjectName("hotcopy_repository");
    dlg.setModal(true);
    dlg.setCaption(i18n("Hotcopy a repository"));
    dlg.setButtons(KDialog::Ok|KDialog::Cancel);
    QWidget* Dialog1Layout = new KVBox(&dlg);
    dlg.setMainWidget(Dialog1Layout);

    LoadDmpDlg_impl * ptr = new LoadDmpDlg_impl(Dialog1Layout);

    KConfigGroup _kc(Kdesvnsettings::self()->config(),"loaddump_repo_size");
    dlg.restoreDialogSize(_kc);
    int i = dlg.exec();

    dlg.saveDialogSize(_kc,KConfigGroup::Normal);
    if (i!=QDialog::Accepted) {
        return;
    }
    svn::repository::Repository _rep(this);
    m_ReposCancel = false;

    try {
        _rep.Open(ptr->repository());
    }catch (const svn::ClientException&e) {
        slotAppendLog(e.msg());
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

    KUrl _uri=ptr->dumpFile();
    QString _input;
    QString tmpfile;
    bool networked = false;
    if (_uri.isLocalFile()) {
        _input = _uri.path();
    } else {
        networked = true;
        if(! KIO::NetAccess::download(_uri, tmpfile, this) ) {
            KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
            KIO::NetAccess::removeTempFile(tmpfile);
            return;
        }
        _input=tmpfile;
    }

    try {
        StopDlg sdlg(this,this,0,"Load Dump",i18n("Loading a dump into a repository."));
        _rep.loaddump(_input,_act,ptr->parentPath(),ptr->usePre(),ptr->usePost(),ptr->validateProps());
        slotAppendLog(i18n("Loading dump finished."));
    }catch (const svn::ClientException&e) {
        slotAppendLog(e.msg());
    }
    if (networked && tmpfile.length()>0) {
        KIO::NetAccess::removeTempFile(tmpfile);
    }
}

void kdesvnView::slotDumpRepo()
{
    KDialog * dlg = new KDialog(KApplication::activeModalWidget());
    if (!dlg) return;
    dlg->setObjectName("dump_repository");
    dlg->setModal(true);
    dlg->setCaption(i18n("Dump a repository"));
    dlg->setButtons(KDialog::Ok|KDialog::Cancel);
    QWidget* Dialog1Layout = new KVBox(dlg);
    dlg->setMainWidget(Dialog1Layout);

    DumpRepo_impl*ptr = new DumpRepo_impl(Dialog1Layout);
    KConfigGroup _kc(Kdesvnsettings::self()->config(),"dump_repo_size");
    dlg->restoreDialogSize(_kc);
    int i = dlg->exec();
    dlg->saveDialogSize(_kc,KConfigGroup::Normal);

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
    svn::Revision st = svn::Revision::UNDEFINED;
    svn::Revision en = svn::Revision::UNDEFINED;

    if (s>-1) {
        st=s;
    }
    if (e>-1) {
        en=e;
    }

    try {
        _rep->Open(re);
    }catch (const svn::ClientException&e) {
        slotAppendLog(e.msg());
        delete _rep;
        return ;
    }

    try {
        StopDlg sdlg(this,this,0,"Dump",i18n("Dumping a repository"));
        _rep->dump(out,st,en,incr,diffs);
        slotAppendLog(i18n("Dump finished."));
    }catch (const svn::ClientException&e) {
        slotAppendLog(e.msg());
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

void kdesvnView::fillCacheStatus(qlonglong current,qlonglong max)
{
    if (current>-1 && max>-1) {
        if (!m_CacheProgressBar) {
            m_CacheProgressBar=new QProgressBar(this);
            m_CacheProgressBar->setRange(0,(int)max);
            m_topLayout->addWidget(m_CacheProgressBar);
            m_CacheProgressBar->setFormat(i18n("Inserted %v not cached log entries of %m."));
        }
        if (!m_CacheProgressBar->isVisible()) {
            m_CacheProgressBar->show();
        }
        m_CacheProgressBar->setValue((int)current);
    } else {
        delete m_CacheProgressBar;
        m_CacheProgressBar=0;
    }
}

void kdesvnView::stopCacheThreads()
{
    m_TreeWidget->stopLogCache();
}

#include "kdesvnview.moc"
