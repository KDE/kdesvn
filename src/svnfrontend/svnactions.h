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
#ifndef SVNACTIONS_H
#define SVNACTIONS_H

#include "svncpp/client.hpp"
#include "svncpp/revision.hpp"
#include "helpers/smart_pointer.h"
#include "kurl.h"

#include <qobject.h>
#include <qdatetime.h>
#include <qstringlist.h>

class ItemDisplay;
class SvnItem;
class KDialog;
class KDialogBase;
class QDialog;
class CContextListener;
class KProcess;
class SvnActionsData;

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
    svn::Client* svnclient();
    void prepareUpdate(bool ask);
    template<class T> KDialogBase* createDialog(T**ptr,const QString&_head,bool OkCance=false,const char*name="standard_dialog");
    void makeCat(svn::Revision start, const QString&what,const QString&disp);
    QByteArray makeGet(svn::Revision start, const QString&what);
    void addItems(const QValueList<svn::Path> &items,bool rec=false);
    void makeDelete(const QValueList<svn::Path>&);
    void makeLock(const QStringList&,const QString&,bool);
    void makeUnlock(const QStringList&,bool);

    bool makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where, bool rec=false,bool all=true);
    bool makeStatus(const QString&what, svn::StatusEntries&dlist, svn::Revision&where, bool rec,bool all,bool display_ignored,bool updates=false);
    bool createModifiedCache(const QString&base);
    void checkModifiedCache(const QString&path,svn::StatusEntries&dlist);

    bool makeIgnoreEntry(SvnItem*which,bool unignore);
    void makeLog(svn::Revision start,svn::Revision end,SvnItem*k);
    void makeBlame(svn::Revision start, svn::Revision end, SvnItem*k);
    void makeUpdate(const QStringList&what,const svn::Revision&rev,bool recurse);
    void makeSwitch(const QString&rUrl,const QString&tPath,const svn::Revision&r,bool rec = true);
    void makeCheckout(const QString&,const QString&,const svn::Revision&,bool,bool,bool);
    void makeInfo(QPtrList<SvnItem> lst,const svn::Revision&,const svn::Revision&,bool recursive = true);
    QString makeMkdir(const QString&);
    bool isLocalWorkingCopy(const KURL&url);
    bool createUpdateCache(const QString&what);
    void checkUpdateCache(const QString&path,svn::StatusEntries&dlist)const;
    bool isUpdated(const QString&path)const;
    void clearUpdateCache();
    void removeFromUpdateCache(const QStringList&what,bool exact_only);

protected:
    smart_pointer<SvnActionsData> m_Data;

    void CheckoutExport(bool _exp);
    void CheckoutExportCurrent(bool _exp);

public slots:
    virtual void slotMakeRangeLog();
    virtual void slotMakeLog();
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
