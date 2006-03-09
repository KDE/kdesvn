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
#include "revisiontree.h"
#include "svnqt/log_entry.hpp"
#include "elogentry.h"

#include <kdebug.h>
#include <kprogress.h>
#include <klocale.h>
#include <kapp.h>
#include <klistview.h>

#include <qwidget.h>
#include <qdatetime.h>

class RtreeData
{
public:
    RtreeData();
    virtual ~RtreeData();

    QMap<long,eLog_Entry> m_History;
    long max_rev,min_rev;
    KProgressDialog*progress;
    QTime m_stopTick;

    KListView*m_Display;
};

RtreeData::RtreeData()
    : max_rev(-1),min_rev(-1)
{
    progress=0;
    m_Display = 0;
}

RtreeData::~RtreeData()
{
    delete progress;
}

class RListItem:public KListViewItem
{
protected:
    long m_rev;
public:
    RListItem(KListView*,long rev);
    RListItem(KListViewItem*,long rev);
    virtual int compare(QListViewItem*i,int,bool)const;
};

RListItem::RListItem(KListView*p,long rev)
    :KListViewItem(p)
{
    m_rev = rev;
}

RListItem::RListItem(KListViewItem*p,long rev)
    :KListViewItem(p)
{
    m_rev = rev;
}

int RListItem::compare(QListViewItem*i,int,bool)const
{
    RListItem*r=static_cast<RListItem*>(i);
    if (r->m_rev>m_rev) {
        return -1;
    } else if (r->m_rev<m_rev) {
        return 1;
    }
    return 0;
}

RevisionTree::RevisionTree(const svn::LogEntries*_logs,const QString&origin,const svn::Revision& baserevision,QWidget*treeParent,QWidget*parent)
    :m_InitialRevsion(0),m_Path(origin),m_Valid(false)
{
    m_Data = new RtreeData;
    long possible_rev=-1;
    kdDebug()<<"Origin: "<<origin << endl;

    m_Data->progress=new KProgressDialog(
        parent,"progressdlg",i18n("Scanning logs"),i18n("Scanning the logs for requested item"),true);
    m_Data->progress->setMinimumDuration(100);
    m_Data->progress->setAllowCancel(true);
    m_Data->progress->progressBar()->setTotalSteps(_logs->count());
    m_Data->progress->setAutoClose(false);
    bool cancel=false;
    for (unsigned j=0; j<_logs->count();++j) {
        m_Data->progress->progressBar()->setProgress(j);
        kapp->processEvents();
        if (m_Data->progress->wasCancelled()) {
            cancel=true;
            break;
        }
        if ((*_logs)[j].revision>m_Data->max_rev) {
            m_Data->max_rev=(*_logs)[j].revision;
        }
        if ((*_logs)[j].revision<m_Data->min_rev||m_Data->min_rev==-1) {
            m_Data->min_rev=(*_logs)[j].revision;
        }
        if (baserevision.kind()==svn_opt_revision_date) {
            if (baserevision.date()<=(*_logs)[j].date && possible_rev==-1||possible_rev>(*_logs)[j].revision) {
                possible_rev=(*_logs)[j].revision;
            }
        }
        m_Data->m_History[(*_logs)[j].revision]=(*_logs)[j];
    }
    if (baserevision.kind()==svn_opt_revision_head||baserevision.kind()==svn_opt_revision_working) {
        m_Baserevision=m_Data->max_rev;
    } else if (baserevision.kind()==svn_opt_revision_number) {
        m_Baserevision=baserevision.revnum();
    } else if (baserevision.kind()==svn_opt_revision_date) {
        m_Baserevision=possible_rev;
    }
    if (!cancel) {
        kdDebug( )<<" max revision " << m_Data->max_rev
            << " min revision " << m_Data->min_rev << endl;
        if (topDownScan()) {
            m_Data->progress->setAutoReset(true);
            m_Data->progress->progressBar()->setTotalSteps(100);
            m_Data->progress->progressBar()->setPercentageVisible(false);
            m_Data->m_stopTick.restart();
            m_Data->m_Display=new KListView(treeParent);
            m_Data->m_Display->addColumn("Item");
            m_Data->m_Display->setRootIsDecorated(true);
            if (bottomUpScan(m_InitialRevsion,0,m_Path,0)) {
                m_Valid=true;
            } else {
                delete m_Data->m_Display;
                m_Data->m_Display=0;
            }
        }
    } else {
        kdDebug()<<"Canceld"<<endl;
    }
    m_Data->progress->hide();
}

