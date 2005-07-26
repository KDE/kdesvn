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
#include "kdesvnfilelist.h"
#include "filelistviewitem.h"
#include "importdir_logmsg.h"
#include "svnactions.h"
#include "svncpp/revision.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"
#include "helpers/dirnotify.h"
#include <kdirlister.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <kdirselectdialog.h>
#include <klineedit.h>
#include <qlayout.h>
#include <kfileitem.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <qvbox.h>

kdesvnfilelist::kdesvnfilelist(QWidget *parent, const char *name)
 : KListView(parent, name),m_SvnWrapper(new SvnActions(this))
{
    m_SelectedItems = 0;
    m_baseUri="";

    setMultiSelection(true);
    setSelectionModeExt(FileManager);
    setShowSortIndicator(true);
    setAllColumnsShowFocus (true);
    setDragEnabled(true);
    setItemsMovable(false);
    setRootIsDecorated(true);
    addColumn(i18n("Name"));
    addColumn(i18n("Status"));
    addColumn(i18n("Last changed Revision"));
    addColumn(i18n("Last author"));
    addColumn(i18n("Last change date"));
    addColumn(i18n("Locked by"));
    setSortColumn(FileListViewItem::COL_NAME);
    setupActions();

    connect(this,SIGNAL(clicked(QListViewItem*)),this,SLOT(slotItemClicked(QListViewItem*)));
    connect(this,SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),this,
        SLOT(slotRightButton(QListViewItem *, const QPoint &, int)));
    connect(this,SIGNAL(doubleClicked(QListViewItem*)),this,SLOT(slotItemDoubleClicked(QListViewItem*)));
    connect(this,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_SvnWrapper,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(m_SvnWrapper,SIGNAL(dirAdded(const QString&,FileListViewItem*)),this,
        SLOT(slotDirAdded(const QString&,FileListViewItem*)));
    connect(m_SvnWrapper,SIGNAL(reinitItem(FileListViewItem*)),this,SLOT(slotReinitItem(FileListViewItem*)));
    connect(m_SvnWrapper,SIGNAL(sigRefreshAll()),this,SLOT(refreshCurrentTree()));
    connect(m_SvnWrapper,SIGNAL(sigRefreshCurrent(FileListViewItem*)),this,SLOT(refreshCurrent(FileListViewItem*)));
    m_DirNotify = new DirNotify();
    setDropHighlighter(true);
    setDragEnabled(true);
}

svn::Client*kdesvnfilelist::svnclient()
{
    return m_SvnWrapper->svnclient();
}

