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
 : KListView(parent, name),m_Svnclient(),m_SvnWrapper(new SvnActions(this))
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
    addColumn(i18n("Last Revision"));
    addColumn(i18n("Author"));
    addColumn(i18n("Last Date"));
    setSortColumn(FileListViewItem::COL_NAME);
    setupActions();

    connect(this,SIGNAL(clicked(QListViewItem*)),this,SLOT(slotItemClicked(QListViewItem*)));
    connect(this,SIGNAL(selectionChanged()),this,SLOT(slotSelectionChanged()));
    connect(m_SvnWrapper,SIGNAL(clientException(const QString&)),this,SLOT(slotClientException(const QString&)));
}

void kdesvnfilelist::setupActions()
{
    m_filesAction = new KActionCollection(this);

    m_LogRangeAction = new KAction("&Log...",KShortcut(),m_SvnWrapper,SLOT(slotMakeRangeLog()),m_filesAction,"make_svn_log");
    m_LogFullAction = new KAction("&Full Log",KShortcut(),m_SvnWrapper,SLOT(slotMakeLog()),m_filesAction,"make_svn_log");
    m_BlameAction = new KAction("&Blame",KShortcut(),m_SvnWrapper,SLOT(slotBlame()),m_filesAction,"make_svn_blame");

    enableSingleActions(false);
}

KActionCollection*kdesvnfilelist::filesActions()
{
    return m_filesAction;
}

FileListViewItem* kdesvnfilelist::singleSelected()
{
    QPtrList<QListViewItem> lst;
    QListViewItemIterator it( this, QListViewItemIterator::Selected );
    while ( it.current() ) {
        lst.append( it.current() );
        ++it;
    }
    if (lst.count()==1) {
        return static_cast<FileListViewItem*>(lst.at(0));
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
    m_directoryList.clear();
    m_Svnclient.setContext(nc);
    QString what = url.url();
    m_isLocal = false;
    if (url.isLocalFile()) {
        what = url.path();
        m_isLocal = true;
    }
    return checkDirs(what,0);
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
        dlist = m_Svnclient.status(what.ascii(),false,true,false,true);
    } catch (svn::ClientException e) {
        //Message box!
        m_LastException = e.message();
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
        }
    }
    if (_parent) {
        _parent->setOpen(true);
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
    m_BlameAction->setEnabled(how&&!_Dir);
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
    qDebug("client exception: %s",what.ascii());
    KMessageBox::sorry(0,what,I18N_NOOP("SVN Error"));
}
