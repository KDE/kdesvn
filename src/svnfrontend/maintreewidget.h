/***************************************************************************
 *   Copyright (C) 2008 by Rajko Albrecht  ral@alwins-world.de             *
 *   http://kdesvn.alwins-world.de/                                        *
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
#ifndef MAINTREEWIDGET_H
#define MAINTREEWIDGET_H

#include "ui_treeWidget.h"
#include "itemdisplay.h"
#include "frontendtypes.h"

#include "svnqt/status.h"
#include "svnqt/client.h"

#include <kservice.h>
#include <krun.h>

class KActionCollection;
class MainTreeWidgetData;
class SvnItemModelNode;
class KJob;

class MainTreeWidget: public QWidget, public Ui::mainTreeWidget, public ItemDisplay
{
    Q_OBJECT
public:
    explicit MainTreeWidget(KActionCollection *aCollection, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
    ~MainTreeWidget();

    QWidget *realWidget() override;
    SvnItem *Selected()const override;
    SvnItemList SelectionList()const override;
    svn::Revision baseRevision()const override;
    bool openUrl(const QUrl &url, bool noReinit = false) override;
    SvnItem *SelectedOrMain()const override;

    SvnItem *DirSelected()const;
    QModelIndex SelectedIndex()const;
    QModelIndex DirSelectedIndex()const;
    SvnItemModelNode *SelectedNode()const;
    SvnItemModelNode *DirSelectedNode()const;
    SvnItemList DirSelectionList()const;
    SvnItem *DirSelectedOrMain()const;
    void refreshItem(SvnItemModelNode *node);

    void clear();
    KActionCollection *filesActions();

Q_SIGNALS:
    void sigLogMessage(const QString &);
    void sigExtraStatusMessage(const QString &);
    void changeCaption(const QString &);
    void sigShowPopup(const QString &, QWidget **);
    void sigUrlOpend(bool);
    void sigSwitchUrl(const QUrl &);
    void sigUrlChanged(const QUrl &);
    void sigProplist(const svn::PathPropertiesMapListPtr &, bool, bool, const QString &);
    void sigListError();
    void sigCacheStatus(qlonglong, qlonglong);

public Q_SLOTS:
    void closeMe();
    void refreshCurrentTree();
    void slotSettingsChanged();
    void slotSelectionChanged(const QItemSelection &, const QItemSelection &);
    void slotNotifyMessage(const QString &);
    void slotMkBaseDirs();
    void slotMkdir();
    void refreshCurrent(SvnItem *);
    void slotReinitItem(SvnItem *);
    void stopLogCache();

protected Q_SLOTS:
    void slotCacheDataChanged();
    void slotItemActivated(const QModelIndex &);
    void slotItemExpanded(const QModelIndex &);
    void slotItemsInserted(const QModelIndex &);
    void slotRefreshItem(const QString &path);
    void _propListTimeout();

    void slotCheckUpdates();
    void slotCheckModified();
    void readSupportData();
    void slotClientException(const QString &);

    void slotIgnore();
    void slotLeftRecAddIgnore();
    void slotRightRecAddIgnore();
    void slotMakeLog()const;
    void slotMakeLogNoFollow()const;
    void slotDirMakeLogNoFollow()const;
    void slotMakeTree();
    void slotMakePartTree();
    void slotSelectBrowsingRevision();
    void slotLock();
    void slotUnlock();
    void slotDisplayLastDiff();
    void slotSimpleHeadDiff();
    void slotSimpleBaseDiff();
    void slotDirSimpleBaseDiff();
    void slotDiffRevisions();
    void slotDiffPathes();
    void slotInfo();
    void slotBlame();
    void slotRangeBlame();
    void slotDisplayProperties();
    void slotChangeProperties(const svn::PropertiesMap &, const QStringList &, const QString &);
    void slotCat();
    void slotRevisionCat();
    void slotResolved();
    void slotTryResolve();
    void slotDelete();
    void slotLeftDelete();
    void slotRename();
    void slotCopy();
    void slotCleanupAction();
    void slotMergeRevisions();
    void slotMerge();
    void slotRelocate();
    void slotImportIntoCurrent(bool);
    void slotImportDirsIntoCurrent();
    void slotImportIntoDir(const QString &source, const QUrl &_targetUri, bool dirs);
    void slotChangeToRepository();
    void slotCheckNewItems();

    void slotCommit();
    void slotDirCommit();
    void slotDirUpdate();
    void slotDirRecProperty();

    void slotDirSelectionChanged(const QItemSelection &_item, const QItemSelection &);
    void checkSyncTreeModel();

    void _openUrl(const QUrl &);
    void enableActions();
    void slotUnfoldTree();
    void slotFoldTree();
    void slotOpenWith();

    void slotContextMenu(const QPoint &);
    void slotDirContextMenu(const QPoint &);
    void slotCopyFinished(KJob *job);
    void slotUpdateLogCache();

    void slotUrlDropped(const QList<QUrl> &, Qt::DropAction, const QModelIndex &, bool);
    void slotRepositorySettings();

    void slotRightProperties();
    void slotLeftProperties();

    void resizeAllColumns();
protected:
    void keyPressEvent(QKeyEvent *) override;
    void setupActions();
    bool uniqueTypeSelected();
    KService::List offersList(SvnItem *item, bool execOnly = false)const;
    int selectionCount()const;
    int DirselectionCount()const;
    void dispProperties(bool);
    void copy_move(bool move);
    void itemActivated(const QModelIndex &index, bool keypress = false);

    void internalDrop(const QList<QUrl> &_lst, Qt::DropAction action, const QModelIndex &index);
    void execContextMenu(const SvnItemList &);
    void simpleWcDiff(SvnItem *which, const svn::Revision &, const svn::Revision &);
    void doLog(bool, bool)const;

    void checkUseNavigation(bool startup = false);
    void makeDelete(const SvnItemList &lst);

    void recAddIgnore(SvnItem *which);

private:
    MainTreeWidgetData *m_Data;
    void enableAction(const QString &, bool);

    QAction *add_action(const QString &actionname,
                        const QString &text,
                        const QKeySequence &sequ,
                        const QIcon &,
                        QObject *,
                        const char *slot);
};

#endif
