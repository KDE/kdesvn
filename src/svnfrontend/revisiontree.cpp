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
#include <qtable.h>
#include <qlabel.h>

class RtreeData
{
public:
    RtreeData();
    virtual ~RtreeData();

    QMap<long,eLog_Entry> m_History;
    QMap<long,eLog_Entry> m_OldHistory;

    long max_rev,min_rev;
    KProgressDialog*progress;
    QTime m_stopTick;
    int current_row;

    //KListView*m_Display;
    QTable*m_Display;
};

RtreeData::RtreeData()
    : max_rev(-1),min_rev(-1)
{
    progress=0;
    m_Display = 0;
    current_row=-1;
}

RtreeData::~RtreeData()
{
    delete progress;
}
/*
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
*/

class RListItem:public QLabel
{
protected:
    long m_rev;
    RListItem*m_prev;
public:
    RListItem(long rev);
    RListItem(RListItem*previous,long rev);
    virtual ~RListItem();
    virtual void setText(int,const QString&);
};

RListItem::RListItem(RListItem*previous,long rev)
    : QLabel(0,0,0),m_rev(rev)
{
    m_prev = previous;
}

RListItem::RListItem(long rev)
    : QLabel(0,0,0),m_rev(rev),m_prev(0)
{
}

RListItem::~RListItem()
{
}

