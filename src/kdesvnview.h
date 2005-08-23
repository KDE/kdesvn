/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef _KDESVNVIEW_H_
#define _KDESVNVIEW_H_

#include <qwidget.h>
#include <kparts/part.h>

class QPainter;
class KURL;
class kdesvnfilelist;
class KdeSvnDirList;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QSplitter;
class KActionCollection;
class KTextBrowser;


/**
 * This is the main view class for kdesvn.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version 0.1
 */
class kdesvnView : public QWidget
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnView(KActionCollection*,QWidget *parent,const char*name=0);

    /**
     * Destructor
     */
    virtual ~kdesvnView();

    /**
     * Random 'get' function
     */
    QString currentURL();

    /**
     * Random 'set' function accessed by DCOP
     */
    virtual bool openURL(QString url);

    /**
     * Random 'set' function
     */
    virtual bool openURL(const KURL& url);

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString&);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString&);

    void sigShowPopup(const QString&);
    void sigSwitchUrl(const KURL&);

public slots:
    virtual void closeMe();
    virtual void slotDispPopup(const QString&);
    virtual void refreshCurrentTree();

protected slots:
    virtual void slotOnURL(const QString& url);
    virtual void slotSetTitle(const QString& title);
    virtual void slotAppendLog(const QString& text);

protected:
    kdesvnfilelist*m_flist;
    KActionCollection*m_Collection;

    QSplitter* m_Splitter;
    QString m_currentURL;
    KTextBrowser*m_LogWindow;
};

#endif // _KDESVNVIEW_H_