void kdesvnfilelist::setupActions()
{
    m_filesAction = new KActionCollection(this);

    /* local and remote actions */
    /* 1. actions on dirs AND files */
    m_LogRangeAction = new KAction(i18n("&Log..."),"history",KShortcut(),m_SvnWrapper,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    m_LogFullAction = new KAction(i18n("&Full Log"),"history",KShortcut(),m_SvnWrapper,SLOT(slotMakeLog()),m_filesAction,"make_svn_log_full");
    m_propertyAction = new KAction(i18n("Properties"),"edit",KShortcut(),m_SvnWrapper,SLOT(slotProperties()),m_filesAction,"make_svn_property");
    m_InfoAction = new KAction(i18n("Details"),"vcs_status",
        KShortcut(),m_SvnWrapper,SLOT(slotInfo()),m_filesAction,"make_svn_info");

    /* 2. actions only on files */
    m_BlameAction = new KAction("&Blame","flag",KShortcut(),m_SvnWrapper,SLOT(slotBlame()),m_filesAction,"make_svn_blame");
    m_BlameRangeAction = new KAction("Blame range","flag",KShortcut(),m_SvnWrapper,SLOT(slotRangeBlame()),m_filesAction,"make_svn_range_blame");
    m_CatAction = new KAction("&Cat head","contents",KShortcut(),m_SvnWrapper,SLOT(slotCat()),m_filesAction,"make_svn_cat");

    /* 3. actions only on dirs */
    m_MkdirAction = new KAction("Make (sub-)directory","folder_new",
        KShortcut(),m_SvnWrapper,SLOT(slotMkdir()),m_filesAction,"make_svn_mkdir");
    m_switchRepository = new KAction("Switch repository","goto",KShortcut(),
        m_SvnWrapper,SLOT(slotSwitch()),m_filesAction,"make_svn_switch");
    m_switchRepository->setToolTip(i18n("Switch repository of working copy (\"svn switch\")"));

    m_changeToRepository = new KAction("Switch to repository","gohome",KShortcut(),
        this,SLOT(slotChangeToRepository()),m_filesAction,"make_switch_to_repo");
    m_changeToRepository->setToolTip(i18n("Switch to repository of current working copy"));

    m_CleanupAction = new KAction("Cleanup",KShortcut(),
        this,SLOT(slotCleanupAction()),m_filesAction,"make_cleanup");
    m_ImportDirsIntoCurrent  = new KAction(i18n("Import directories into current"),"fileimport",KShortcut(),
        this,SLOT(slotImportDirsIntoCurrent()),m_filesAction,"make_import_dirs_into_current");
    m_ImportDirsIntoCurrent->setToolTip(i18n("Import directory content into current url"));

    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent = new KAction("Add selected files/dirs","vcs_add",KShortcut(),m_SvnWrapper,SLOT(slotAdd()),m_filesAction,"make_svn_add");
    m_AddCurrent->setToolTip(i18n("Adding selected files and/or directories to repository"));
    m_DelCurrent = new KAction("Delete selected files/dirs","vcs_remove",
        KShortcut(),m_SvnWrapper,SLOT(slotDelete()),m_filesAction,"make_svn_remove");
    m_DelCurrent->setToolTip(i18n("Deleting selected files and/or directories from repository"));
    m_RevertAction  = new KAction("Revert current changes","revert",
        KShortcut(),m_SvnWrapper,SLOT(slotRevert()),m_filesAction,"make_svn_revert");
    m_ResolvedAction = new KAction("Resolve recursive",KShortcut(),
        this,SLOT(slotResolved()),m_filesAction,"make_resolved");
    m_ResolvedAction->setToolTip(i18n("Marking files or dirs resolved"));

    m_UpdateHead = new KAction("Update to head","vcs_update",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateHeadRec()),m_filesAction,"make_svn_headupdate");
    m_UpdateRev = new KAction("Update to revision...","vcs_update",
        KShortcut(),m_SvnWrapper,SLOT(slotUpdateTo()),m_filesAction,"make_svn_revupdate");
    m_commitAction = new KAction("Commit","vcs_commit",
        KShortcut(),m_SvnWrapper,SLOT(slotCommit()),m_filesAction,"make_svn_commit");
    m_simpleDiffHead = new KAction("Diff against head","vcs_diff",
        KShortcut(),m_SvnWrapper,SLOT(slotSimpleDiff()),m_filesAction,"make_svn_headdiff");

    /* remote actions only */
    m_CheckoutCurrentAction = new KAction("Checkout current repository path",KShortcut(),
        m_SvnWrapper,SLOT(slotCheckoutCurrent()),m_filesAction,"make_svn_checkout_current");
    m_ExportCurrentAction = new KAction(i18n("Export current repository path"),"fileexport",KShortcut(),
        m_SvnWrapper,SLOT(slotExportCurrent()),m_filesAction,"make_svn_export_current");

    /* independe actions */
    m_CheckoutAction = new KAction(i18n("Checkout a repository"),"bottom",
        KShortcut(),m_SvnWrapper,SLOT(slotCheckout()),m_filesAction,"make_svn_checkout");
    m_ExportAction = new KAction(i18n("Export a repository"),"fileexport",
        KShortcut(),m_SvnWrapper,SLOT(slotExport()),m_filesAction,"make_svn_export");
    m_RefreshViewAction = new KAction(i18n("Refresh view"),"reload",KShortcut(),this,SLOT(refreshCurrentTree()),m_filesAction,"make_view_refresh");

    enableActions();
    m_filesAction->setHighlightingEnabled(true);
}

KActionCollection*kdesvnfilelist::filesActions()
{
    return m_filesAction;
}

FileListViewItemList* kdesvnfilelist::allSelected()
{
    if (!m_SelectedItems) {
        m_SelectedItems = new FileListViewItemList;
    }
    return m_SelectedItems;
}

FileListViewItem* kdesvnfilelist::singleSelected()
{
    if (m_SelectedItems && m_SelectedItems->count()==1) {
        return m_SelectedItems->at(0);
    }
    return 0;
}

