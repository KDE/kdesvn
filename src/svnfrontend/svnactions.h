/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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

#include "svnqt/client.h"
#include "svnqt/revision.h"
#include "svnqt/svnqttypes.h"

#include "simple_logcb.h"
#include "frontendtypes.h"

#include <QObject>
#include <QScopedPointer>
#include <QStringList>

class ItemDisplay;
class SvnItem;
class QDialog;
class CContextListener;
class SvnActionsData;
class CheckModifiedThread;
class CheckUpdatesThread;
class FillCacheThread;
class WatchedProcess;

namespace svn
{
class LogEntry;
class InfoEntry;
}

namespace KIO
{
class Job;
}

/**
@author Rajko Albrecht
*/
class SvnActions : public QObject, public SimpleLogCb
{
    Q_OBJECT
public:
    enum ThreadType {
        checkupdatethread,
        fillcachethread,
        checkmodifiedthread
    };

    explicit SvnActions(ItemDisplay *parent, bool processes_blocked = false);
    ~SvnActions();
    void reInitClient();
    svn::ClientP svnclient();
    void prepareUpdate(bool ask);

    bool makeGet(const svn::Revision &start, const QString &what, const QString &target,
                 const svn::Revision &peg = svn::Revision::UNDEFINED, QWidget *dlgparent = nullptr);

    bool addItems(const svn::Paths &items, svn::Depth depth = svn::DepthEmpty);
    void checkAddItems(const QString &path, bool print_error_box = true);

    bool makeDelete(const svn::Targets &target, bool keep_local = true, bool force = false);
    bool makeDelete(const QStringList &);
    void makeLock(const QStringList &, const QString &, bool);
    void makeUnlock(const QStringList &, bool);

    bool makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, bool rec = false, bool all = true);
    bool makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, bool rec, bool all, bool display_ignored, bool updates = false);
    bool makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, svn::Depth depth, bool all, bool display_ignored, bool updates = false);

    bool makeList(const QString &url, svn::DirEntries &dlist, const svn::Revision &where, svn::Depth depth = svn::DepthInfinity);

    bool createModifiedCache(const QString &base);
    bool checkModifiedCache(const QString &path) const;
    bool checkConflictedCache(const QString &path) const;
    bool checkReposLockCache(const QString &path) const;
    bool checkReposLockCache(const QString &path, svn::StatusPtr &t) const;
    void addModifiedCache(const svn::StatusPtr &what);
    void deleteFromModifiedCache(const QString &what);

    bool makeIgnoreEntry(SvnItem *which, bool unignore);
    bool makeIgnoreEntry(const svn::Path &item, const QStringList &ignorePattern, bool unignore);

    bool isLockNeeded(SvnItem *which, const svn::Revision &where);
    QString searchProperty(QString &store, const QString &property, const QString &start, const svn::Revision &where, bool up = false);
    svn::PathPropertiesMapListPtr propList(const QString &which, const svn::Revision &where, bool cacheOnly);

    bool changeProperties(const svn::PropertiesMap &setList, const QStringList &, const QString &path, const svn::Depth &depth = svn::DepthEmpty);

    //! generate and displays a revision tree
    /*!
     * the parameter @a what must prepared, eg, if it comes from working copy
     * it must not contain a "file://" inside.
     * \param what item to display
     * \param rev Revision the item-path is available, intersting only when @a what is a repository item
     * \param startr startrevision for log
     * \param endr endrevision for log
     */
    void makeTree(const QString &what, const svn::Revision &rev,
                  const svn::Revision &startr = svn::Revision(1),
                  const svn::Revision &endr = svn::Revision::HEAD);
    void makeLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &, bool follow, bool list_files = false, int limit = 0);
    svn::LogEntriesMapPtr getLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &, bool list_files, int limit, QWidget *parent = nullptr);
    svn::LogEntriesMapPtr getLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &, bool list_files, int limit, bool follow_nodes, QWidget *parent = nullptr);
    bool getSingleLog(svn::LogEntry &, const svn::Revision &, const QString &, const svn::Revision &, QString &root) override;

    void makeBlame(const svn::Revision &start, const svn::Revision &end, SvnItem *k);
    void makeBlame(const svn::Revision &start, const svn::Revision &end, const QString &, QWidget *parent = nullptr, const svn::Revision &peg = svn::Revision::UNDEFINED, SimpleLogCb *_acb = nullptr);
    void makeUpdate(const svn::Targets &targets, const svn::Revision &rev, svn::Depth depth);
    bool makeSwitch(const QUrl &rUrl, const QString &tPath, const svn::Revision &r, svn::Depth depth, const svn::Revision &peg, bool stickydepth, bool ignore_externals, bool allow_unversioned);
    bool makeSwitch(const QString &path, const QUrl &what);
    bool makeRelocate(const QUrl &fUrl, const QUrl &tUrl, const QString &path, bool recursive, bool ignore_externals);
    bool makeCheckout(const QString &, const QString &, const svn::Revision &, const svn::Revision &, svn::Depth, bool isExport, bool openit, bool ignore_externals, bool overwrite, bool ignoreKeywords, QWidget *p);
    void makeInfo(const SvnItemList &lst, const svn::Revision &, const svn::Revision &, bool recursive = true);
    void makeInfo(const QStringList &lst, const svn::Revision &, const svn::Revision &, bool recursive = true);
    bool makeCommit(const svn::Targets &);
    void CheckoutExport(const QUrl &what, bool _exp, bool urlisTarget = false);

    QString getInfo(const SvnItemList &lst, const svn::Revision &rev, const svn::Revision &peg, bool recursive, bool all = true);
    QString getInfo(const QString &_what, const svn::Revision &rev, const svn::Revision &peg, bool recursive, bool all = true);
    QString getInfo(const svn::InfoEntries &entries, const QString &what, bool all);


    QString makeMkdir(const QString &);
    bool makeMkdir(const svn::Targets &which, const QString &logMessage);
    bool isLocalWorkingCopy(const QString &path, QUrl &repoUrl);
    bool createUpdateCache(const QString &what);
    bool checkUpdateCache(const QString &path)const;
    bool isUpdated(const QString &path)const;
    bool getUpdated(const QString &path, svn::StatusPtr &d)const;
    void clearUpdateCache();
    void removeFromUpdateCache(const QStringList &what, bool exact_only);
    void stopCheckModifiedThread();
    void stopCheckUpdateThread();
    void startFillCache(const QString &path, bool startup = false);
    void stopMain();
    void killallThreads();

    bool checkUpdatesRunning();
    void getaddedItems(const QString &path, svn::StatusEntries &target);

    bool makeCopy(const QString &, const QString &, const svn::Revision &rev);
    bool makeCopy(const QList<QUrl> &, const QString &, const svn::Revision &rev);

    bool makeMove(const QString &, const QString &);
    bool makeMove(const QList<QUrl> &, const QString &);

    virtual bool makeCleanup(const QString &);

    bool get(const QString &what, const QString &to, const svn::Revision &rev, const svn::Revision &peg, QWidget *p);
    bool singleInfo(const QString &what, const svn::Revision &rev, svn::InfoEntry &target, const svn::Revision &_peg = svn::Revision::UNDEFINED);
    bool hasMergeInfo(const QString &originpath);

    void setContextData(const QString &, const QString &);
    void clearContextData();
    QString getContextData(const QString &)const;

    bool threadRunning(ThreadType which) const;

    bool doNetworking();
    virtual void doCommit(const SvnItemList &);
    virtual void editProperties(SvnItem *k, const svn::Revision &rev);

