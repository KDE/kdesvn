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
#include <kdesvniface.h>

class QPainter;
class KURL;
class kdesvnfilelist;
class KdeSvnDirList;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QSplitter;


/**
 * This is the main view class for kdesvn.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * This kdesvn uses an HTML component as an example.
 *
 * @short Main view
 * @author Rajko Albrecht <ral@alwins-world.de>
 * @version 0.1
 */
class kdesvnView : public QWidget, public kdesvnIface
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnView(QWidget *parent);

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
    virtual void openURL(QString url);

    /**
     * Random 'set' function
     */
    virtual void openURL(const KURL& url);

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

private slots:
    void slotOnURL(const QString& url);
    void slotSetTitle(const QString& title);

private:
    //KParts::ReadOnlyPart *m_html;
    kdesvnfilelist*m_flist;
    KdeSvnDirList*m_LeftList;
    QSplitter* m_Splitter;
};

#endif // _KDESVNVIEW_H_
