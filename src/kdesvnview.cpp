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


#include "kdesvnview.h"
#include "svnfrontend/kdesvnfilelist.h"
#include "svnfrontend/createrepo_impl.h"
#include "svnfrontend/dumprepo_impl.h"
#include "svnfrontend/stopdlg.h"
#include "src/settings/kdesvnsettings.h"
#include "svnqt/url.hpp"
#include "svnqt/repository.hpp"

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
    : QWidget(parent,name),m_Collection(aCollection),
      m_currentURL("")
{
    setupActions();
    QVBoxLayout *top_layout = new QVBoxLayout(this);
    m_Splitter = new QSplitter( this, "m_Splitter" );
    m_Splitter->setOrientation( QSplitter::Vertical );
    m_flist=new kdesvnfilelist(m_Collection,m_Splitter);

    m_flist->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 1, m_flist->sizePolicy().hasHeightForWidth() ) );
    m_LogWindow=new KTextBrowser(m_Splitter);
    top_layout->addWidget(m_Splitter);
    connect(m_flist,SIGNAL(sigLogMessage(const QString&)),this,SLOT(slotAppendLog(const QString&)));
    connect(m_flist,SIGNAL(changeCaption(const QString&)),this,SLOT(slotSetTitle(const QString&)));
    connect(m_flist,SIGNAL(sigShowPopup(const QString&,QWidget**)),this,SLOT(slotDispPopup(const QString&,QWidget**)));
    connect(m_flist,SIGNAL(sigUrlOpend(bool)),parent,SLOT(slotUrlOpened(bool)));
    connect(m_flist,SIGNAL(sigSwitchUrl(const KURL&)),this,SIGNAL(sigSwitchUrl(const KURL&)));
    connect(m_flist,SIGNAL(sigUrlChanged( const QString& )),this,SLOT(slotUrlChanged(const QString&)));
    connect(this,SIGNAL(sigMakeBaseDirs()),m_flist,SLOT(slotMkBaseDirs()));
}

void kdesvnView::slotAppendLog(const QString& text)
{
    m_LogWindow->append(text);
}

kdesvnView::~kdesvnView()
{
}

//void kdesvnView::print(QPainter *p, int height, int width)
void kdesvnView::print(QPainter *, int , int)
{
    // do the actual printing, here
    // p->drawText(etc..)
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
    Createrepo_impl*ptr = new Createrepo_impl(Dialog1Layout);
    dlg->resize(dlg->configDialogSize(*(Kdesvnsettings::self()->config()),"create_repo_size"));
    int i = dlg->exec();
    dlg->saveDialogSize(*(Kdesvnsettings::self()->config()),"create_repo_size",false);

    if (i!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::Repository*_rep = new svn::Repository(this);
    bool ok = true;
    bool createdirs;
    QString path = ptr->targetDir();
    closeMe();
    kdDebug()<<"Creating "<<path << endl;
    try {
        _rep->CreateOpen(path,ptr->fsType(),ptr->disableFsync(),
            !ptr->keepLogs(),false);
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
    svn::Repository*_rep = new svn::Repository(this);
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


