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
#include "svncpp/status.hpp"
#include "svncpp/client.hpp"

class KAction;
class KActionMenu;
class KActionCollection;

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

    bool openURL( const KURL &url,bool noReinit=false );
    const QString&lastError()const{return m_LastException;}
    const svn::StatusEntries&directories()const{return m_directoryList;}
    const svn::Status&maindir()const{return m_mainEntry;}
    const QString&baseUri()const{return m_baseUri;}
    bool isLocal()const{return m_isLocal;}

    KActionCollection*filesActions();

protected:
    svn::StatusEntries m_directoryList;
    svn::Status m_mainEntry;
    bool m_isLocal;

    QString m_LastException,m_baseUri;
    QMap<QString,bool> m_Dirsread;

    KActionCollection* m_filesAction;
    KAction*m_LogFullAction,*m_LogRangeAction,*m_BlameAction,*m_BlameRangeAction,*m_CatAction,*m_MkdirAction;
    KAction*m_InfoAction,*m_propertyAction,*m_commitAction,*m_simpleDiffHead,*m_UpdateHead,*m_UpdateRev;
    KAction*m_AddCurrent,*m_DelCurrent,*m_CheckoutAction,*m_CheckoutCurrentAction,*m_RevertAction;
    KAction*m_changeToRepository,*m_switchRepository,*m_ExportAction,*m_ExportCurrentAction;

    SvnActions*m_SvnWrapper;
    /* the parent entry must removed from list before */
    void insertDirs(FileListViewItem * _parent,svn::StatusEntries&);
    bool checkDirs(const QString&,FileListViewItem * _parent);
    void setupActions();
    svn::Client*svnclient();

    void enableSingleActions(bool how,bool forDir=false);

    FileListViewItem* singleSelected();
    QPtrList<FileListViewItem> allSelected();

protected slots:
    virtual void slotItemClicked(QListViewItem*);
    virtual void slotSelectionChanged();
    virtual void slotClientException(const QString&);
    virtual void slotNotifyMessage(const QString&);
    virtual void slotDirAdded(const QString&,FileListViewItem*);
    virtual void slotChangeToRepository();
    virtual void slotReinitItem(FileListViewItem*);
    virtual void slotItemDoubleClicked(QListViewItem*);

signals:
    void sigLogMessage(const QString&);
    void changeCaption(const QString&);
};

#endif
