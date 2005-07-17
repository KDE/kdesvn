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
#ifndef SVNACTIONS_H
#define SVNACTIONS_H

#include <qobject.h>
#include <qdatetime.h>
#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"

class kdesvnfilelist;
class FileListViewItem;
class KDialog;
class QDialog;
class CContextListener;
class KProcess;

namespace svn {
    class Context;
}

/**
@author Rajko Albrecht
*/
class SvnActions : public QObject
{
    Q_OBJECT
public:
    SvnActions(kdesvnfilelist *parent = 0, const char *name = 0);
    ~SvnActions();
    void reInitClient();
    //svn::Client&svnClient(){return m_Svnclient;}
    svn::Client* svnclient(){return &m_Svnclient;}

protected:
    SvnActions(QObject *parent = 0, const char *name = 0);
    void makeLog(svn::Revision start,svn::Revision end,FileListViewItem*k);
    void makeBlame(svn::Revision start, svn::Revision end, FileListViewItem*k);
    void makeCat(svn::Revision start, FileListViewItem*k);
    kdesvnfilelist* m_ParentList;

    CContextListener*m_SvnContext;
    svn::Context* m_CurrentContext;
    svn::Client m_Svnclient;

    template<class T> KDialog* createDialog(T**ptr,const QString&_head,bool OkCance=false);
    static QDateTime apr2qttime(apr_time_t);

public slots:
    virtual void slotMakeRangeLog();
    virtual void slotMakeLog();
    virtual void slotBlame();
    virtual void slotRangeBlame();
    virtual void slotCat();
    virtual void slotMkdir();
    virtual void slotInfo();
    virtual void slotProperties();
    virtual void slotNotifyMessage(const QString&);
    virtual void slotCommit();
    virtual void slotSimpleDiff();
    virtual void slotSimpleDiffBase();
    virtual void makeDiff(const QString&,const svn::Revision&start,const svn::Revision&end);

signals:
    void clientException(const QString&);
    void dirAdded(const QString&,FileListViewItem*);
    void sendNotify(const QString&);
protected slots:
    virtual void wroteStdin(KProcess*);
    virtual void procClosed(KProcess*);
};

#endif
