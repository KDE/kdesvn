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
#ifndef KDESVNFILELIST_H
#define KDESVNFILELIST_H

#include <klistview.h>
#include <kdirlister.h>
#include <qmap.h>
#include "svncpp/client.hpp"
#include "svncpp/status.hpp"

class KAction;
class KActionMenu;
class KActionCollection;
class CContextListener;

/**
@author Rajko Albrecht
*/
class kdesvnfilelist : public KListView
{
    Q_OBJECT
    friend class FileListViewItem;
    friend class SvnActions;
public:
    kdesvnfilelist(QWidget *parent = 0, const char *name = 0);
    virtual ~kdesvnfilelist();

    bool openURL( const KURL &url );
    const QString&lastError()const{return m_LastException;}
    const svn::StatusEntries&directories()const{return m_directoryList;}
    const svn::Status&maindir()const{return m_mainEntry;}
    const QString&baseUri()const{return m_baseUri;}
    bool isLocal()const{return m_isLocal;}

    KActionCollection*filesActions();

protected:
    svn::Client m_Svnclient;
    svn::StatusEntries m_directoryList;
    svn::Status m_mainEntry;
    bool m_isLocal;

    QString m_LastException,m_baseUri;
    QMap<QString,bool> m_Dirsread;

    KActionCollection* m_filesAction;
    KAction*m_LogFullAction,*m_LogRangeAction,*m_BlameAction/*,*m_BlameRangeAction*/,*m_CatAction,*m_MkdirAction;
    SvnActions*m_SvnWrapper;
    CContextListener*m_SvnContext;
    /* the parent entry must removed from list before */
    void insertDirs(FileListViewItem * _parent,svn::StatusEntries&);
    bool checkDirs(const QString&,FileListViewItem * _parent);
    svn::Client* svnclient(){return &m_Svnclient;}
    void setupActions();
    void enableSingleActions(bool how,bool forDir=false);

    FileListViewItem* singleSelected();

protected slots:
    virtual void slotItemClicked(QListViewItem*);
    virtual void slotSelectionChanged();
    virtual void slotClientException(const QString&);
    virtual void slotNotifyMessage(const QString&);
    virtual void slotDirAdded(const QString&,FileListViewItem*);
signals:
    void sigLogMessage(const QString&);
};

#endif