RevisionTree::~RevisionTree()
{
    delete m_Data;
}

bool RevisionTree::topDownScan()
{
    m_Data->progress->progressBar()->setTotalSteps(m_Data->max_rev-m_Data->min_rev);
    bool cancel=false;
    for (long j=m_Data->max_rev;j>=m_Data->min_rev;--j) {
        m_Data->progress->progressBar()->setProgress(m_Data->max_rev-j);
        kapp->processEvents();
        if (m_Data->progress->wasCancelled()) {
            cancel=true;
            break;
        }
        for (unsigned i = 0; i<m_Data->m_History[j].changedPaths.count();++i) {
            if (m_Data->m_History[j].changedPaths[i].copyFromPath.isEmpty()) {
                continue;
            }
            long r = m_Data->m_History[j].changedPaths[i].copyFromRevision;
            QString sourcepath = m_Data->m_History[j].changedPaths[i].copyFromPath;
            m_Data->m_History[r].addCopyTo(
                sourcepath,
                m_Data->m_History[j].changedPaths[i].path,
                j,
                m_Data->m_History[j].changedPaths[i].action);
        }
    }
    if (cancel==true) {
        return false;
    }
    for (long j=m_Data->max_rev;j>=m_Data->min_rev;--j) {
        m_Data->progress->progressBar()->setProgress(m_Data->max_rev-j);
        kapp->processEvents();
        if (m_Data->progress->wasCancelled()) {
            cancel=true;
            break;
        }
        for (unsigned i = 0; i<m_Data->m_History[j].changedPaths.count();++i) {
            /* find min revision of item */
            if (m_Data->m_History[j].changedPaths[i].action=='A'&&
                isParent(m_Data->m_History[j].changedPaths[i].path,m_Path))
            {
                if (!m_Data->m_History[j].changedPaths[i].copyFromPath.isEmpty()) {
                    if (m_InitialRevsion<m_Data->m_History[j].revision) {
                        QString tmpPath = m_Path;
                        QString r = m_Path.mid(m_Data->m_History[j].changedPaths[i].path.length());
                        m_Path=m_Data->m_History[j].changedPaths[i].copyFromPath;
                        m_Path+=r;
                        kdDebug()<<"Switched target to "<<m_Path<<endl;
#if 0
                        m_Data->m_History[m_Data->m_History[j].changedPaths[i].copyFromRevision].addCopyTo(
                            m_Path,
                            tmpPath,
                            m_Data->m_History[j].changedPaths[i].copyFromRevision,
                            'A'
                        );
#endif
                    }
                } else if (m_Data->m_History[j].changedPaths[i].path==m_Path){
                    // here it is added
                    m_InitialRevsion = m_Data->m_History[j].revision;
                    kdDebug()<<"Found add item at revision "<<m_InitialRevsion<<endl;
                }
            } else {
#if 0
                /* build forward ref */
                m_Data->m_History[m_Data->m_History[j].changedPaths[i].copyFromRevision].addCopyTo(
                    m_Data->m_History[j].changedPaths[i].copyFromPath,
                    m_Data->m_History[j].changedPaths[i].path,
                    m_Data->m_History[j].changedPaths[i].copyFromRevision,
                    m_Data->m_History[j].changedPaths[i].action);
#endif
            }
        }
    }
    return !cancel;
}

bool RevisionTree::isParent(const QString&_par,const QString&tar)
{
    if (_par==tar) return true;
    QString par = _par+(_par.endsWith("/")?"":"/");
    return tar.startsWith(par);
}

bool RevisionTree::isValid()const
{
    return m_Valid;
}

