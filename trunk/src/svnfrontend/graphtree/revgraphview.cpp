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
#include "revgraphview.h"
#include "graphtreelabel.h"
#include "pannerview.h"
#include "graphtree_defines.h"
#include "src/settings/kdesvnsettings.h"
#include "../stopdlg.h"
#include "src/svnqt/client.h"

#include <kapplication.h>
#include <kdebug.h>
#include <ktemporaryfile.h>
#include <ktempdir.h>
#include <kprocess.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kglobalsettings.h>

#include <QMatrix>
#include <QPainter>
#include <QRegExp>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QScrollBar>

#include <math.h>

#define LABEL_WIDTH 160
#define LABEL_HEIGHT 90

RevGraphView::RevGraphView(QObject*aListener,svn::Client*_client,QWidget * parent, const char * name)
: QGraphicsView(parent)
{
    setObjectName(name?name:"RevGraphView");
    m_Scene = 0L;
    m_Client = _client;
    m_Listener = aListener;
    dotTmpFile = 0;
    m_Selected = 0;
    renderProcess = 0;
    m_Marker = 0;
    m_CompleteView = new PannerView(this);
    m_CompleteView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_CompleteView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_CompleteView->raise();
    m_CompleteView->hide();
    connect(m_CompleteView, SIGNAL(zoomRectMoved(qreal,qreal)),
            this, SLOT(zoomRectMoved(qreal,qreal)));
    connect(m_CompleteView, SIGNAL(zoomRectMoveFinished()),
            this, SLOT(zoomRectMoveFinished()));
    m_LastAutoPosition = TopLeft;
    _isMoving = false;
    _noUpdateZoomerPos = false;
    m_LabelMap[""]="";
}

RevGraphView::~RevGraphView()
{
    setScene(0);
    delete m_Scene;
    dotTmpFile = 0;
    delete m_CompleteView;
    delete renderProcess;
}

void RevGraphView::showText(const QString&s)
{
    clear();
    m_Scene = new QGraphicsScene;
    m_Scene->addSimpleText(s);
    setScene(m_Scene);
    m_Scene->update();
    m_CompleteView->hide();
}

void RevGraphView::clear()
{
    if (m_Selected) {
        m_Selected->setSelected(false);
        m_Selected=0;
    }
    if (m_Marker) {
        m_Marker->hide();
        delete m_Marker;
        m_Marker=0;
    }
    setScene(0);
    m_CompleteView->setScene(0);
    delete m_Scene;
    m_Scene = 0;
}

void RevGraphView::beginInsert()
{
    viewport()->setUpdatesEnabled(false);
}

void RevGraphView::endInsert()
{
    if (m_Scene) {
/*
        _cvZoom = 0;
        updateSizes();
*/
        m_Scene->update();
    }
    viewport()->setUpdatesEnabled(true);
}

void RevGraphView::readDotOutput()
{
    if (!renderProcess) return;
    dotOutput+=QString::fromLocal8Bit(renderProcess->readAllStandardOutput());
}

