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

#ifndef KDESVN_PART_H
#define KDESVN_PART_H

#include "kdesvn-config.h"

#include <kparts/readonlypart.h>
#include <kparts/readwritepart.h>
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>

#include <KAboutData>

class kdesvnView;
class KdesvnBrowserExtension;
class KAboutApplicationDialog;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version 0.1
 */
class kdesvnpart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnpart(QWidget *parentWidget,
               QObject *parent, const QVariantList &args = QVariantList());

    kdesvnpart(QWidget *parentWidget,
               QObject *parent, bool ownapp, const QVariantList &args = QVariantList());

    /**
     * Destructor
     */
    ~kdesvnpart();
    bool closeUrl() override;

Q_SIGNALS:
    void refreshTree();
    void settingsChanged();

public slots:
    virtual void slotDispPopup(const QString &, QWidget **target);
    virtual void slotFileProperties();
    bool openUrl(const QUrl &) override;
    virtual void slotSshAdd();
    virtual void showDbStatus();

protected:
    /**
     * This must be implemented by each part
     */
    bool openFile() override;
    virtual void setupActions();
    KAboutApplicationDialog *m_aboutDlg;

    void init(QWidget *parentWidget, bool full);

protected slots:
    virtual void slotLogFollowNodes(bool);
    virtual void slotDisplayIgnored(bool);
    virtual void slotDisplayUnkown(bool);
    void slotUrlChanged(const QUrl &);
    void showAboutApplication();
    void appHelpActivated();
    virtual void slotShowSettings();

private:
    kdesvnView *m_view;
    KdesvnBrowserExtension *m_browserExt;

protected slots:
    void slotSettingsChanged(const QString &);

protected slots:
    virtual void slotHideUnchanged(bool);
    virtual void slotEnableNetwork(bool);
};

class commandline_part;
class KCmdLineArgs;

class KdesvnBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    explicit KdesvnBrowserExtension(kdesvnpart *);
    ~KdesvnBrowserExtension();

public slots:
    void properties();
};

#endif // _KDESVNPART_H_