bool RevisionTree::bottomUpScan(long startrev,unsigned recurse,const QString&path,RListItem*parentItem)
{
#define REVENTRY m_Data->m_History[j]
#define FORWARDENTRY m_Data->m_History[j].forwardPaths[i]
#define CHANGEDENTRY m_Data->m_History[j].changedPaths[i]
    kdDebug()<<"Searching for "<<path<< " at revision " << startrev
        << " recursion " << recurse << endl;
    bool cancel = false;
    RListItem*previous = 0;
    RListItem*addItem=0;
    for (long j=startrev;j<=m_Data->max_rev;++j) {
        if (m_Data->m_stopTick.elapsed()>500) {
            m_Data->progress->progressBar()->advance(1);
            kapp->processEvents();
            m_Data->m_stopTick.restart();
        }
        if (m_Data->progress->wasCancelled()) {
            cancel=true;
            break;
        }
        for (unsigned i=0;i<REVENTRY.forwardPaths.count();++i) {
            if (!isParent(FORWARDENTRY.path,path)) {
                continue;
            }
            QString text;
            switch (FORWARDENTRY.toAction) {
                case eLogChangePathEntry::added:
                    kdDebug()<<"Inserting adding base item"<<endl;
                    addItem = getItem(parentItem,j);
                    text=QString("Add %1 at revision %2").arg(FORWARDENTRY.path).arg(j);
                    addItem->setText(0,text);
                    break;
                case eLogChangePathEntry::addedWithHistory:
                    {
                        QString tmpPath = path;
                        QString recPath;
                        QString r = path.mid(FORWARDENTRY.path.length());
                        recPath= FORWARDENTRY.copyToPath;
                        recPath+=r;
                        kdDebug()<<"Copy to "<< recPath << endl;
                        previous = getItem(parentItem?parentItem:addItem,j);
                        previous->setText(0,QString("Copy %1 to %2 (%3 to %4)")
                            .arg(tmpPath).arg(recPath)
                            .arg(j).arg(FORWARDENTRY.copyToRevision));
                        //previous->setText(0,"Copy source: "+path);
                        if (!bottomUpScan(FORWARDENTRY.copyToRevision,recurse+1,recPath,previous)) {
                            return false;
                        }
                        break;
                    }
                case eLogChangePathEntry::modified:
                {
                    kdDebug()<<path<<" modified at revision "<< j << " recurse " << recurse << endl;
                    break;
                }
                case eLogChangePathEntry::deleted:
                {
                    kdDebug()<<"Item deleted at revision "<< j << " recurse " << recurse << endl;
                    break;
                }
            }
        }
#if 1
        for (unsigned i=0;i<REVENTRY.changedPaths.count();++i) {
            if (!isParent(CHANGEDENTRY.path,path)) {
                continue;
            }
            QString text;
            if (isParent(CHANGEDENTRY.path,path)&&CHANGEDENTRY.action=='A') {
                if (!CHANGEDENTRY.copyFromPath.isEmpty()) {
                    kdDebug()<<"Add copy from"<<CHANGEDENTRY.copyFromPath<<endl;
                    previous=getItem(parentItem,j);
                    text=QString("Copy from %1 to %2").arg(CHANGEDENTRY.copyFromPath).arg(path);
                    previous->setText(0,text);
                } else {
                    kdDebug()<<"Base add of "<<path<<endl;
                    addItem = getItem(parentItem,j);
                    text = QString("Add at %1 path %2").arg(j).arg(path);
                    addItem->setText(0,text);
                }
            } else if (CHANGEDENTRY.path==path) {
                switch (CHANGEDENTRY.action) {
                    case 'A':
                        break;
                    case 'M':
                        kdDebug()<<"Item modified at revision "<< j << " recurse " << recurse << endl;
                        text=QString("Modify %1 at %2").arg(path).arg(j);
                        previous = getItem(parentItem?parentItem:addItem,j);
                        previous->setText(0,text);
                        break;
                    case 'D':
                        kdDebug()<<"Item deleted at revision "<< j << " recurse " << recurse << endl;
                        break;
                }
            }

        }
#endif
    }
    return !cancel;
}

KListView*RevisionTree::getView()
{
    return m_Data->m_Display;
}

RListItem*RevisionTree::getItem(KListViewItem*parent,long rev)
{
    RListItem * res;
    if (parent) {
        kdDebug()<<"Item with parent"<<endl;
        res = new RListItem(parent,rev);
    } else {
        res = new RListItem(m_Data->m_Display,rev);
    }
    res->setOpen(true);
    return res;
}