void RevGraphView::dotExit(int exitcode,QProcess::ExitStatus exitStatus)
{
    if (!renderProcess)return;
    if (exitStatus!=QProcess::NormalExit || exitcode!=0) {
        QString error = i18n("Could not run process \"%1\".\n\nProcess stopped with message:\n%2",renderProcess->program().join(" "),
                              QString::fromLocal8Bit(renderProcess->readAllStandardError()));
        showText(error);
        delete renderProcess;
        renderProcess=0;
        return;
    }
    // remove line breaks when lines to long
    QRegExp endslash("\\\\\\n");
    dotOutput.remove(endslash);
    double scale = 1.0, scaleX = 1.0, scaleY = 1.0;
    double dotWidth, dotHeight;
    QTextStream* dotStream = new QTextStream(&dotOutput, QIODevice::ReadOnly);
    QString line,cmd;
    int lineno=0;
    beginInsert();
    clear();
    /* mostly taken from kcachegrind */
    scaleX = scale * 60; scaleY = scale * 70;
    QRect startRect;
    while (1) {
        line = dotStream->readLine();
        if (line.isNull()) break;
        lineno++;
        if (line.isEmpty()) continue;
        QTextStream lineStream(&line, QIODevice::ReadOnly);
        lineStream >> cmd;
        if (cmd == "stop") {break; }

        if (cmd == "graph") {
            lineStream >> scale >> dotWidth >> dotHeight;
            int w = (int)(scaleX * dotWidth);
            int h = (int)(scaleY * dotHeight);
            _xMargin = 50;
            if (w < QApplication::desktop()->width())
                _xMargin += (QApplication::desktop()->width()-w)/2;
            _yMargin = 50;
            if (h < QApplication::desktop()->height())
                _yMargin += (QApplication::desktop()->height()-h)/2;
            m_Scene = new QGraphicsScene(0.0,0.0,qreal(w+2*_xMargin), qreal(h+2*_yMargin));
            m_Scene->setBackgroundBrush(Qt::white);
            continue;
        }
        if ((cmd != "node") && (cmd != "edge")) {
            kWarning() << "Ignoring unknown command '" << cmd << "' from dot ("
                << dotTmpFile->fileName() << ":" << lineno << ")" << endl;
            continue;
        }
        if (cmd=="node") {
            QString nodeName, label;
            QString _x,_y,_w,_h;
            double x, y, width, height;
            lineStream >> nodeName >> _x >> _y >> _w >> _h;
            x=_x.toDouble();
            y=_y.toDouble();
            width=_w.toDouble();
            height=_h.toDouble();
            // better here 'cause dot may scramble utf8 labels so we regenerate it better
            // and do not read it in.
            label = getLabelstring(nodeName);
            int xx = (int)(scaleX * x + _xMargin);
            int yy = (int)(scaleY * (dotHeight - y) + _yMargin);
            int w = (int)(scaleX * width);
            int h = (int)(scaleY * height);
            QRect r(xx-w/2, yy-h/2, w, h);
            GraphTreeLabel*t=new GraphTreeLabel(label,nodeName,r);
            m_Scene->addItem(t);
            if (isStart(nodeName)) {
                startRect = r;
                ensureVisible(startRect);
            }
            t->setBgColor(getBgColor(nodeName));
            t->setZValue(1.0);
            t->show();
            m_NodeList[nodeName]=t;
            t->setToolTip(toolTip(nodeName));
        } else {
            QString node1Name, node2Name, label;
            QString _x,_y;
            double x, y;
            QPolygonF pa;
            int points, i;
            lineStream >> node1Name >> node2Name;
            lineStream >> points;
            pa.resize(points);
            for (i=0;i<points;++i) {
                if (lineStream.atEnd()) break;
                lineStream >> _x >> _y;
                x=_x.toDouble();
                y=_y.toDouble();
                double xx = (double)(scaleX * x + _xMargin);
                double yy = (double)(scaleY * (dotHeight - y) + _yMargin);
#if 0
                if (0) qDebug("   P %d: ( %f / %f ) => ( %d / %d)",
                    i, x, y, xx, yy);
#endif
                pa[i]=QPointF(xx, yy);
            }
            if (i < points) {
                qDebug("CallGraphView: Can't read %d spline points (%d)",
                    points,  lineno);
                continue;
            }

            GraphEdge * n = new GraphEdge();
            QColor arrowColor = Qt::black;
            m_Scene->addItem(n);
            n->setPen(QPen(arrowColor,1));
            n->setControlPoints(pa);
            n->setZValue(0.5);
            n->show();

            /* arrow dir
             * eg. it is vx and vy for computing, NO absolute points!
             */
            QPointF arrowDir;
            int indexHead = -1;

            QMap<QString,GraphTreeLabel*>::Iterator it;
            it = m_NodeList.find(node2Name);
            if (it!=m_NodeList.end()) {
                it.value()->setSource(node1Name);
            }
            it = m_NodeList.find(node1Name);
            if (it!=m_NodeList.end()) {
                GraphTreeLabel*tlab = it.value();
                if (tlab) {
                    QPointF toCenter = tlab->rect().center();
                    qreal dx0 = pa[0].x() - toCenter.x();
                    qreal dy0 = pa[0].y() - toCenter.y();
                    qreal dx1 = pa[points-1].x() - toCenter.x();
                    qreal dy1 = pa[points-1].y() - toCenter.y();

                    if (dx0*dx0+dy0*dy0 > dx1*dx1+dy1*dy1) {
                        // start of spline is nearer to call target node
                        indexHead=-1;
                        while(arrowDir.isNull() && (indexHead<points-2)) {
                            indexHead++;
                            arrowDir = pa[indexHead] - pa[indexHead+1];
                        }
                    }
                }
            }

            if (arrowDir.isNull()) {
                indexHead = points;
                // sometimes the last spline points from dot are the same...
                while(arrowDir.isNull() && (indexHead>1)) {
                    indexHead--;
                    arrowDir = pa[indexHead] - pa[indexHead-1];
                }
            }
            if (!arrowDir.isNull()) {
                QPointF baseDir = arrowDir;
                arrowDir *= 10.0/sqrt(double(arrowDir.x()*arrowDir.x() +arrowDir.y()*arrowDir.y()));
                baseDir/=(sqrt(baseDir.x()*baseDir.x()+baseDir.y()*baseDir.y()));

                QPointF t1(-baseDir.y()-baseDir.x(),baseDir.x()-baseDir.y());
                QPointF t2(baseDir.y()-baseDir.x(),-baseDir.x()-baseDir.y());
                QPolygonF a;
                t1*=3;
                t2*=3;
                a<<pa[indexHead]+t1<<pa[indexHead]+arrowDir<<pa[indexHead]+t2;

                GraphEdgeArrow* aItem = new GraphEdgeArrow(n,0);
                m_Scene->addItem(aItem);
                aItem->setPolygon(a);
                aItem->setBrush(arrowColor);
                aItem->setZValue(1.5);
                aItem->show();
            }
        }
    }
    if (!m_Scene) {
        QString s = i18n("Error running the graph layouting tool.\n");
        s += i18n("Please check that 'dot' is installed (package GraphViz).");
        showText(s);
    } else {
        setScene(m_Scene);
        m_CompleteView->setScene(m_Scene);
        if (startRect.isValid()) {
            ensureVisible(startRect);
        }
    }
    endInsert();
    delete renderProcess;
    renderProcess=0;
}

