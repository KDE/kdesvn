/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
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

#include <svnqt/revision.h>
#include <svnqt/shared_pointer.h>

#include <QContextMenuEvent>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QProcess>

namespace svn {
    class LogEntry;
    class Client;
}

class KTemporaryFile;
class KProcess;
class RevisionTree;
class GraphTreeLabel;
class GraphMark;
class PannerView;
class QGraphicsScene;
class CContextListener;
class GraphTreeLabel;

typedef svn::SharedPointer<KTemporaryFile> TempFilePtr;

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class RevGraphView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ZoomPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };
    /* avoid large copy operations */
    friend class RevisionTree;

    RevGraphView(QObject*,svn::Client*,QWidget * parent = 0, const char * name = 0);
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
    typedef QList<targetData> tlist;

    struct keyData {
        QString name,Author,Date,Message;
        long rev;
        char Action;
        tlist targets;
    };

    typedef QMap<QString,keyData> trevTree;

    QString toolTip(const QString&nodename,bool full=false)const;

    void setBasePath(const QString&);
    void dumpRevtree();

signals:
    void dispDetails(const QString&);
    void makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*);
    void makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);
    void makeRecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);

public slots:
    virtual void zoomRectMoved(qreal,qreal);
    virtual void zoomRectMoveFinished();
    virtual void slotClientException(const QString&what);

protected slots:
    virtual void readDotOutput();
    virtual void dotExit(int,QProcess::ExitStatus);

protected:
    QGraphicsScene*m_Scene;
    GraphMark*m_Marker;
    svn::Client*m_Client;
    GraphTreeLabel*m_Selected;
    QObject*m_Listener;
    TempFilePtr dotTmpFile;
    QString dotOutput;
    KProcess*renderProcess;
    trevTree m_Tree;
    QColor getBgColor(const QString&nodeName)const;
    bool isStart(const QString&nodeName)const;
    char getAction(const QString&)const;
    const QString&getLabelstring(const QString&nodeName);

    QMap<QString,GraphTreeLabel*> m_NodeList;
    QMap<QString,QString> m_LabelMap;

    int _xMargin,_yMargin;
    PannerView*m_CompleteView;
    double _cvZoom;
    ZoomPosition m_LastAutoPosition;

    virtual void resizeEvent(QResizeEvent*);
    virtual void mousePressEvent ( QMouseEvent * e );
    virtual void mouseReleaseEvent ( QMouseEvent * e );
    virtual void mouseMoveEvent ( QMouseEvent*e);
    virtual void contextMenuEvent(QContextMenuEvent*e);
    virtual void mouseDoubleClickEvent ( QMouseEvent * e );
    virtual void scrollContentsBy(int dx, int dy);

    GraphTreeLabel*firstLabelAt(const QPoint&pos)const;

    bool _isMoving;
    QPoint _lastPos;

    bool _noUpdateZoomerPos;

    QString _basePath;

private:
    void updateSizes(QSize s = QSize(0,0));
    void updateZoomerPos();
    void setNewDirection(int dir);
    void makeDiffPrev(GraphTreeLabel*);
    void makeDiff(const QString&,const QString&);
    void makeSelected(GraphTreeLabel*);
    void makeCat(GraphTreeLabel*_l);
};

#endif
