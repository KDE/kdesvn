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

#ifndef _KDESVNPART_H_
#define _KDESVNPART_H_

#include "kdesvn-config.h"

#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kparts/factory.h>
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>

class kdesvnView;
class QPainter;
class KUrl;
class KdesvnBrowserExtension;
class K3AboutApplication;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Rajko Albrecht <rajko.albrecht@tecways.com>
 * @version 0.1
 */
class KDESVN_EXPORT kdesvnPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnPart(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList&);

    /**
     * Destructor
     */
    virtual ~kdesvnPart();
    virtual bool closeURL();
    static KAboutData* createAboutData();

signals:
    void refreshTree();
    void settingsChanged();

public slots:
    virtual void slotDispPopup(const QString&,QWidget**target);
    virtual void slotFileProperties();
    virtual bool openURL(const KUrl&);
    virtual void slotSshAdd();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();
    virtual void setupActions();
    K3AboutApplication* m_aboutDlg;

protected slots:
    virtual void slotLogFollowNodes(bool);
    virtual void slotDisplayIgnored(bool);
    virtual void slotDisplayUnkown(bool);
    virtual void slotUseKompare(bool);
    virtual void slotUrlChanged(const QString&);
    void reportBug();
    void showAboutApplication();
    void appHelpActivated();
    virtual void slotShowSettings();

private:
    kdesvnView *m_view;
    KdesvnBrowserExtension*m_browserExt;
    static QString m_Extratext;
protected slots:
    void slotSettingsChanged();
protected slots:
    virtual void slotHideUnchanged(bool);
};

class commandline_part;
class KCmdLineArgs;

/* we make it ourself 'cause we will enhance a little bit! */
class KDESVN_EXPORT cFactory : public KParts::Factory
{
    Q_OBJECT
public:
    cFactory():KParts::Factory(){}
    virtual ~cFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    virtual commandline_part*createCommandIf(QObject*parent,const char*name, KCmdLineArgs *args);

    static KInstance* instance();

private:
    static KInstance* s_instance;
    static KAboutData* s_about;
    static commandline_part*s_cline;
};

typedef cFactory kdesvnPartFactory;

class KPARTS_EXPORT KdesvnBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    KdesvnBrowserExtension( kdesvnPart * );
    virtual ~KdesvnBrowserExtension();
    void setPropertiesActionEnabled(bool enabled);

public slots:
    void properties();
};

#endif // _KDESVNPART_H_