protected:
    QScopedPointer<SvnActionsData> m_Data;

    void showInfo(const QStringList &infoList);
    void CheckoutExportCurrent(bool _exp);
    void makeAdd(bool rec);
    CheckModifiedThread *m_CThread, *m_UThread;
    FillCacheThread *m_FCThread;
    void makeDiffinternal(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *, const svn::Revision &peg = svn::Revision::UNDEFINED);
    void makeDiffExternal(const QString &p1, const svn::Revision &start, const QString &p2, const svn::Revision &end, const svn::Revision &_peg, bool isDir, QWidget *p, bool rec = true);

public Q_SLOTS:
    virtual void dispDiff(const QByteArray &);
    virtual void slotNotifyMessage(const QString &);
    virtual void slotUpdateHeadRec();
    virtual void slotUpdateTo();
    virtual void slotAdd();
    virtual void slotAddRec();
    virtual void slotCheckoutCurrent();
    virtual void slotExportCurrent();
    virtual void slotCheckout();
    virtual void slotExport();
    virtual void slotRevert();
    virtual void slotRevertItems(const QStringList &);
    virtual void slotSwitch();
    virtual void slotResolved(const QString &);
    virtual void slotResolve(const QString &);
    virtual void makeDiff(const QString &, const svn::Revision &, const svn::Revision &, const svn::Revision &_peg, bool isDir);
    virtual void makeDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &);
    virtual void makeDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, const svn::Revision &, bool, QWidget *p);
    virtual void makeDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    virtual void makeNorecDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    virtual void slotImport(const QString &, const QUrl &, const QString &, svn::Depth, bool noIgnore, bool noUnknown);
    virtual void slotMergeWcRevisions(const QString &, const svn::Revision &, const svn::Revision &, bool, bool, bool, bool, bool);
    virtual void slotMerge(const QString &, const QString &, const QString &,
                           const svn::Revision &, const svn::Revision &, const svn::Revision &,
                           bool, bool, bool, bool, bool, bool, bool);
    virtual void slotMergeExternal(const QString &src1, const QString &src2, const QString &target,
                                   const svn::Revision &rev1, const svn::Revision &rev2, const svn::Revision &_peg, bool);
    virtual void slotExtraLogMsg(const QString &);
    virtual void slotMakeCat(const svn::Revision &start, const QString &what, const QString &disp, const svn::Revision &peg, QWidget *dlgparent);

    virtual void slotCancel(bool);
    virtual void stopFillCache();

Q_SIGNALS:
    void clientException(const QString &);
    void sendNotify(const QString &);
    void reinitItem(SvnItem *);
    void sigRefreshAll();
    void sigThreadsChanged();
    void sigRefreshCurrent(SvnItem *);
    void sigExtraLogMsg(const QString &);
    void sigGotourl(const QUrl &);
    void sigCacheStatus(qlonglong, qlonglong);
    void sigCacheDataChanged();
    void sigItemsReverted(const QStringList &);
    void sigExtraStatusMessage(const QString &);
    void sigRefreshItem(const QString &path);

protected Q_SLOTS:
    virtual void checkModifiedThread();
    virtual void checkUpdateThread();
    virtual void slotProcessDataRead(const QByteArray &, WatchedProcess *);
};

#endif