bool kdesvnfilelist::openURL( const KURL &url,bool noReinit )
{
    clear();
    m_Dirsread.clear();
    if (m_SelectedItems) {
        m_SelectedItems->clear();
    }
    m_LastException="";
    if (!noReinit) m_SvnWrapper->reInitClient();
    m_baseUri = url.url();
    m_isLocal = false;
    if (url.isLocalFile()) {
        m_baseUri = url.path();
        m_isLocal = true;
    }
    while (m_baseUri.endsWith("/")) {
        m_baseUri.truncate(m_baseUri.length()-1);
    }
#if 0
    if (m_isLocal) {
        m_DirNotify->setBase(m_baseUri);
    } else {
        m_DirNotify->stop();
    }
#endif
    bool result = checkDirs(m_baseUri,0);
    if (result && m_isLocal) {
        if (firstChild()) firstChild()->setOpen(true);
    }
    m_MkdirAction->setEnabled(result);
    m_InfoAction->setEnabled(m_isLocal);
    m_commitAction->setEnabled(m_isLocal);
    m_simpleDiffHead->setEnabled(m_isLocal);
    m_UpdateHead->setEnabled(m_isLocal);
    m_UpdateRev->setEnabled(m_isLocal);
    m_AddCurrent->setEnabled(m_isLocal);
    m_RevertAction->setEnabled(m_isLocal);
    m_changeToRepository->setEnabled(m_isLocal);
    m_DelCurrent->setEnabled(true);
    m_CheckoutCurrentAction->setEnabled(!m_isLocal);
    m_switchRepository->setEnabled(m_isLocal);

    if (result) enableActions();

    if (result) {
        emit changeCaption(m_baseUri);
    } else {
        emit changeCaption(i18n("No repository opend"));
    }
    return result;
}

bool kdesvnfilelist::checkDirs(const QString&_what,FileListViewItem * _parent)
{
    QString what = _what;
    svn::StatusEntries dlist;
    while (what.endsWith("/")) {
        what.truncate(what.length()-1);
    }

    try {
        /* settings about unknown and ignored files must be setable */
        //                                                          rec   all  up    noign
        dlist = m_SvnWrapper->svnclient()->status(what.local8Bit(),false,true,false,true);
    } catch (svn::ClientException e) {
        //Message box!
        m_LastException = QString::fromLocal8Bit(e.message());
        emit sigLogMessage(m_LastException);
        return false;
    } catch (...) {
        kdDebug()<<"Other exception"<<endl;
        return false;
    }
    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem * pitem = 0;
    bool main_found = false;
    for (;it!=dlist.end();++it) {
        if (it->path()==what||QString::compare(it->entry().url(),what)==0){
            if (!_parent) {
                pitem = new FileListViewItem(this,*it);
                m_Dirsread[pitem->fullName()]=true;
            }
            dlist.erase(it);
            main_found = true;
            break;
        }
    }
    if (_parent) {
        pitem = _parent;
    }
    insertDirs(pitem,dlist);
    return true;
}

void kdesvnfilelist::insertDirs(FileListViewItem * _parent,svn::StatusEntries&dlist)
{
    svn::StatusEntries::iterator it;
    for (it = dlist.begin();it!=dlist.end();++it) {
        FileListViewItem * item;
        if (!_parent) {
            item = new FileListViewItem(this,*it);
        } else {
            item = new FileListViewItem(this,_parent,*it);
        }
        if (item->isDir()) {
            m_Dirsread[item->fullName()]=false;
        }
    }
}

/* newdir is the NEW directory! just required if local */
void kdesvnfilelist::slotDirAdded(const QString&newdir,FileListViewItem*k)
{
    if (k) {
        k->refreshStatus();
    }
    if (!m_isLocal) {
        if (k) {
            k->removeChilds();
            m_Dirsread[k->fullName()]=false;
            slotItemClicked(k);
            return;
        }
        QListViewItem*temp;
        while ((temp=firstChild())) {
            delete temp;
        }
        m_Dirsread.clear();
        checkDirs(m_baseUri,0);
        return;
    }
    svn::Status stat;
    try {
        stat = m_SvnWrapper->svnclient()->singleStatus(newdir.local8Bit());
    } catch (svn::ClientException e) {
        m_LastException = QString::fromLocal8Bit(e.message());
        emit sigLogMessage(m_LastException);
        return;
    }
    FileListViewItem * item,*pitem;
    pitem = k;
    if (!pitem) {
        pitem = (FileListViewItem*)firstChild();
        if (pitem->fullName()!=m_baseUri) {
            pitem = 0;
        }
    }
    if (!pitem) {
        item = new FileListViewItem(this,stat);
    } else {
        item = new FileListViewItem(this,pitem,stat);
    }
    if (item->isDir()) {
        m_Dirsread[item->fullName()]=false;
    }
}

