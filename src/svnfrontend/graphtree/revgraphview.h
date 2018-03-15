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

#include <svnqt/client.h>
#include <svnqt/revision.h>

#include <QContextMenuEvent>
#include <QGraphicsView>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QProcess>

namespace svn
{
class LogEntry;
}

class QTemporaryFile;
class KProcess;
class RevisionTree;
class GraphTreeLabel;
class GraphMark;
class PannerView;
class QGraphicsScene;
class GraphTreeLabel;

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

    RevGraphView(const svn::ClientP &_client, QWidget *parent = nullptr);
    ~RevGraphView();

    void showText(const QString &s);
    void clear();

    void beginInsert();
    void endInsert();

    struct targetData {
        char Action;
        QString key;
        targetData(const QString &n, char _a)
            : Action(_a)
            , key(n)
        {
        }
    };
    typedef QList<targetData> tlist;

    struct keyData {
        QString name, Author, Date, Message;
        long rev;
        char Action;
        tlist targets;
    };

    typedef QMap<QString, keyData> trevTree;

    QString toolTip(const QString &nodename, bool full = false)const;

    void setBasePath(const QString &);
    void dumpRevtree();

signals:
    void dispDetails(const QString &);
    void makeCat(const svn::Revision &, const QString &, const QString &, const svn::Revision &, QWidget *);
    void makeNorecDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    void makeRecDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);

public slots:
    virtual void zoomRectMoved(qreal, qreal);
    virtual void zoomRectMoveFinished();
    virtual void slotClientException(const QString &what);

protected slots:
    virtual void readDotOutput();
    virtual void dotExit(int, QProcess::ExitStatus);

protected:
    QGraphicsScene *m_Scene;
    GraphMark *m_Marker;
    svn::ClientP m_Client;
    GraphTreeLabel *m_Selected;
    QTemporaryFile *m_dotTmpFile;
    QString m_dotOutput;
    KProcess *m_renderProcess;
    trevTree m_Tree;
    QColor getBgColor(const QString &nodeName)const;
    bool isStart(const QString &nodeName)const;
    char getAction(const QString &)const;
    QString getLabelstring(const QString &nodeName);

    QMap<QString, GraphTreeLabel *> m_NodeList;
    QMap<QString, QString> m_LabelMap;

    int m_xMargin, m_yMargin;
    PannerView *m_CompleteView;
    double m_cvZoom;
    ZoomPosition m_LastAutoPosition;

    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;

    GraphTreeLabel *firstLabelAt(const QPoint &pos)const;

    bool m_isMoving;
    QPoint m_lastPos;

    bool m_noUpdateZoomerPos;

    QString m_basePath;

private:
    void updateSizes(QSize s = QSize(0, 0));
    void updateZoomerPos();
    void setNewDirection(int dir);
    void makeDiffPrev(GraphTreeLabel *);
    void makeDiff(const QString &, const QString &);
    void makeSelected(GraphTreeLabel *);
    void makeCat(GraphTreeLabel *_l);
};

#endif