bool RevGraphView::isStart(const QString&nodeName)const
{
    bool res = false;
    trevTree::ConstIterator it;
    it = m_Tree.find(nodeName);
    if (it==m_Tree.end()) {
        return res;
    }
    switch (it.value().Action) {
        case 'A':
            res = true;
            break;
    }
    return res;
}

char RevGraphView::getAction(const QString&nodeName)const
{
    trevTree::ConstIterator it;
    it = m_Tree.find(nodeName);
    if (it==m_Tree.end()) {
        return (char)0;
    }
    return it.value().Action;
}

QColor RevGraphView::getBgColor(const QString&nodeName)const
{
    trevTree::ConstIterator it;
    it = m_Tree.find(nodeName);
    QColor res = Qt::white;
    if (it==m_Tree.end()) {
        return res;
    }
    switch (it.value().Action) {
        case 'D':
            res = Kdesvnsettings::tree_delete_color();
            break;
        case 'R':
        case 'M':
            res = Kdesvnsettings::tree_modify_color();
            break;
        case 'A':
            res = Kdesvnsettings::tree_add_color();
            break;
        case 'C':
        case 1:
            res = Kdesvnsettings::tree_copy_color();
            break;
        case 2:
            res = Kdesvnsettings::tree_rename_color();
            break;
        default:
            res = Kdesvnsettings::tree_modify_color();
            break;
    }
    return res;
}

const QString&RevGraphView::getLabelstring(const QString&nodeName)
{
    QMap<QString,QString>::ConstIterator nIt;
    nIt = m_LabelMap.find(nodeName);
    if (nIt!=m_LabelMap.end()) {
        return nIt.value();
    }
    trevTree::ConstIterator it1;
    it1 = m_Tree.find(nodeName);
    if (it1==m_Tree.end()) {
        return m_LabelMap[""];
    }
    QString res;
    QString revstring = svn::Revision(it1.value().rev).toString();
    switch (it1.value().Action) {
    case 'D':
        res = i18n("Deleted at revision %1",revstring);
    break;
    case 'A':
        res = i18n("Added at revision %1 as %2",
                    revstring,
                    it1.value().name);
    break;
    case 'C':
    case 1:
        res = i18n("Copied to %1 at revision %2",it1.value().name,revstring);
    break;
    case 2:
        res = i18n("Renamed to %1 at revision %2",it1.value().name,revstring);
    break;
    case 'M':
        res = i18n("Modified at revision %1",revstring);
    break;
    case 'R':
        res = i18n("Replaced at revision %1",revstring);
        break;
    default:
        res=i18n("Revision %1",revstring);
    break;
    }
    m_LabelMap[nodeName]=res;
    return m_LabelMap[nodeName];
}