kdesvnfilelist::~kdesvnfilelist()
{
}

void kdesvnfilelist::slotItemClicked(QListViewItem*aItem)
{
    if (!aItem) return;
    FileListViewItem* k = static_cast<FileListViewItem*>( aItem );
    if (k->isDir()&&(m_Dirsread.find(k->fullName())==m_Dirsread.end()||m_Dirsread[k->fullName()]==false)) {
        if (checkDirs(k->fullName(),k)) {
            m_Dirsread[k->fullName()]=true;
        }
    }
}

void kdesvnfilelist::slotReinitItem(FileListViewItem*k)
{
    if (!k) return;
    if (k->isDir()) {
        k->removeChilds();
        m_Dirsread[k->fullName()]=false;;
    }
    slotItemClicked(k);
}

void kdesvnfilelist::enableActions()
{
    bool isopen = m_baseUri.length()>0;
    bool single = allSelected()->count()==1&&isopen;
    bool multi = allSelected()->count()>1&&isopen;
    bool none = allSelected()->count()==0&&isopen;
    bool dir = false;
    if (single && allSelected()->at(0)->isDir()) {
        dir = true;
    }
    /* local and remote actions */
    /* 1. actions on dirs AND files */
    m_LogRangeAction->setEnabled(single||(isLocal()&&!single&&!multi&&isopen));
    m_LogFullAction->setEnabled(single||(isLocal()&&!single&&!multi&&isopen));
    m_propertyAction->setEnabled(single);
    m_InfoAction->setEnabled(single&&m_isLocal);
    /* 2. only on files */
    m_BlameAction->setEnabled(single&&!dir);
    m_BlameRangeAction->setEnabled(single&&!dir);
    m_CatAction->setEnabled(single&&!dir);
    /* 3. actions only on dirs */
    m_MkdirAction->setEnabled(dir);
    m_switchRepository->setEnabled(dir);
    m_changeToRepository->setEnabled(isLocal());
    m_CleanupAction->setEnabled(isLocal()&&dir);
    m_ImportDirsIntoCurrent->setEnabled(isLocal()&&dir);
    /* local only actions */
    /* 1. actions on files AND dirs*/
    m_AddCurrent->setEnabled( (multi||single) && isLocal());
    m_DelCurrent->setEnabled( (multi||single) && isLocal());
    m_RevertAction->setEnabled( (multi||single) && isLocal());
    m_ResolvedAction->setEnabled( (multi||single) && isLocal());

    m_UpdateHead->setEnabled( (multi||single) && isLocal());
    m_UpdateRev->setEnabled( (multi||single) && isLocal());
    m_commitAction->setEnabled( (multi||single) && isLocal());
    m_simpleDiffHead->setEnabled( (single || none) && isLocal());

    m_changeToRepository->setEnabled(single&&dir&&isLocal());

    /* remote actions only */
    m_CheckoutCurrentAction->setEnabled( (single&&dir) && !isLocal());
    m_ExportCurrentAction->setEnabled( (single&&dir) && !isLocal());
    /* independ actions */
    m_CheckoutAction->setEnabled(true);
    m_ExportAction->setEnabled(true);
    m_RefreshViewAction->setEnabled(isopen);
}

void kdesvnfilelist::slotSelectionChanged()
{
    if (m_SelectedItems==0) {
        m_SelectedItems = new FileListViewItemList;
        m_SelectedItems->setAutoDelete(false);
    }
    m_SelectedItems->clear();

    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        m_SelectedItems->append( static_cast<FileListViewItem*>(it.current()) );
        ++it;
    }
    enableActions();
}

#include "kdesvnfilelist.moc"


/*!
    \fn kdesvnfilelist::slotClientException(const QString&)
 */
void kdesvnfilelist::slotClientException(const QString&what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(0,what,i18n("SVN Error"));
}


/*!
    \fn kdesvnfilelist::slotNotifyMessage(const QString&)
 */
void kdesvnfilelist::slotNotifyMessage(const QString&what)
{
    emit sigLogMessage(what);
    kapp->processEvents(20);
}

