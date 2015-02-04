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

#ifndef KDESVNVIEW_H
#define KDESVNVIEW_H

#include "src/svnqt/repositorylistener.h"
#include <qwidget.h>
#include <kparts/part.h>

class QPainter;
class KUrl;
class KdeSvnDirList;
class QVBoxLayout;
class QSpacerItem;
class QSplitter;
class KActionCollection;
class KTextBrowser;
class QProgressBar;
class MainTreeWidget;

/**
 * This is the main view class for kdesvn.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version 0.1
 */
class kdesvnView : public QWidget, public svn::repository::RepositoryListener
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnView(KActionCollection *, QWidget *parent, bool full = false);

    /**
     * Destructor
     */
    virtual ~kdesvnView();

    /**
     * Random 'get' function
     */
    QString currentUrl();

    /**
     * Random 'set' function
     */
    virtual bool openUrl(const KUrl &url);

    /* repositorylistener methods */
    virtual void sendWarning(const QString &);
    virtual void sendError(const QString &);
    virtual bool isCanceld();
    virtual void stopCacheThreads();

Q_SIGNALS:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString &);
    /**
     * Extra messages for a temporary status bar
     */
    void sigExtraStatusMessage(const QString &);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString &);

    void sigShowPopup(const QString &, QWidget **);
    void sigSwitchUrl(const KUrl &);
    void setWindowCaption(const QString &);
    void sigUrlChanged(const QString &);
    void sigMakeBaseDirs();

    /* repositorylistener methods */
    void tickProgress();
    void waitShow(bool);

public Q_SLOTS:
    virtual void closeMe();
    virtual void slotDispPopup(const QString &, QWidget **);
    virtual void refreshCurrentTree();
    virtual void slotSettingsChanged();
    virtual void slotCreateRepo();
    virtual void slotDumpRepo();
    virtual void slotHotcopy();
    virtual void slotLoaddump();

    /* repositorylistener methods */
    virtual void setCanceled(bool);
    virtual void fillCacheStatus(qlonglong, qlonglong);

    virtual void slotSavestate();

protected Q_SLOTS:
    virtual void slotOnURL(const QString &url);
    virtual void slotSetTitle(const QString &title);
    virtual void slotAppendLog(const QString &text);
    virtual void slotUrlChanged(const QString &);

protected:
    //kdesvnfilelist*m_flist;
    MainTreeWidget *m_TreeWidget;
    KActionCollection *m_Collection;

    QSplitter *m_Splitter, *m_infoSplitter;
    QString m_currentUrl;
    KTextBrowser *m_LogWindow;
    QVBoxLayout *m_topLayout;
    QProgressBar *m_CacheProgressBar;

protected:
    virtual void setupActions();
    bool m_ReposCancel;
};

#endif // _KDESVNVIEW_H_