void RevGraphView::dumpRevtree()
{
    if (dotTmpFile) {
        dotTmpFile->close();
    }
    clear();
    dotOutput = "";
    dotTmpFile = new KTemporaryFile;
    dotTmpFile->setSuffix(".dot");
    dotTmpFile->setAutoRemove(true);
    dotTmpFile->open();

    if (!dotTmpFile->open()) {
        showText(i18n("Could not open tempfile %1 for writing.",dotTmpFile->fileName()));
        return;
    }
    QTextStream stream(dotTmpFile);
    QFont f = KGlobalSettings::fixedFont();
    QFontMetrics _fm(KGlobalSettings::fixedFont());
    int _fontsize = _fm.height();
    if (_fontsize<0) {
        _fontsize=10;
    }

    stream << "digraph \"callgraph\" {\n";
    stream << "  bgcolor=\"transparent\";\n";
    int dir = Kdesvnsettings::tree_direction();
    stream << QString("  rankdir=\"");
    switch (dir) {
        case 3:
            stream << "TB";
        break;
        case 2:
            stream << "RL";
        break;
        case 1:
            stream << "BT";
        break;
        case 0:
        default:
            stream << "LR";
        break;
    }
    stream << "\";\n";

    //stream << QString("  overlap=false;\n  splines=true;\n");

    RevGraphView::trevTree::ConstIterator it1;
    for (it1=m_Tree.begin();it1!=m_Tree.end();++it1) {
        stream << "  " << it1.key()
            << "[ "
            << "shape=box, "
            << "label=\""<<"Zeile 1 geht ab Zeile 2 geht ab"/*getLabelstring(it1.key())*/<<"\","
	    <<"fontsize="<<_fontsize<<",fontname=\""<<f.family()<<"\","
            << "];\n";
        for (int j=0;j<it1.value().targets.count();++j) {
            stream<<"  "<<it1.key().toLatin1()<< " "
                << "->"<<" "<<it1.value().targets[j].key
                << " [fontsize="<<_fontsize<<",fontname=\""<<f.family()<<"\",style=\"solid\"];\n";
        }
    }
    stream << "}\n"<<flush;
    renderProcess = new KProcess();
    renderProcess->setEnv("LANG","C");
    *renderProcess << "dot";
    *renderProcess << dotTmpFile->fileName() << "-Tplain";
    connect(renderProcess,SIGNAL(finished(int,QProcess::ExitStatus)),
        this,SLOT(dotExit(int,QProcess::ExitStatus)));
    connect(renderProcess,SIGNAL(readyReadStandardOutput()),
        this,SLOT(readDotOutput()) );
    renderProcess->setOutputChannelMode(KProcess::SeparateChannels);
    renderProcess->start();
}

QString RevGraphView::toolTip(const QString&_nodename,bool full)const
{
    QString res;
    trevTree::ConstIterator it;
    it = m_Tree.find(_nodename);
    if (it==m_Tree.end()) {
        return res;
    }
    QStringList sp = it.value().Message.split('\n');
    QString sm;
    if (sp.count()==0) {
        sm = it.value().Message;
    } else {
        if (!full) {
            sm = sp[0]+"...";
        } else {
            for (int j = 0; j<sp.count(); ++j) {
                if (j>0) sm+="<br>";
                sm+=sp[j];
            }
        }
    }
    if (!full && sm.length()>50) {
        sm.truncate(47);
        sm+="...";
    }
    static QString csep = "</td><td>";
    static QString rend = "</td></tr>";
    static QString rstart = "<tr><td>";
    res = QString("<html><body>");

    if (!full) {
        res+=QString("<b>%1</b>").arg(it.value().name);
        res += i18n("<br>Revision: %1<br>Author: %2<br>Date: %3<br>Log: %4</html>",
                     it.value().rev,
                     it.value().Author,
                     it.value().Date,
                     sm);
    } else {
        res+="<table><tr><th colspan=\"2\"><b>"+it.value().name+"</b></th></tr>";
        res+=rstart;
        res+=i18n("<b>Revision</b>%1%2%3",csep,it.value().rev,rend);
        res+=rstart+i18n("<b>Author</b>%1%2%3",csep,it.value().Author,rend);
        res+=rstart+i18n("<b>Date</b>%1%2%3",csep,it.value().Date,rend);
        res+=rstart+i18n("<b>Log</b>%1%2%3",csep,sm,rend);
        res+="</table></body></html>";
    }
    return res;
}