void kdesvnfilelist::slotChangeToRepository()
{
    if (!isLocal()) {
        return;
    }
    FileListViewItem*k = static_cast<FileListViewItem*>(firstChild());
    /* huh... */
    if (!k||!k->isDir()) return;
    QString ex = baseUri();
    if (!openURL(k->svnStatus().entry().url())) {
        openURL(ex);
    }
}

void kdesvnfilelist::slotItemDoubleClicked(QListViewItem*item)
{
    if (!item) return;

    FileListViewItem*fki = static_cast<FileListViewItem*>(item);
    if (fki->isDir()) {
        if (fki->isOpen()) {
            fki->setOpen(false);
        } else {
            fki->setOpen(true);
        }
        return;
    }
    KFileItem fitem(KFileItem::Unknown,KFileItem::Unknown,fki->fullName());
    fitem.run();
}

void kdesvnfilelist::slotCleanupAction()
{
    if (!isLocal()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which||!which->isDir()) return;
    m_SvnWrapper->slotCleanup(which->fullName());
    which->refreshStatus(true);
}

void kdesvnfilelist::slotResolved()
{
    if (!isLocal()) return;
    FileListViewItem*which= singleSelected();
    if (!which) which = static_cast<FileListViewItem*>(firstChild());
    if (!which) return;
    m_SvnWrapper->slotResolved(which->fullName());
    which->refreshStatus(true);
}

template<class T> KDialog* kdesvnfilelist::createDialog(T**ptr,const QString&_head,bool OkCancel)
{
    KDialogBase * dlg = new KDialogBase(
        0,
        0,
        true,
        _head,
        (OkCancel?KDialogBase::Ok|KDialogBase::Cancel:KDialogBase::Ok)/*,
        (OkCancel?KDialogBase::Cancel:KDialogBase::Close),
        KDialogBase::Cancel,
        true*//*,(OkCancel?KStdGuiItem::ok():KStdGuiItem::close())*/);

    if (!dlg) return dlg;
    QWidget* Dialog1Layout = dlg->makeVBoxMainWidget();
    *ptr = new T(Dialog1Layout);
    dlg->resize( QSize(320,240).expandedTo(dlg->minimumSizeHint()) );
    return dlg;
}

void kdesvnfilelist::slotImportDirsIntoCurrent()
{
    slotImportIntoCurrent(true);
}

/*!
    \fn kdesvnfilelist::slotImportIntoCurrent()
 */
void kdesvnfilelist::slotImportIntoCurrent(bool dirs)
{
    if (allSelected()->count()>1) {
        KMessageBox::error(this,i18n("Cannot import into multiple targets!"));
        return;
    }
    QString targetUri;
    if (allSelected()->count()==0) {
        targetUri=baseUri();
    } else {
        targetUri = QString::fromLocal8Bit(allSelected()->at(0)->svnStatus().entry().url());
    }
    while (targetUri.endsWith("/")) {
        targetUri.truncate(targetUri.length()-1);
    }

    KURL uri;
    if (dirs) uri = KFileDialog::getExistingDirectory(QString::null,this,"Import files from directory");
    else uri = KFileDialog::getImageOpenURL(QString::null,this,"Import file");

    if (uri.url().isEmpty()) return;

    if ( !uri.protocol().isEmpty() && uri.protocol()!="file") {
        KMessageBox::error(this,i18n("Cannot import into remote targets!"));
        return;
    }

    Logmsg_impl*ptr;
    Importdir_logmsg*ptr2 = 0;

    KDialog*dlg;
    if (dirs) {
         dlg = createDialog(&ptr2,QString(i18n("Import log")),true);
         ptr = ptr2;
    } else {
        dlg = createDialog(&ptr,QString(i18n("Import log")),true);
    }
    if (!dlg) return;
    ptr->initHistory();
    if (dlg->exec()!=QDialog::Accepted) {
        delete dlg;
        return;
    }
    QString logMessage = ptr->getMessage();
    bool rec = ptr->isRecursive();
    ptr->saveHistory();
    bool isMkdir = false;
    uri.setProtocol("");
    QString iurl = uri.path();

    if (dirs && ptr2) {
        isMkdir=ptr2->createDir();
        if (isMkdir) {
            targetUri+= uri.path();
        }
    }
    while (iurl.endsWith("/")) {
        iurl.truncate(iurl.length()-1);
    }
    m_SvnWrapper->slotImport(iurl,targetUri,logMessage,rec);
    if (!isLocal()) {
        if (allSelected()->count()==0) {
            openURL(baseUri());
        } else {
            slotReinitItem(allSelected()->at(0));
        }
    }
}

