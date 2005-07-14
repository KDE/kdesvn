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
#include "ccontextlistener.h"
#include "filelistviewitem.h"
#include "svnactions.h"
#include "svncpp/revision.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"
#include <kdirlister.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kmessagebox.h>

kdesvnfilelist::kdesvnfilelist(QWidget *parent, const char *name)
 : KListView(parent, name),m_Svnclient(),m_SvnWrapper(new SvnActions(this)),
    m_SvnContext(new CContextListener(this))
{
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
//    addColumn(i18n("Current revision"));
    setSortColumn(FileListViewItem::COL_NAME);
    setupActions();

    connect(this,SIGNAL(clicked(QListViewItem*)),this,SLOT(slotItemClicked(QListViewItem*)));
    connect(this,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
    connect(m_SvnContext,SIGNAL(sendNotify(const QString&)),this,SLOT(slotNotifyMessage(const QString&)));
    connect(m_SvnWrapper,SIGNAL(dirAdded(const QString&,FileListViewItem*)),this,
        SLOT(slotDirAdded(const QString&,FileListViewItem*)));
}

void kdesvnfilelist::setupActions()
{
    m_filesAction = new KActionCollection(this);

    m_LogRangeAction = new KAction("&Log...",KShortcut(),m_SvnWrapper,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    m_LogFullAction = new KAction("&Full Log",KShortcut(),m_SvnWrapper,SLOT(slotMakeLog()),m_filesAction,"make_svn_log");
    m_BlameAction = new KAction("&Blame",KShortcut(),m_SvnWrapper,SLOT(slotBlame()),m_filesAction,"make_svn_blame");
    m_CatAction = new KAction("&Cat head",KShortcut(),m_SvnWrapper,SLOT(slotCat()),m_filesAction,"make_svn_cat");
    // m_BlameRangeAction = new KAction("&Blame",KShortcut(),m_SvnWrapper,SLOT(slotRangeBlame()),m_filesAction,"make_svn_range_blame");
    m_MkdirAction = new KAction("Make (sub-)directory",KShortcut(),m_SvnWrapper,SLOT(slotMkdir()),m_filesAction,"make_svn_mkdir");
    m_InfoAction = new KAction("Details",KShortcut(),m_SvnWrapper,SLOT(slotInfo()),m_filesAction,"make_svn_info");

    m_MkdirAction->setEnabled(false);
    m_InfoAction->setEnabled(false);
    enableSingleActions(false);
}

KActionCollection*kdesvnfilelist::filesActions()
{
    return m_filesAction;
}

QPtrList<FileListViewItem> kdesvnfilelist::allSelected()
{
    QPtrList<FileListViewItem> lst;
    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        lst.append( static_cast<FileListViewItem*>(it.current()) );
        ++it;
    }
    return lst;
}

FileListViewItem* kdesvnfilelist::singleSelected()
{
    QPtrList<FileListViewItem> lst = allSelected();
    if (lst.count()==1) {
        return lst.at(0);
    }
    return 0;
}

bool kdesvnfilelist::openURL( const KURL &url )
{
    clear();
    m_Dirsread.clear();
    m_LastException="";
    svn::Context*nc = new svn::Context();
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
    nc->setListener(m_SvnContext);
    m_directoryList.clear();
    m_Svnclient.setContext(nc);
    m_baseUri = url.url();
    m_isLocal = false;
    if (url.isLocalFile()) {
        m_baseUri = url.path();
        m_isLocal = true;
    }
    while (m_baseUri.endsWith("/")) {
        m_baseUri.truncate(m_baseUri.length()-1);
    }
    bool result = checkDirs(m_baseUri,0);
    m_MkdirAction->setEnabled(result);
    m_InfoAction->setEnabled(m_isLocal);
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
        //                                       rec   all  up    noign
        dlist = m_Svnclient.status(what.local8Bit(),false,true,false,true);
    } catch (svn::ClientException e) {
        //Message box!
        m_LastException = QString::fromLocal8Bit(e.message());
        emit sigLogMessage(m_LastException);
        return false;
    } catch (...) {
        qDebug("Other exception");
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
            item->setOpen(true);
            m_directoryList.push_back(*it);
            m_Dirsread[item->fullName()]=false;
        }
    }
    if (_parent) {
        _parent->setOpen(true);
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
        stat = m_Svnclient.singleStatus(newdir.local8Bit());
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
        item->setOpen(true);
        m_Dirsread[item->fullName()]=false;
    }
}

kdesvnfilelist::~kdesvnfilelist()
{
    svn::Context*oc = (svn::Context*)m_Svnclient.getContext();
    if (oc) {
        delete oc;
    }
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

void kdesvnfilelist::enableSingleActions(bool how,bool _Dir)
{
    m_LogFullAction->setEnabled(how);
    m_LogRangeAction->setEnabled(how);
    m_CatAction->setEnabled(how&&!_Dir);
    /* blame buggy in lib */
    //m_BlameAction->setEnabled(false);
    m_BlameAction->setEnabled(how&&!_Dir);
    //m_BlameRangeAction->setEnabled(how&&!_Dir);
}

void kdesvnfilelist::slotSelectionChanged()
{
    QPtrList<FileListViewItem> lst;
    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        lst.append( static_cast<FileListViewItem*>(it.current()) );
        ++it;
    }
    enableSingleActions(lst.count()==1,lst.count()>0?lst.at(0)->isDir():false);
}

#include "kdesvnfilelist.moc"


/*!
    \fn kdesvnfilelist::slotClientException(const QString&)
 */
void kdesvnfilelist::slotClientException(const QString&what)
{
    emit sigLogMessage(what);
    KMessageBox::sorry(0,what,I18N_NOOP("SVN Error"));
}


/*!
    \fn kdesvnfilelist::slotNotifyMessage(const QString&)
 */
void kdesvnfilelist::slotNotifyMessage(const QString&what)
{
    emit sigLogMessage(what);
}