void RevGraphView::updateSizes(QSize s)
{
    if (!m_Scene) return;
    if (s == QSize(0,0)) s = size();

    // the part of the canvas that should be visible
    qreal cWidth  = m_Scene->width()  - 2*_xMargin + 100;
    qreal cHeight = m_Scene->height() - 2*_yMargin + 100;

    // hide birds eye view if no overview needed
    if (((cWidth < s.width()) && cHeight < s.height())||m_NodeList.count()==0) {
      m_CompleteView->hide();
      return;
    }

    m_CompleteView->show();

    // first, assume use of 1/3 of width/height (possible larger)
    double zoom = .33 * s.width() / cWidth;
    if (zoom * cHeight < .33 * s.height()) zoom = .33 * s.height() / cHeight;

    // fit to widget size
    if (cWidth  * zoom  > s.width())   zoom = s.width() / (double)cWidth;
    if (cHeight * zoom  > s.height())  zoom = s.height() / (double)cHeight;

    // scale to never use full height/width
    zoom = zoom * 3/4;

    // at most a zoom of 1/3
    if (zoom > .33) zoom = .33;

    if (zoom != _cvZoom) {
      _cvZoom = zoom;
      QMatrix wm;
      m_CompleteView->setMatrix(wm.scale( zoom, zoom ));

      // make it a little bigger to compensate for widget frame
      m_CompleteView->resize(int(cWidth * zoom) + 4,
                            int(cHeight * zoom) + 4);

      // update ZoomRect in completeView
      scrollContentsBy(0, 0);
    }

    m_CompleteView->centerOn(m_Scene->width()/2, m_Scene->height()/2);
    updateZoomerPos();
}

void RevGraphView::updateZoomerPos()
{
    int cvW = m_CompleteView->width();
    int cvH = m_CompleteView->height();
    int x = width()- cvW - verticalScrollBar()->width()    -2;
    int y = height()-cvH - horizontalScrollBar()->height() -2;

    QPoint oldZoomPos = m_CompleteView->pos();
    QPoint newZoomPos = QPoint(0,0);

    ZoomPosition zp = m_LastAutoPosition;
    int tlCols = items(QRect(0,0, cvW,cvH)).count();
    int trCols = items(QRect(x,0, cvW,cvH)).count();
    int blCols = items(QRect(0,y, cvW,cvH)).count();
    int brCols = items(QRect(x,y, cvW,cvH)).count();
    int minCols = tlCols;

    zp = m_LastAutoPosition;
    switch(zp) {
        case TopRight:    minCols = trCols; break;
        case BottomLeft:  minCols = blCols; break;
        case BottomRight: minCols = brCols; break;
        default:
        case TopLeft:     minCols = tlCols; break;
    }
    if (minCols > tlCols) { minCols = tlCols; zp = TopLeft; }
    if (minCols > trCols) { minCols = trCols; zp = TopRight; }
    if (minCols > blCols) { minCols = blCols; zp = BottomLeft; }
    if (minCols > brCols) { minCols = brCols; zp = BottomRight; }

    m_LastAutoPosition = zp;
    switch(zp) {
    case TopRight:
        newZoomPos = QPoint(x,0);
        break;
    case BottomLeft:
        newZoomPos = QPoint(0,y);
        break;
    case BottomRight:
        newZoomPos = QPoint(x,y);
        break;
    default:
    break;
    }
    if (newZoomPos != oldZoomPos) m_CompleteView->move(newZoomPos);
}

void RevGraphView::zoomRectMoved(qreal dx,qreal dy)
{
  //if (leftMargin()>0) dx = 0;
  //if (topMargin()>0) dy = 0;
  _noUpdateZoomerPos = true;
  QScrollBar *hBar = horizontalScrollBar();
  QScrollBar *vBar = verticalScrollBar();
  hBar->setValue(hBar->value() + int(dx));
  vBar->setValue(vBar->value() + int(dy));
  _noUpdateZoomerPos = false;
}

