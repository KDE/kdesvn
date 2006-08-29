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
#ifndef KDESVNFILELIST_H
#define KDESVNFILELIST_H

#include "itemdisplay.h"
#include "filelistviewitem.h"
#include "src/svnqt/status.hpp"
#include "src/svnqt/client.hpp"

#include <klistview.h>
#include <kurl.h>
#include <ktrader.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qevent.h>

class KAction;
class KActionMenu;
class KActionCollection;
class KDialog;
class KDialogBase;
class KdesvnFileListPrivate;
class SvnActions;

namespace KIO {
    class Job;
}

namespace svn {
    class Revision;
}
/**
@author Rajko Albrecht
*/
class kdesvnfilelist : public KListView,public ItemDisplay
{
    Q_OBJECT
    friend class FileListViewItem;
public:
    kdesvnfilelist(KActionCollection*,QWidget *parent = 0, const char *name = 0);
    virtual ~kdesvnfilelist();

    virtual bool openURL( const KURL &url,bool noReinit=false );
    virtual SvnItem*SelectedOrMain();
    virtual SvnItem*Selected();
    virtual void SelectionList(SvnItemList*target);

    virtual QWidget*realWidget();
    const svn::Status&maindir()const{return m_mainEntry;}

    KActionCollection*filesActions();
    bool refreshItem(FileListViewItem*);

protected:
    svn::Status m_mainEntry;
    const svn::Revision& remoteRevision()const;
    bool m_deletePerfect;
    QMap<QString,bool> m_Dirsread;

    KActionCollection* m_filesAction;
    KAction*m_LogFullAction,*m_LogRangeAction,*m_BlameAction,*m_BlameRangeAction,*m_CatAction,*m_MkdirAction;
    KAction*m_InfoAction,*m_propertyAction,*m_commitAction,*m_simpleDiffHead,*m_UpdateHead,*m_UpdateRev;
    KAction*m_AddCurrent,*m_DelCurrent,*m_CheckoutAction,*m_CheckoutCurrentAction,*m_RevertAction;
    KAction*m_changeToRepository,*m_switchRepository,*m_ExportAction,*m_ExportCurrentAction;
    KAction*m_CleanupAction,*m_ResolvedAction,*m_ImportDirsIntoCurrent,*m_RefreshViewAction,*m_MergeRevisionAction;
    KAction*m_RenameAction,*m_CopyAction;
    KAction*m_LockAction,*m_UnlockAction,*m_IgnoreAction;

    SvnActions*m_SvnWrapper;

    /* the parent entry must removed from list before */
    void insertDirs(FileListViewItem * _parent,svn::StatusEntries&);
    bool checkDirs(const QString&,FileListViewItem * _parent);
    void setupActions();
    svn::Client*svnclient();

    void enableActions();

    FileListViewItem* singleSelected();
    FileListViewItemList* allSelected();

    template<class T> KDialogBase* createDialog(T**ptr,
        const QString&_head,
        bool OkCancel=false,
        const char*name="dialog"
        );

    FileListViewItemList* m_SelectedItems;
    FileListViewItem* findEntryItem(const QString&,FileListViewItem*startAt=0);

    virtual void refreshRecursive(FileListViewItem*,bool down=true);
    virtual void updateParents(FileListViewItem*);
    virtual void checkUnversionedDirs( FileListViewItem * _parent );

    /**
     * Overridden virtuals for Qt drag 'n drop (XDND)
     */
    virtual void contentsDragEnterEvent( QDragEnterEvent* );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent* );
    virtual void contentsDragMoveEvent( QDragMoveEvent* );
    virtual void contentsDropEvent( QDropEvent* );
    virtual bool acceptDrag(QDropEvent *event)const;
    virtual void startDrag();

    void dispDummy();
    void reinitItems(FileListViewItem*_item = 0);
    KURL::List selectedUrls();

    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void contentsWheelEvent( QWheelEvent * e );
    virtual void leaveEvent(QEvent*e);
    virtual void rescanIconsRec(FileListViewItem*_parent=0,bool checkNewer=false,bool no_update=false);

    KTrader::OfferList offersList(SvnItem*item,bool execOnly=false);

private:
    KdesvnFileListPrivate*m_pList;
    void cleanHighLighter();
    bool validDropEvent(QDropEvent*event,QListViewItem*&item);
    void copy_move(bool move);

protected slots:
    virtual void slotSelectBrowsingRevision();
    virtual void slotItemRead(QListViewItem*);
    virtual void slotContextMenuRequested(QListViewItem *, const QPoint &, int);
    virtual void slotSelectionChanged();
    virtual void slotClientException(const QString&);
    virtual void slotNotifyMessage(const QString&);
    virtual void slotDirAdded(const QString&,FileListViewItem*);
    virtual void slotReinitItem(SvnItem*);
    virtual void slotItemDoubleClicked(QListViewItem*);
    virtual void slotImportIntoCurrent(bool);
    virtual void slotImportDirsIntoCurrent();
    virtual void slotImportIntoDir(const KURL&,const QString&,bool);

    /* subversion slots */
    virtual void slotChangeToRepository();
    virtual void slotCleanupAction();
    virtual void slotResolved();
    virtual void slotMergeRevisions();
    virtual void slotMerge();
    virtual void slotDropped(QDropEvent *,QListViewItem*);
    virtual void viewportPaintEvent(QPaintEvent *);
    virtual void slotRename();
    virtual void slotCopy();
    virtual void slotCat();
    virtual void slotDelete();

    /* callback slots */
    virtual void slotCopyFinished( KIO::Job *);
    virtual void slotDeleteFinished(KIO::Job*);
    virtual void _openURL(const QString&);

signals:
    void sigLogMessage(const QString&);
    void changeCaption(const QString&);
    void sigShowPopup(const QString&,QWidget**);
    void sigUrlOpend(bool);
    void sigSwitchUrl(const KURL&);
    void sigUrlChanged(const QString&);

public slots:
    virtual void refreshCurrentTree();
    virtual void refreshCurrent(SvnItem*);
    virtual void closeMe();
    virtual void slotMkdir();
    virtual void slotMkBaseDirs();
    virtual void slotSettingsChanged();

protected slots:
    virtual void slotLock();
    virtual void slotUnlock();
    virtual void slotIgnore();
    virtual void slotBlame();
    virtual void slotRangeBlame();
    virtual void slotSimpleHeadDiff();
    virtual void slotSimpleBaseDiff();

    virtual void slotDiffRevisions();
    virtual void slotRevisionCat();
    virtual void slotCheckUpdates();
    virtual void slotInfo();
    virtual void slotDirItemCreated(const QString&);
    virtual void slotDirItemDirty(const QString&);
    virtual void slotDirItemDeleted(const QString&);
    virtual void slotRelocate();
    virtual void slotRescanIcons(bool);
    virtual void slotCheckNewItems();
    virtual void slotMakeRangeLog();
    virtual void slotMakeLog();
    virtual void slotMakeTree();
    virtual void slotMakePartTree();
    virtual void slotInternalDrop();
    virtual void slotOpenWith();

private slots:
    void gotPreview( const KFileItem*, const QPixmap& );
    void gotPreviewResult();
};

#endif
