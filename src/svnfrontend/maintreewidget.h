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

#include "src/svnfrontend/ui_treeWidget.h"
#include "itemdisplay.h"
#include "frontendtypes.h"

#include "src/svnqt/status.h"
#include "src/svnqt/client.h"

#include <kservice.h>
#include <krun.h>

class KActionCollection;
class MainTreeWidgetData;
class KIcon;
class KShortcut;
class KAction;
class KActionCollection;
class SvnItemModelNode;
class KDialog;
class KJob;

class MainTreeWidget:public QWidget,public Ui::mainTreeWidget,public ItemDisplay
{
    Q_OBJECT
public:
    explicit MainTreeWidget(KActionCollection*aCollection,QWidget*parent=0,Qt::WindowFlags f = 0);
    virtual ~MainTreeWidget();

    virtual bool openUrl(const KUrl &url,bool noReinit=false);
    virtual QWidget* realWidget();
    virtual SvnItem* Selected()const;
    virtual SvnItem* DirSelected()const;
    virtual QModelIndex SelectedIndex()const;
    virtual QModelIndex DirSelectedIndex()const;
    SvnItemModelNode*SelectedNode()const;
    SvnItemModelNode*DirSelectedNode()const;
    virtual void SelectionList(SvnItemList&target)const;
    virtual void DirSelectionList(SvnItemList&target)const;
    virtual SvnItem* SelectedOrMain()const;
    virtual SvnItem* DirSelectedOrMain()const;
    virtual const svn::Revision&baseRevision()const;
    void refreshItem(SvnItemModelNode*node);

    void clear();
    KActionCollection*filesActions();

Q_SIGNALS:
    void sigLogMessage(const QString&);
    void sigExtraStatusMessage(const QString&);
    void changeCaption(const QString&);
    void sigShowPopup(const QString&,QWidget**);
    void sigUrlOpend(bool);
    void sigSwitchUrl(const KUrl&);
    void sigUrlChanged(const QString&);
    void sigProplist(const svn::PathPropertiesMapListPtr&,bool,bool,const QString&);
    void sigListError();
    void sigCacheStatus(qlonglong,qlonglong);

public Q_SLOTS:
    virtual void closeMe();
    virtual void refreshCurrentTree();
    virtual void slotSettingsChanged();
    virtual void slotSelectionChanged(const QItemSelection&,const QItemSelection&);
    virtual void slotNotifyMessage(const QString&);
    virtual void slotMkBaseDirs();
    virtual void slotMkdir();
    virtual void refreshCurrent(SvnItem*);
    virtual void slotReinitItem(SvnItem*);
    virtual void stopLogCache();

protected Q_SLOTS:
    void slotCacheDataChanged();
    void slotItemActivated(const QModelIndex&);
    void slotItemExpanded(const QModelIndex&);
    void slotItemsInserted(const QModelIndex&);
    void slotRescanIcons();
    void _propListTimeout();

    void slotCheckUpdates();
    void slotCheckModified();
    void readSupportData();
    void slotClientException(const QString&);

    void slotIgnore();
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
    void slotChangeProperties(const svn::PropertiesMap&,const QStringList&,const QString&);
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
    void slotImportIntoDir(const KUrl&,const QString&,bool);
    void slotChangeToRepository();
    void slotCheckNewItems();

    void slotCommit();
    void slotDirCommit();
    void slotDirUpdate();

    void slotDirSelectionChanged(const QItemSelection&,const QItemSelection&);

    void _openUrl(const QString&);
    void enableActions();
    void slotUnfoldTree();
    void slotFoldTree();
    void slotOpenWith();

    void slotContextMenu(const QPoint&);
    void slotDirContextMenu(const QPoint&);
    void slotCopyFinished(KJob*job);
    void slotUpdateLogCache();

    void slotUrlDropped(const KUrl::List&,Qt::DropAction,const QModelIndex&,bool);
    void slotRepositorySettings();

    void slotRightProperties();
    void slotLeftProperties();

protected:
    virtual void keyPressEvent(QKeyEvent*);
    void setupActions();
    bool uniqueTypeSelected();
    KService::List offersList(SvnItem*item,bool execOnly=false)const;
    int selectionCount()const;
    int DirselectionCount()const;
    void dispProperties(bool);
    void copy_move(bool move);
    void itemActivated(const QModelIndex&index,bool keypress=false);

    void internalDrop(const KUrl::List&_lst,Qt::DropAction action,const QModelIndex&index);
    void resizeAllColumns();
    void execContextMenu(const SvnItemList&);
    void simpleWcDiff(SvnItem*which,const svn::Revision&,const svn::Revision&);
    void doLog(bool,bool)const;

    void checkUseNavigation(bool startup = false);
    void makeDelete(const SvnItemList&lst);

private:
    MainTreeWidgetData*m_Data;
    void enableAction(const QString&,bool);

    KAction* add_action(const QString&actionname,
        const QString&text,
        const KShortcut&sequ,
        const KIcon&,
        QObject*,
        const char*slot);
};

#endif
