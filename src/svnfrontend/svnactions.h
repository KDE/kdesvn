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
#include <qstringlist.h>
#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"

class ItemDisplay;
class SvnItem;
class KDialog;
class KDialogBase;
class QDialog;
class CContextListener;
class KProcess;

namespace svn {
    class Context;
}

namespace KIO {
    class Job;
}

/**
@author Rajko Albrecht
*/
class SvnActions : public QObject
{
    Q_OBJECT
public:
    SvnActions(ItemDisplay *parent, const char *name = 0);
    ~SvnActions();
    void reInitClient();
    //svn::Client&svnClient(){return m_Svnclient;}
    svn::Client* svnclient(){return &m_Svnclient;}
    void prepareUpdate(bool ask);
    template<class T> KDialogBase* createDialog(T**ptr,const QString&_head,bool OkCance=false,const char*name="standard_dialog");
    void makeCat(svn::Revision start, const QString&what,const QString&disp);
    void addItems(const QValueList<svn::Path> &items,bool rec=false);
    void makeDelete(const std::vector<svn::Path>&);
    void makeLock(const QStringList&,const QString&,bool);
    void makeUnlock(const QStringList&,bool);
    bool makeStatus(const QString&what, svn::StatusEntries&dlist);
    bool makeIgnoreEntry(SvnItem*which,bool unignore);
    void makeLog(svn::Revision start,svn::Revision end,SvnItem*k);
    void makeBlame(svn::Revision start, svn::Revision end, SvnItem*k);
    void makeUpdate(const QStringList&what,const svn::Revision&rev,bool recurse);
    void makeSwitch(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool rec = true);
    void makeCheckout(const QString&,const QString&,const svn::Revision&,bool,bool,bool);
    QString makeMkdir(const QString&);

protected:
    ItemDisplay* m_ParentList;

    CContextListener*m_SvnContext;
    svn::Context* m_CurrentContext;
    svn::Client m_Svnclient;

    void CheckoutExport(bool _exp);
    void CheckoutExportCurrent(bool _exp);

public slots:
    virtual void slotMakeRangeLog();
    virtual void slotMakeLog();
    virtual void slotInfo();
    virtual void slotProperties();
    virtual void slotNotifyMessage(const QString&);
    virtual void slotCommit();
    virtual void slotUpdateHeadRec();
    virtual void slotUpdateTo();
    virtual void slotAdd();
    virtual void slotCheckoutCurrent();
    virtual void slotExportCurrent();
    virtual void slotCheckout();
    virtual void slotExport();
    virtual void slotRevert();
    virtual void slotRevertItems(const QStringList&);
    virtual void slotSwitch();
    virtual void slotCleanup(const QString&);
    virtual void slotResolved(const QString&);
    virtual void makeDiff(const QString&,const svn::Revision&,const svn::Revision&);
    virtual void slotImport(const QString&,const QString&,const QString&,bool);
    virtual void slotMergeWcRevisions(const QString&,const svn::Revision&,const svn::Revision&,bool,bool,bool,bool);
    virtual void slotCopyMove(bool,const QString&,const QString&,bool);

signals:
    void clientException(const QString&);
    void sendNotify(const QString&);
    void reinitItem(SvnItem*);
    void sigRefreshAll();
    void sigRefreshCurrent(SvnItem*);

protected slots:
    virtual void wroteStdin(KProcess*);
    virtual void procClosed(KProcess*);
};

#endif
