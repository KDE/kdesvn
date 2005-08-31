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

#ifndef _KDESVNPART_H_
#define _KDESVNPART_H_

#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kparts/factory.h>
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>

class kdesvnView;
class QPainter;
class KURL;
class KdesvnBrowserExtension;
class KAboutApplication;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Rajko Albrecht <rajko.albrecht@tecways.com>
 * @version 0.1
 */
class kdesvnPart : public KParts::ReadOnlyPart
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
    virtual void slotDispPopup(const QString&);
    virtual void slotFileProperties();
    virtual bool openURL(const KURL&);

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();
    virtual void setupActions();
    KAboutApplication* m_aboutDlg;

protected slots:
    virtual void slotLogFollowNodes(bool);
    virtual void slotDisplayIgnored(bool);
    virtual void slotDisplayUnkown(bool);
    virtual void slotUseKompare(bool);
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
};

typedef KParts::GenericFactory<kdesvnPart> kdesvnPartFactory;

class KdesvnBrowserExtension : public KParts::BrowserExtension
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
