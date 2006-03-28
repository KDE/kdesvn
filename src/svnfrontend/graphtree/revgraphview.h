/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#ifndef REVGRAPHVIEW_H
#define REVGRAPHVIEW_H

#include <qcanvas.h>

namespace svn {
    class LogEntry;
}

class KTempFile;
class KProcess;
class RevisionTree;
class GraphTreeLabel;
class GraphViewTip;
class PannerView;

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class RevGraphView : public QCanvasView
{
    Q_OBJECT
public:
    enum ZoomPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };
    /* avoid large copy operations */
    friend class RevisionTree;

    RevGraphView(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
    virtual ~RevGraphView();

    void showText(const QString&s);
    void clear();

    void beginInsert();
    void endInsert();

    struct targetData {
        char Action;
        QString key;
        targetData(const QString&n,char _a)
        {
            key = n;
            Action = _a;
        }
        targetData(){Action=0;key="";}
    };
    typedef QValueList<targetData> tlist;

    struct keyData {
        QString name,Author,Date,Message;
        long rev;
        char Action;
        tlist targets;
    };

    typedef QMap<QString,keyData> trevTree;

    QString toolTip(const QString&nodename)const;

public slots:
    virtual void contentsMovingSlot(int,int);
    virtual void zoomRectMoved(int,int);
    virtual void zoomRectMoveFinished();

protected slots:
    virtual void readDotOutput(KProcess *   proc,char *   buffer,int   buflen);
    virtual void dotExit(KProcess*);

protected:
    QCanvas*m_Canvas;
    KTempFile*dotTmpFile;
    QString dotOutput;
    KProcess*renderProcess;
    trevTree m_Tree;
    void dumpRevtree();
    QColor getBgColor(const QString&nodeName);
    bool isStart(const QString&nodeName);

    QMap<QString,GraphTreeLabel*> m_NodeList;

    int _xMargin,_yMargin;
    GraphViewTip*m_Tip;
    PannerView*m_CompleteView;
    double _cvZoom;
    ZoomPosition m_LastAutoPosition;

    virtual void resizeEvent(QResizeEvent*);
    virtual void contentsMousePressEvent ( QMouseEvent * e );
    virtual void contentsMouseReleaseEvent ( QMouseEvent * e );
    virtual void contentsMouseMoveEvent ( QMouseEvent * e );

    bool _isMoving;
    QPoint _lastPos;

    bool _noUpdateZoomerPos;

private:
    void updateSizes(QSize s = QSize(0,0));
    void updateZoomerPos();
};

#endif