void RevGraphView::zoomRectMoveFinished()
{
#if 0
    if (_zoomPosition == Auto)
#endif
    updateZoomerPos();
}

void RevGraphView::resizeEvent(QResizeEvent*e)
{
    QGraphicsView::resizeEvent(e);
    if (m_Scene) updateSizes(e->size());
}

void RevGraphView::makeSelected(GraphTreeLabel*gtl)
{
    if (m_Selected) {
        m_Selected->setSelected(false);
    }
    m_Selected=gtl;
    if (m_Marker) {
        m_Marker->hide();
        delete m_Marker;
        m_Marker=0;
    }
    if (gtl) {
        m_Marker = new GraphMark(gtl);
        m_Scene->addItem(m_Marker);
        m_Marker->setPos(gtl->pos());
        m_Marker->setZValue(-1);
    }
    m_Scene->update();
    m_CompleteView->update();

}

GraphTreeLabel*RevGraphView::firstLabelAt(const QPoint&pos)const
{
    QList<QGraphicsItem*> its=items(pos);
    for (QList<QGraphicsItem*>::size_type i=0;i<its.size();++i) {
        if (its[i]->type()==GRAPHTREE_LABEL) {
            return static_cast<GraphTreeLabel*>(its[i]);
        }
    }

    return 0;
}

void RevGraphView::mouseDoubleClickEvent ( QMouseEvent * e )
{
    setFocus();
    if (e->button() == Qt::LeftButton) {
        GraphTreeLabel* i = firstLabelAt(e->pos());
        if (i == 0) {
            return;
        }
        makeSelected((GraphTreeLabel*)i);
        emit dispDetails(toolTip(((GraphTreeLabel*)i)->nodename(),true));
    }
}

void RevGraphView::mousePressEvent ( QMouseEvent * e )
{
    setFocus();
    if (e->button() == Qt::LeftButton) {
        _isMoving = true;
        _lastPos = e->pos();
    }
}

void RevGraphView::mouseReleaseEvent ( QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && _isMoving) {
        QPointF topLeft = mapToScene(QPoint(0, 0));
        QPointF bottomRight = mapToScene(QPoint(width(), height()));
        QRectF z(topLeft, bottomRight);
        m_CompleteView->setZoomRect(z);
        _isMoving = false;
        updateZoomerPos();
    }
}

void RevGraphView::scrollContentsBy(int dx, int dy)
{
    // call QGraphicsView implementation
    QGraphicsView::scrollContentsBy(dx, dy);

    QPointF topLeft = mapToScene(QPoint(0, 0));
    QPointF bottomRight = mapToScene(QPoint(width(), height()));
    m_CompleteView->setZoomRect(QRectF(topLeft, bottomRight));
    if (!_noUpdateZoomerPos && !_isMoving) {
        updateZoomerPos();
    }
}

void RevGraphView::mouseMoveEvent ( QMouseEvent * e )
{
    if (_isMoving) {
        QPoint delta = e->pos() - _lastPos;
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        hBar->setValue(hBar->value() - delta.x());
        vBar->setValue(vBar->value() - delta.y());
        _lastPos = e->pos();
    }
}

void RevGraphView::setNewDirection(int dir)
{
    if (dir<0)dir=3;
    else if (dir>3)dir=0;
    Kdesvnsettings::setTree_direction(dir);
    dumpRevtree();
}