void kdesvnfilelist::refreshCurrentTree()
{
    FileListViewItem*item = static_cast<FileListViewItem*>(firstChild());
    if (!item) return;
    kapp->processEvents();
    setUpdatesEnabled(false);
    if (item->fullName()==baseUri()) {
        item->refreshMe();
        if (!item->isValid()) {
            delete item;
            return;
        } else {
            refreshRecursive(item);
        }
    } else {
        refreshRecursive(0);
    }
    setUpdatesEnabled(true);
    viewport()->repaint();
}

void kdesvnfilelist::refreshCurrent(FileListViewItem*cur)
{
    if (!cur) {
        refreshCurrentTree();
        return;
    }
    kapp->processEvents();
    setUpdatesEnabled(false);
    refreshRecursive(cur);
    setUpdatesEnabled(true);
    viewport()->repaint();
}

void kdesvnfilelist::refreshRecursive(FileListViewItem*_parent)
{
    FileListViewItem*item;
    if (_parent) {
        item = static_cast<FileListViewItem*>(_parent->firstChild());
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }

    if (!item) return;
    kapp->processEvents();

    FileListViewItemList currentSync;
    currentSync.setAutoDelete(false);

    while (item) {
        currentSync.append(item);
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }

    QString what = (_parent!=NULL?_parent->fullName():baseUri());
    svn::StatusEntries dlist;
    try {
        /* settings about unknown and ignored files must be setable */
        //                                                              rec   all  up    noign
        dlist = m_SvnWrapper->svnclient()->status(what.local8Bit(),false,true,false,true);
    } catch (svn::ClientException e) {
        //Message box!
        m_LastException = QString::fromLocal8Bit(e.message());
        emit sigLogMessage(m_LastException);
        return;
    } catch (...) {
        kdDebug()<< "Other exception" << endl;
        return;
    }
    svn::StatusEntries::iterator it = dlist.begin();
    FileListViewItem*k;
    bool gotit = false;
    for (;it!=dlist.end();++it) {
        if (QString::compare(it->path(),what)==0) {
            continue;
        }
        FileListViewItemListIterator clistIter(currentSync);
        gotit = false;
        while ( (k=clistIter.current()) ) {
            ++clistIter;
            if ( QString::compare(k->fullName(),QString::fromLocal8Bit(it->path()))==0) {
                currentSync.removeRef(k);
                k->updateStatus(*it);
                gotit = true;
                break;
            }
        }
        if (!gotit) {
            FileListViewItem * item;
            if (!_parent) {
                item = new FileListViewItem(this,*it);
            } else {
                item = new FileListViewItem(this,_parent,*it);
            }
            if (item->isDir()) {
                m_Dirsread[item->fullName()]=false;
            }
        }
    }
    FileListViewItemListIterator dIter(currentSync);
#ifndef NDEBUG
    slotSelectionChanged();
    kdDebug() << "Selected items " << m_SelectedItems->count()<< endl;
#endif
    while ( (k=dIter.current()) ) {
        ++dIter;
        delete k;
        // @todo just for debugging!
#ifndef NDEBUG
        m_SelectedItems->clear();
        QListViewItemIterator qlvit( this, QListViewItemIterator::Selected );
        while ( qlvit.current() ) {
            m_SelectedItems->append( static_cast<FileListViewItem*>(qlvit.current()) );
            ++qlvit;
        }
        kdDebug() << "Selected items " << m_SelectedItems->count() << endl;
#endif
    }
    if (_parent) {
        item = static_cast<FileListViewItem*>(_parent->firstChild());
    } else {
        item = static_cast<FileListViewItem*>(firstChild());
    }
    while (item) {
        if (item->isDir()&&(m_Dirsread.find(item->fullName())!=m_Dirsread.end()&&m_Dirsread[item->fullName()]==true)) {
            refreshRecursive(item);
        }
        item = static_cast<FileListViewItem*>(item->nextSibling());
    }
}

void kdesvnfilelist::slotRightButton(QListViewItem *_item, const QPoint &, int)
{
    FileListViewItem*item = static_cast<FileListViewItem*>(_item);
    if (item) {
        if (isLocal()) {
            emit sigShowPopup("local_context");
        } else {
            emit sigShowPopup("remote_context");
        }
    } else {
        emit sigShowPopup("general_empty");
    }
}
