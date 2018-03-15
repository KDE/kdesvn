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

#ifndef KDESVN_H
#define KDESVN_H

#include "kdesvn-config.h"

#include <kparts/mainwindow.h>
#include <kparts/readonlypart.h>
#include <KBookmarks/KBookmarkManager>
#include <KBookmarks/KBookmarkMenu>
#include <KBookmarks/KBookmarkActionMenu>

class KAction;
class KActionMenu;
class KToggleAction;

/**
 * This class serves as the main window for kdesvn.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version $Rev$
 */
class kdesvn : public KParts::MainWindow, public KBookmarkOwner
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    kdesvn();

    /**
     * Default Destructor
     */
    ~kdesvn();

    void openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) override;
    QUrl currentUrl() const override;
    QString currentTitle() const override;
    void checkReload();

protected:
    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &) override;

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &) override;
    void closeEvent(QCloseEvent *event) override;
    void enableClose(bool how);

public Q_SLOTS:
    virtual void slotUrlOpened(bool);
    /**
     * Use this method to load whatever file/URL you have
     */
    void load(const QUrl &_url)
    {
        load(_url, true);
    }
    void loadRescent(const QUrl &);
    void load(const QUrl &, bool);

private slots:
    void fileOpen();
    void fileNew();
    void fileClose();
    void optionsShowStatusbar();
    void changeStatusbar(const QString &);
    void resetStatusBar();

private:
    void setupAccel();
    void setupActions();

    KActionMenu *m_FileMenu;
    QString m_bookmarkFile;
    KBookmarkManager *m_BookmarkManager;
    KBookmarkActionMenu *m_BookmarksActionmenu;
    KActionCollection *m_Bookmarkactions;
    KBookmarkMenu *m_pBookmarkMenu;
    KParts::ReadOnlyPart *m_part;
    KToggleAction *m_statusbarAction;

protected Q_SLOTS:
    virtual void optionsConfigureToolbars();
    virtual void optionsConfigureKeys();
    virtual void applyNewToolbarConfig();
    virtual void slotLoadLast(bool);
    virtual void slotExtraStatus(const QString &);
    virtual void slotResetExtraStatus();

Q_SIGNALS:
    void sigSavestate();
};

#endif // _KDESVN_H_