void RListItem::setText(int,const QString&text)
{
    QLabel::setText(text);
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
        m_Data->m_OldHistory[(*_logs)[j].revision]=(*_logs)[j];
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
            /*
            m_Data->m_Display=new KListView(treeParent);
            m_Data->m_Display->addColumn("Item");
            m_Data->m_Display->setRootIsDecorated(true);
            */
            m_Data->m_Display=new QTable(treeParent);
            m_Data->m_Display->setShowGrid(false);
            m_Data->m_Display->setRowMovingEnabled(false);

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

bool RevisionTree::isDeleted(long revision,const QString&path)
{
    for (unsigned i = 0;i<m_Data->m_History[revision].changedPaths.count();++i) {
        if (isParent(m_Data->m_History[revision].changedPaths[i].path,path) && m_Data->m_History[revision].changedPaths[i].action=='D') {
            return true;
        }
    }
    return false;
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
        for (unsigned i = 0; i<m_Data->m_OldHistory[j].changedPaths.count();++i) {
            /* find min revision of item */
            if (m_Data->m_OldHistory[j].changedPaths[i].action=='A'&&
                isParent(m_Data->m_OldHistory[j].changedPaths[i].path,m_Path))
            {
                if (!m_Data->m_OldHistory[j].changedPaths[i].copyFromPath.isEmpty()) {
                    if (m_InitialRevsion<m_Data->m_OldHistory[j].revision) {
                        QString tmpPath = m_Path;
                        QString r = m_Path.mid(m_Data->m_OldHistory[j].changedPaths[i].path.length());
                        m_Path=m_Data->m_OldHistory[j].changedPaths[i].copyFromPath;
                        m_Path+=r;
                        kdDebug()<<"Switched target to "<<m_Path<<endl;
                    }
                } else if (m_Data->m_OldHistory[j].changedPaths[i].path==m_Path){
                    // here it is added
                    m_InitialRevsion = m_Data->m_OldHistory[j].revision;
                    kdDebug()<<"Found add item at revision "<<m_InitialRevsion<<endl;
                }
            } else {
            }
        }
    }
    if (cancel==true) {
        return false;
    }
    /* find forward references and filter them out */
    for (long j=m_Data->max_rev;j>=m_Data->min_rev;--j) {
        m_Data->progress->progressBar()->setProgress(m_Data->max_rev-j);
        kapp->processEvents();
        if (m_Data->progress->wasCancelled()) {
            cancel=true;
            break;
        }
        for (unsigned i = 0; i<m_Data->m_OldHistory[j].changedPaths.count();++i) {
            if (!m_Data->m_OldHistory[j].changedPaths[i].copyFromPath.isEmpty()) {
                long r = m_Data->m_OldHistory[j].changedPaths[i].copyFromRevision;
                char a='H';
                for (unsigned z = 0;z<m_Data->m_OldHistory[j].changedPaths.count();++z) {
                    if (isParent(m_Data->m_OldHistory[j].changedPaths[z].path,m_Data->m_OldHistory[j].changedPaths[z].path)
                         && m_Data->m_OldHistory[j].changedPaths[z].action=='D') {
                        a='R';
                        if (m_Data->m_OldHistory[j].changedPaths[z].path==m_Data->m_OldHistory[j].changedPaths[i].path) {
                            m_Data->m_OldHistory[j].changedPaths[z].action=0;
                        }
                        break;
                    }
                }
                QString sourcepath = m_Data->m_OldHistory[j].changedPaths[i].copyFromPath;
                m_Data->m_History[j].addCopyTo(sourcepath,m_Data->m_OldHistory[j].changedPaths[i].path,j,a,r);
                m_Data->m_OldHistory[j].changedPaths[i].action=0;
            }
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
        for (unsigned i = 0; i<m_Data->m_OldHistory[j].changedPaths.count();++i) {
            if (m_Data->m_OldHistory[j].changedPaths[i].action==0) {
                continue;
            }
            m_Data->m_History[j].addCopyTo(m_Data->m_OldHistory[j].changedPaths[i].path,QString::null,-1,m_Data->m_OldHistory[j].changedPaths[i].action);
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

bool RevisionTree::bottomUpScan(long startrev,unsigned recurse,const QString&_path,RListItem*parentItem)
{
#define REVENTRY m_Data->m_History[j]
#define FORWARDENTRY m_Data->m_History[j].forwardPaths[i]

    QString path = _path;
    kdDebug()<<"Searching for "<<path<< " at revision " << startrev
        << " recursion " << recurse << endl;
    bool cancel = false;
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
            if (isParent(FORWARDENTRY.path,path)) {
                RListItem*previous = 0;
                bool get_out = false;
                if (FORWARDENTRY.action=='H' || FORWARDENTRY.action=='R') {
                    bool ren = FORWARDENTRY.action=='R';
                    QString tmpPath = path;
                    QString recPath;
                    if (FORWARDENTRY.copyToPath.length()==0) {
                        continue;
                    }
                    QString r = path.mid(FORWARDENTRY.path.length());
                    recPath= FORWARDENTRY.copyToPath;
                    recPath+=r;
                    if (!ren) {
                        kdDebug()<<"Copy to "<< recPath << endl;
                    } else {
                        kdDebug()<<"Renamed to "<< recPath << " at revison " << j << endl;
                        path=recPath;
                    }
                    previous = getItem(parentItem,j);
                    previous->setText(0,QString("%1 %2 to %3 (%4 to %5)")
                        .arg(ren?"Rename":"Copy")
                        .arg(tmpPath).arg(recPath)
                        .arg(FORWARDENTRY.copyFromRevision).arg(FORWARDENTRY.copyToRevision));
                    //previous->setText(0,"Copy source: "+path);
                    if (!ren) {
                        if (!bottomUpScan(FORWARDENTRY.copyToRevision,recurse+1,recPath,previous)) {
                            return false;
                        }
                    }
                } else if (FORWARDENTRY.path==path) {
                    switch (FORWARDENTRY.action) {
                    case 'A':
                        kdDebug()<<"Inserting adding base item"<<endl;
                        previous = getItem(parentItem,j);
                        text=QString("Add %1 at revision %2").arg(FORWARDENTRY.path).arg(j);
                        previous->setText(0,text);
                    break;
                    case 'M':
                        kdDebug()<<"Item modified at revision "<< j << " recurse " << recurse << endl;
                        text=QString("Modify %1 at %2").arg(path).arg(j);
                        previous = getItem(parentItem,j);
                        previous->setText(0,text);
                    break;
                    case 'D':
                        kdDebug()<<"Item deleted at revision "<< j << " recurse " << recurse << endl;
                        text=QString("Delete %1 at %2").arg(path).arg(j);
                        previous = getItem(parentItem,j);
                        previous->setText(0,text);
                        get_out= true;
                    break;
                    default:
                    break;
                    }
                } else {
                    switch (FORWARDENTRY.action) {
                    case 'D':
                        kdDebug()<<"Item deleted at revision "<< j << " recurse " << recurse << endl;
                        text=QString("Delete %1 at %2").arg(path).arg(j);
                        previous = getItem(parentItem,j);
                        previous->setText(0,text);
                        get_out = true;
                    break;
                    default:
                    break;
                    }
                }
                if (previous) {
                    ++m_Data->current_row;
                    QString tip = m_Data->m_OldHistory[j].author;
//                    previous->setTooltip(tip);
                    if (recurse+1>m_Data->m_Display->numCols()) {
                        m_Data->m_Display->setNumCols(recurse+1);
                    }
                    m_Data->m_Display->setNumRows(m_Data->current_row+1);
                    m_Data->m_Display->setCellWidget(m_Data->current_row,recurse,previous);
                }
                if (get_out) {
                    return true;
                }
            }
        }
    }
    return !cancel;
}

QWidget*RevisionTree::getView()
{
    return m_Data->m_Display;
}

RListItem*RevisionTree::getItem(RListItem*parent,long rev)
{
    RListItem * res;
    if (parent) {
        kdDebug()<<"Item with parent"<<endl;
        res = new RListItem(parent,rev);
    } else {
        //res = new RListItem(m_Data->m_Display,rev);
        res = new RListItem(rev);
    }
    //res->setOpen(true);
    return res;
}
