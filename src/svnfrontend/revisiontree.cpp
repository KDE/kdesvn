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

class RtreeData
{
public:
    RtreeData();
    virtual ~RtreeData();

    QMap<long,eLog_Entry> m_History;
    long max_rev,min_rev;
};

RtreeData::RtreeData()
    : max_rev(-1),min_rev(-1)
{
}

RtreeData::~RtreeData()
{
}

RevisionTree::RevisionTree(const svn::LogEntries*_logs,const QString&origin,const svn::Revision& baserevision)
    :m_Path(origin),m_InitialRevsion(0)
{
    m_Data = new RtreeData;
    long possible_rev=-1;

    for (unsigned j=0; j<_logs->count();++j) {
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
    kdDebug()<<"Origin: "<<origin<<" max revision " << m_Data->max_rev
        << " min revision " << m_Data->min_rev << endl;
    topDownScan();
}

RevisionTree::~RevisionTree()
{
    delete m_Data;
}

void RevisionTree::topDownScan()
{
    for (long j=m_Data->max_rev;j>=m_Data->min_rev;--j) {
        for (unsigned i = 0; i<m_Data->m_History[j].echangedPaths.count();++i) {
            /* find min revision of item */
            if (m_Data->m_History[j].echangedPaths[i].action=='A'&&
                isParent(m_Data->m_History[j].echangedPaths[i].path,m_Path))
            {
                if (!m_Data->m_History[j].echangedPaths[i].copyFromPath.isEmpty()) {
                    if (m_InitialRevsion<m_Data->m_History[j].revision) {
                        QString r = m_Path.mid(m_Data->m_History[j].echangedPaths[i].path.length());
                        m_Path=m_Data->m_History[j].echangedPaths[i].copyFromPath;
                        m_Path+=r;
                        kdDebug()<<"Switched target to "<<m_Path<<endl;
                    }
                } else if (m_Data->m_History[j].echangedPaths[i].path==m_Path){
                    // here it is added
                    m_InitialRevsion = m_Data->m_History[j].revision;
                    kdDebug()<<"Found add item at revision "<<m_InitialRevsion<<endl;
                }
            }

            /* build forward ref */
            if (!m_Data->m_History[j].echangedPaths[i].copyFromPath.isEmpty()) {
                /*kdDebug()<<"Insert copy to " << m_Data->m_History[j].echangedPaths[i].path
                    << " from revision " << m_Data->m_History[j].echangedPaths[i].copyFromRevision << endl;*/
                m_Data->m_History[m_Data->m_History[j].echangedPaths[i].copyFromRevision].addCopyTo(m_Data->m_History[j].echangedPaths[i].copyFromPath,m_Data->m_History[j].echangedPaths[i].path,m_Data->m_History[j].echangedPaths[i].copyFromRevision);
#if 0
                if (m_Data->m_History[j].echangedPaths[i].path==m_Path) {
                    kdDebug()<<"Switch over target to "<<m_Data->m_History[j].echangedPaths[i].copyFromPath<<endl;
                    m_Path=m_Data->m_History[j].echangedPaths[i].copyFromPath;
                    m_Baserevision=m_Data->m_History[j].echangedPaths[i].copyFromRevision;
                } else if (isParent(m_Data->m_History[j].echangedPaths[i].path,m_Path)) {
                    QString r = m_Path.mid(m_Data->m_History[j].echangedPaths[i].path.length());
                    m_Path=m_Data->m_History[j].echangedPaths[i].copyFromPath;
                    m_Path+=r;
                    kdDebug()<<"Parent copy - change to "<<m_Path << " ("<<m_Data->m_History[j].echangedPaths[i].path<<")"<<endl;
                    m_Baserevision=m_Data->m_History[j].echangedPaths[i].copyFromRevision;
//                    j=m_Data->m_History[j].echangedPaths[i].copyFromRevision+1;
                }
#endif
            }
        }
    }
}

bool RevisionTree::isParent(const QString&_par,const QString&tar)
{
    if (_par==tar) return true;
    QString par = _par+(_par.endsWith("/")?"":"/");
    return tar.startsWith(par);
}