void RevGraphView::contextMenuEvent(QContextMenuEvent* e)
{
    if (!m_Scene) {
        return;
    }
    GraphTreeLabel* i = firstLabelAt(e->pos());

    QAction * ac;

    KMenu popup;
    if (i) {
        if (!((GraphTreeLabel*)i)->source().isEmpty() && getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.addAction(i18n("Diff to previous"))->setData(301);
        }
        if (m_Selected && m_Selected != i && getAction(m_Selected->nodename())!='D'
            && getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.addAction(i18n("Diff to selected item"))->setData(302);
        }
        if (getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.addAction(i18n("Cat this version"))->setData(303);
        }
        if (m_Selected == i) {
            popup.addAction(i18n("Unselect item"))->setData(401);
        } else {
            popup.addAction(i18n("Select item"))->setData(402);
        }
        popup.addSeparator();
        popup.addAction(i18n("Display details"))->setData(403);
        popup.addSeparator();
    }
    popup.addAction(i18n("Rotate counter-clockwise"))->setData(101);
    popup.addAction(i18n("Rotate clockwise"))->setData(102);
    popup.addSeparator();
    ac = popup.addAction(i18n("Diff in revisiontree is recursive"));
    ac->setData(202);
    ac->setCheckable(true);
    ac->setChecked(Kdesvnsettings::tree_diff_rec());
    popup.addAction(i18n("Save tree as png"))->setData(201);

    ac = popup.exec(e->globalPos());
    int r = 0;
    if (ac) {
        r = ac->data().toInt();
    }

    switch (r) {
        case 101:
        {
            int dir = Kdesvnsettings::tree_direction();
            setNewDirection(++dir);
        }
        break;
        case 102:
        {
            int dir = Kdesvnsettings::tree_direction();
            setNewDirection(--dir);
        }
        break;
        case 201:
        {
            QString fn = KFileDialog::getSaveFileName(KUrl(),
                                                   "image/png",
                                                   this
                                                   );
            if (!fn.isEmpty()) {
                if (m_Marker) {
                    m_Marker->hide();
                }
                if (m_Selected) {
                    m_Selected->setSelected(false);
                }
                QRect r = m_Scene->sceneRect().toRect();
                QPixmap pix(r.width(),r.height());
                pix.fill();
                QPainter p(&pix);
                m_Scene->render(&p);
                pix.save(fn,"PNG");
                if (m_Marker) {
                    m_Marker->show();
                }
                if (m_Selected) {
                    m_Selected->setSelected(true);
                    m_Scene->update();
                    m_CompleteView->updateCurrentRect();
                }
            }
        }
        case 202:
        {
            Kdesvnsettings::setTree_diff_rec(!Kdesvnsettings::tree_diff_rec());
            break;
        }
        break;
        case 301:
            if (i && i->type()==GRAPHTREE_LABEL && !((GraphTreeLabel*)i)->source().isEmpty()) {
                makeDiffPrev(i);
            }
        break;
        case 302:
            if (i && m_Selected) {
                makeDiff(i->nodename(),m_Selected->nodename());
            }
        break;
        case 303:
            if (i) {
                makeCat(i);
            }
        break;
        case 401:
            makeSelected(0);
        break;
        case 402:
            makeSelected(i);
        break;
        case 403:
            emit dispDetails(toolTip(i->nodename(),true));
        break;
        default:
        break;
    }
}

void RevGraphView::makeCat(GraphTreeLabel*_l)
{
    if (!_l) {
        return;
    }
    QString n1 = _l->nodename();
    trevTree::ConstIterator it = m_Tree.find(n1);
    if (it==m_Tree.end()) {
        return;
    }
    svn::Revision tr(it.value().rev);
    QString tp = _basePath+it.value().name;
    emit makeCat(tr,tp,it.value().name,tr,kapp->activeModalWidget());
}

void RevGraphView::makeDiffPrev(GraphTreeLabel*_l)
{
    if (!_l) return;
    QString n1,n2;
    n1 = _l->nodename();
    n2 = _l->source();
    makeDiff(n1,n2);
}

void RevGraphView::makeDiff(const QString&n1,const QString&n2)
{
    if (n1.isEmpty()||n2.isEmpty()) return;
    trevTree::ConstIterator it;
    it = m_Tree.find(n2);
    if (it==m_Tree.end()) {
        return;
    }
    svn::Revision sr(it.value().rev);
    QString sp = _basePath+it.value().name;

    it = m_Tree.find(n1);
    if (it==m_Tree.end()) {
        return;
    }
    svn::Revision tr(it.value().rev);
    QString tp = _basePath+it.value().name;
    if (Kdesvnsettings::tree_diff_rec()) {
        emit makeRecDiff(sp,sr,tp,tr,kapp->activeModalWidget());
    } else {
        emit makeNorecDiff(sp,sr,tp,tr,kapp->activeModalWidget());
    }
}

void RevGraphView::setBasePath(const QString&_path)
{
    _basePath = _path;
}

void RevGraphView::slotClientException(const QString&what)
{
    KMessageBox::sorry(KApplication::activeModalWidget(),what,i18n("SVN Error"));
}

#include "revgraphview.moc"
