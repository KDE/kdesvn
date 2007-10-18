/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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


#ifndef _KDESVN_H_
#define _KDESVN_H_

#include "kdesvn-config.h"

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <qtimer.h>
#include <kbookmarkmanager.h>

class KUrl;
class KAction;
class KActionMenu;
class KToggleAction;
class KBookmarkMenu;

/**
 * This class serves as the main window for kdesvn.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version $Rev$
 */
class kdesvn : public KParts::MainWindow,public KBookmarkOwner
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
    virtual ~kdesvn();

    virtual void openBookmarkURL (const QString &_url);
    virtual QString currentURL () const;
    void checkReload();

protected:
    /**
     * This function is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This function is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);
    virtual bool queryExit();
    void enableClose(bool how);


public slots:
    virtual void slotUrlOpened(bool);
    /**
     * Use this method to load whatever file/URL you have
     */
    virtual void load(const KUrl&_url) {
        load(_url,true);
    }
    virtual void loadRescent(const KURL&);
    virtual void load(const KURL&,bool);

private slots:
    void fileOpen();
    void fileNew();
    void fileClose();
    void optionsShowStatusbar();
    void changeStatusbar(const QString&);
    void resetStatusBar();

private:
    void setupAccel();
    void setupActions();
    void connectActionCollection( KActionCollection *coll );
    void disconnectActionCollection( KActionCollection *coll );

    KActionMenu *m_FileMenu;
    QString m_bookmarkFile;
    KBookmarkManager * m_BookmarkManager;
    KActionMenu* m_BookmarksActionmenu;
    KActionCollection*m_Bookmarkactions;
    KBookmarkMenu * m_pBookmarkMenu;
    KParts::ReadOnlyPart *m_part;
    KToggleAction *m_statusbarAction;

protected slots:
    virtual void optionsConfigureToolbars();
    virtual void optionsConfigureKeys();
    virtual void applyNewToolbarConfig();
    virtual void slotLoadLast(bool);
};

#endif // _KDESVN_H_
