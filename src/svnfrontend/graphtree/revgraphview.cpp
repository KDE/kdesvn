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
#include "revgraphview.h"
#include "graphtreelabel.h"
#include "pannerview.h"
#include "graphtree_defines.h"
#include "src/settings/kdesvnsettings.h"
#include "../stopdlg.h"
#include "svnqt/client.hpp"

#include <kapp.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <ktempdir.h>
#include <kprocess.h>
#include <klocale.h>
#include <kfiledialog.h>

#include <qtooltip.h>
#include <qwmatrix.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qregexp.h>

#include <math.h>

#define LABEL_WIDTH 160
#define LABEL_HEIGHT 90


class GraphViewTip:public QToolTip
{
public:
  GraphViewTip( QWidget* p ):QToolTip(p) {}
  virtual ~GraphViewTip(){}

protected:
    void maybeTip( const QPoint & );
};

void GraphViewTip::maybeTip( const QPoint & pos)
{
    if (!parentWidget()->inherits( "RevGraphView" )) return;
    RevGraphView* cgv = (RevGraphView*)parentWidget();
    QPoint cPos = cgv->viewportToContents(pos);
    QCanvasItemList l = cgv->canvas()->collisions(cPos);
    if (l.count() == 0) return;
    QCanvasItem* i = l.first();
    if (i->rtti() == GRAPHTREE_LABEL) {
        GraphTreeLabel*tl = (GraphTreeLabel*)i;
        QString nm = tl->nodename();
        QString tipStr = cgv->toolTip(nm);
        if (tipStr.length()>0) {
            QPoint vPosTL = cgv->contentsToViewport(i->boundingRect().topLeft());
            QPoint vPosBR = cgv->contentsToViewport(i->boundingRect().bottomRight());
            tip(QRect(vPosTL, vPosBR), tipStr);
        }
    }
}

RevGraphView::RevGraphView(QObject*aListener,svn::Client*_client,QWidget * parent, const char * name, WFlags f)
 : QCanvasView(parent,name,f)
{
    m_Canvas = 0L;
    m_Client = _client;
    m_Listener = aListener;
    dotTmpFile = 0;
    m_Selected = 0;
    renderProcess = 0;
    m_Marker = 0;
    m_Tip = new GraphViewTip(this);
    m_CompleteView = new PannerView(this);
    m_CompleteView->setVScrollBarMode(QScrollView::AlwaysOff);
    m_CompleteView->setHScrollBarMode(QScrollView::AlwaysOff);
    m_CompleteView->raise();
    m_CompleteView->hide();
    connect(this, SIGNAL(contentsMoving(int,int)),
            this, SLOT(contentsMovingSlot(int,int)));
    connect(m_CompleteView, SIGNAL(zoomRectMoved(int,int)),
            this, SLOT(zoomRectMoved(int,int)));
    connect(m_CompleteView, SIGNAL(zoomRectMoveFinished()),
            this, SLOT(zoomRectMoveFinished()));
    m_LastAutoPosition = TopLeft;
    _isMoving = false;
    _noUpdateZoomerPos = false;
    m_LabelMap[""]="";
}

RevGraphView::~RevGraphView()
{
    setCanvas(0);
    delete m_Canvas;
    delete dotTmpFile;
    delete m_CompleteView;
    delete m_Tip;
    delete renderProcess;
}

void RevGraphView::showText(const QString&s)
{
    clear();
    m_Canvas = new QCanvas(QApplication::desktop()->width(),
                        QApplication::desktop()->height());

    QCanvasText* t = new QCanvasText(s, m_Canvas);
    t->move(5, 5);
    t->show();
    center(0,0);
    setCanvas(m_Canvas);
    m_Canvas->update();
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
    if (!m_Canvas) return;
    delete m_Canvas;
    m_Canvas = 0;
    setCanvas(0);
    m_CompleteView->setCanvas(0);
}

void RevGraphView::beginInsert()
{
    viewport()->setUpdatesEnabled(false);
}

void RevGraphView::endInsert()
{
    if (m_Canvas) {
        _cvZoom = 0;
        updateSizes();
        m_Canvas->update();
    }
    viewport()->setUpdatesEnabled(true);
}

void RevGraphView::readDotOutput(KProcess*,char *   buffer,int   buflen)
{
    dotOutput+=QString::fromLocal8Bit(buffer, buflen);
}

void RevGraphView::dotExit(KProcess*p)
{
    if (p!=renderProcess)return;
    // remove line breaks when lines to long
    QRegExp endslash("\\\\\\n");
    dotOutput.replace(endslash,"");
    double scale = 1.0, scaleX = 1.0, scaleY = 1.0;
    double dotWidth, dotHeight;
    QTextStream* dotStream;
    dotStream = new QTextStream(dotOutput, IO_ReadOnly);
    QString line,cmd;
    int lineno=0;
    clear();
    beginInsert();
    /* mostly taken from kcachegrind */
    while (1) {
        line = dotStream->readLine();
        if (line.isNull()) break;
        lineno++;
        if (line.isEmpty()) continue;
        QTextStream lineStream(line, IO_ReadOnly);
        lineStream >> cmd;
        if (cmd == "stop") {break; }

        if (cmd == "graph") {
            lineStream >> scale >> dotWidth >> dotHeight;
            scaleX = scale * 60; scaleY = scale * 100;
            int w = (int)(scaleX * dotWidth);
            int h = (int)(scaleY * dotHeight);

            _xMargin = 50;
            if (w < QApplication::desktop()->width())
                _xMargin += (QApplication::desktop()->width()-w)/2;
            _yMargin = 50;
            if (h < QApplication::desktop()->height())
                _yMargin += (QApplication::desktop()->height()-h)/2;
            m_Canvas = new QCanvas(int(w+2*_xMargin), int(h+2*_yMargin));
            continue;
        }
        if ((cmd != "node") && (cmd != "edge")) {
            kdWarning() << "Ignoring unknown command '" << cmd << "' from dot ("
                << dotTmpFile->name() << ":" << lineno << ")" << endl;
            continue;
        }
        if (cmd=="node") {
            QString nodeName, label;
            double x, y, width, height;
            lineStream >> nodeName >> x >> y >> width >> height;
            // better here 'cause dot may scramble utf8 labels so we regenerate it better
            // and do not read it in.
            label = getLabelstring(nodeName);
            int xx = (int)(scaleX * x + _xMargin);
            int yy = (int)(scaleY * (dotHeight - y) + _yMargin);
            int w = (int)(scaleX * width);
            int h = (int)(scaleY * height);
            QRect r(xx-w/2, yy-h/2, w, h);
            GraphTreeLabel*t=new GraphTreeLabel(label,nodeName,r,m_Canvas);
            if (isStart(nodeName)) {
                ensureVisible(r.x(),r.y());
            }
            t->setBgColor(getBgColor(nodeName));
            t->setZ(1.0);
            t->show();
            m_NodeList[nodeName]=t;
        } else {
            QString node1Name, node2Name, label;
            double x, y;
            QPointArray pa;
            int points, i;
            lineStream >> node1Name >> node2Name;
            lineStream >> points;
            pa.resize(points);
            for (i=0;i<points;++i) {
                if (lineStream.atEnd()) break;
                lineStream >> x >> y;
                int xx = (int)(scaleX * x + _xMargin);
                int yy = (int)(scaleY * (dotHeight - y) + _yMargin);

                if (0) qDebug("   P %d: ( %f / %f ) => ( %d / %d)",
                    i, x, y, xx, yy);
                pa.setPoint(i, xx, yy);
            }
            if (i < points) {
                qDebug("CallGraphView: Can't read %d spline points (%d)",
                    points,  lineno);
                continue;
            }

            GraphEdge * n = new GraphEdge(m_Canvas);
            QColor arrowColor = Qt::black;
            n->setPen(QPen(arrowColor,1));
            n->setControlPoints(pa,false);
            n->setZ(0.5);
            n->show();

            /* arrow */
            QPoint arrowDir;
            int indexHead = -1;

            QMap<QString,GraphTreeLabel*>::Iterator it;
            it = m_NodeList.find(node2Name);
            if (it!=m_NodeList.end()) {
                it.data()->setSource(node1Name);
            }
            it = m_NodeList.find(node1Name);
            if (it!=m_NodeList.end()) {
                GraphTreeLabel*tlab = it.data();
                if (tlab) {
                    QPoint toCenter = tlab->rect().center();
                    int dx0 = pa.point(0).x() - toCenter.x();
                    int dy0 = pa.point(0).y() - toCenter.y();
                    int dx1 = pa.point(points-1).x() - toCenter.x();
                    int dy1 = pa.point(points-1).y() - toCenter.y();
                    if (dx0*dx0+dy0*dy0 > dx1*dx1+dy1*dy1) {
                    // start of spline is nearer to call target node
                        indexHead=-1;
                        while(arrowDir.isNull() && (indexHead<points-2)) {
                            indexHead++;
                            arrowDir = pa.point(indexHead) - pa.point(indexHead+1);
                        }
                    }
                }
            }

            if (arrowDir.isNull()) {
                indexHead = points;
                // sometimes the last spline points from dot are the same...
                while(arrowDir.isNull() && (indexHead>1)) {
                    indexHead--;
                    arrowDir = pa.point(indexHead) - pa.point(indexHead-1);
                }
            }
            if (!arrowDir.isNull()) {
                arrowDir *= 10.0/sqrt(double(arrowDir.x()*arrowDir.x() +
                    arrowDir.y()*arrowDir.y()));
                QPointArray a(3);
                a.setPoint(0, pa.point(indexHead) + arrowDir);
                a.setPoint(1, pa.point(indexHead) + QPoint(arrowDir.y()/2,
                    -arrowDir.x()/2));
                a.setPoint(2, pa.point(indexHead) + QPoint(-arrowDir.y()/2,
                    arrowDir.x()/2));
                GraphEdgeArrow* aItem = new GraphEdgeArrow(n,m_Canvas);
                aItem->setPoints(a);
                aItem->setBrush(arrowColor);
                aItem->setZ(1.5);
                aItem->show();
//                sItem->setArrow(aItem);
            }
        }
    }
    if (!m_Canvas) {
        QString s = i18n("Error running the graph layouting tool.\n");
        s += i18n("Please check that 'dot' is installed (package GraphViz).");
        showText(s);
    } else {
        setCanvas(m_Canvas);
        m_CompleteView->setCanvas(m_Canvas);
    }
    endInsert();
    delete p;
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
    switch (it.data().Action) {
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
    return it.data().Action;
}

QColor RevGraphView::getBgColor(const QString&nodeName)const
{
    trevTree::ConstIterator it;
    it = m_Tree.find(nodeName);
    QColor res = Qt::white;
    if (it==m_Tree.end()) {
        return res;
    }
    switch (it.data().Action) {
        case 'D':
            res = Kdesvnsettings::tree_delete_color();
            break;
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
        case 'R':
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
        return nIt.data();
    }
    trevTree::ConstIterator it1;
    it1 = m_Tree.find(nodeName);
    if (it1==m_Tree.end()) {
        return m_LabelMap[""];
    }
    QString res;
    switch (it1.data().Action) {
    case 'D':
        res = i18n("Deleted at revision %1").arg(it1.data().rev);
    break;
    case 'A':
        res = i18n("Added at revision %1 as %2")
            .arg(it1.data().rev)
            .arg(it1.data().name);
    break;
    case 'C':
    case 1:
        res = i18n("Copied to %1 at revision %2").arg(it1.data().name).arg(it1.data().rev);
    break;
    case 2:
        res = i18n("Renamed to %1 at revision %2").arg(it1.data().name).arg(it1.data().rev);
    break;
    case 'M':
        res = i18n("Modified at revision %1").arg(it1.data().rev);
    break;
    default:
        res=i18n("Revision %1").arg(it1.data().rev);
    break;
    }
    m_LabelMap[nodeName]=res;
    return m_LabelMap[nodeName];
}

void RevGraphView::dumpRevtree()
{
    delete dotTmpFile;
    clear();
    dotOutput = "";
    dotTmpFile = new KTempFile(QString::null,".dot");
    dotTmpFile->setAutoDelete(true);

    QTextStream* stream = dotTmpFile->textStream();
    if (!stream) {
        showText(i18n("Could not open tempfile %1 for writing.").arg(dotTmpFile->name()));
        return;
    }

    *stream << "digraph \"callgraph\" {\n";
    *stream << "  bgcolor=\"transparent\";\n";
    int dir = Kdesvnsettings::tree_direction();
    *stream << QString("  rankdir=\"");
    switch (dir) {
        case 3:
            *stream << "TB";
        break;
        case 2:
            *stream << "RL";
        break;
        case 1:
            *stream << "BT";
        break;
        case 0:
        default:
            *stream << "LR";
        break;
    }
    *stream << "\";\n";

    //*stream << QString("  overlap=false;\n  splines=true;\n");

    RevGraphView::trevTree::ConstIterator it1;
    for (it1=m_Tree.begin();it1!=m_Tree.end();++it1) {
        *stream << "  " << it1.key()
            << "[ "
            << "shape=box, "
            << "label=\""<<getLabelstring(it1.key())<<"\","
            << "];\n";
        for (unsigned j=0;j<it1.data().targets.count();++j) {
            *stream<<"  "<<it1.key().latin1()<< " "
                << "->"<<" "<<it1.data().targets[j].key
                << " [fontsize=10,style=\"solid\"];\n";
        }
    }
    *stream << "}\n"<<flush;
    renderProcess = new KProcess();
    renderProcess->setEnvironment("LANG","C");
    *renderProcess << "dot";
    *renderProcess << dotTmpFile->name() << "-Tplain";
    connect(renderProcess,SIGNAL(processExited(KProcess*)),this,SLOT(dotExit(KProcess*)));
    connect(renderProcess,SIGNAL(receivedStdout(KProcess*,char*,int)),
        this,SLOT(readDotOutput(KProcess*,char*,int)) );
    if (!renderProcess->start(KProcess::NotifyOnExit,KProcess::Stdout)) {
        QString error = i18n("Could not start process \"");
        for (unsigned c=0;c<renderProcess->args().count();++c) {
            error+=QString(" %1").arg(renderProcess->args()[c]);
        }
        error+=" \".";
        showText(error);
        renderProcess=0;
        //delete renderProcess;<
    }
}

QString RevGraphView::toolTip(const QString&_nodename,bool full)const
{
    QString res = QString::null;
    trevTree::ConstIterator it;
    it = m_Tree.find(_nodename);
    if (it==m_Tree.end()) {
        return res;
    }
    QStringList sp = QStringList::split("\n",it.data().Message);
    QString sm;
    if (sp.count()==0) {
        sm = it.data().Message;
    } else {
        if (!full) {
            sm = sp[0]+"...";
        } else {
            for (unsigned j = 0; j<sp.count(); ++j) {
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
        res+=QString("<b>%1</b>").arg(it.data().name);
        res += i18n("<br>Revision: %2<br>Author: %3<br>Date: %4<br>Log:%5</html>")
            .arg(it.data().rev)
            .arg(it.data().Author)
            .arg(it.data().Date)
            .arg(sm);
    } else {
        res+="<table><tr><th colspan=\"2\"><b>"+it.data().name+"</b></th></tr>";
        res+=rstart;
        res+=i18n("<b>Revision</b>%1%2%3").arg(csep).arg(it.data().rev).arg(rend);
        res+=rstart+i18n("<b>Author</b>%1%2%3").arg(csep).arg(it.data().Author).arg(rend);
        res+=rstart+i18n("<b>Date</b>%1%2%3").arg(csep).arg(it.data().Date).arg(rend);
        res+=rstart+i18n("<b>Log</b>%1%2%3").arg(csep).arg(sm).arg(rend);
        res+="</table></body></html>";
    }
    return res;
}

void RevGraphView::updateSizes(QSize s)
{
    if (!m_Canvas) return;
    if (s == QSize(0,0)) s = size();

    // the part of the canvas that should be visible
    int cWidth  = m_Canvas->width()  - 2*_xMargin + 100;
    int cHeight = m_Canvas->height() - 2*_yMargin + 100;

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
      if (0) qDebug("Canvas Size: %dx%d, Visible: %dx%d, Zoom: %f",
            m_Canvas->width(), m_Canvas->height(),
            cWidth, cHeight, zoom);

      QWMatrix wm;
      wm.scale( zoom, zoom );
      m_CompleteView->setWorldMatrix(wm);

      // make it a little bigger to compensate for widget frame
      m_CompleteView->resize(int(cWidth * zoom) + 4,
                            int(cHeight * zoom) + 4);

      // update ZoomRect in completeView
      contentsMovingSlot(contentsX(), contentsY());
    }

    m_CompleteView->setContentsPos(int(zoom*(_xMargin-50)),
                  int(zoom*(_yMargin-50)));
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

#if 0
    ZoomPosition zp = _zoomPosition;
    if (zp == Auto) {
#else
    ZoomPosition zp = m_LastAutoPosition;
#endif
    QPoint tl1Pos = viewportToContents(QPoint(0,0));
    QPoint tl2Pos = viewportToContents(QPoint(cvW,cvH));
    QPoint tr1Pos = viewportToContents(QPoint(x,0));
    QPoint tr2Pos = viewportToContents(QPoint(x+cvW,cvH));
    QPoint bl1Pos = viewportToContents(QPoint(0,y));
    QPoint bl2Pos = viewportToContents(QPoint(cvW,y+cvH));
    QPoint br1Pos = viewportToContents(QPoint(x,y));
    QPoint br2Pos = viewportToContents(QPoint(x+cvW,y+cvH));
    int tlCols = m_Canvas->collisions(QRect(tl1Pos,tl2Pos)).count();
    int trCols = m_Canvas->collisions(QRect(tr1Pos,tr2Pos)).count();
    int blCols = m_Canvas->collisions(QRect(bl1Pos,bl2Pos)).count();
    int brCols = m_Canvas->collisions(QRect(br1Pos,br2Pos)).count();
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
#if 0
    }
#endif
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

void RevGraphView::contentsMovingSlot(int x,int y)
{
    QRect z(int(x * _cvZoom), int(y * _cvZoom),
        int(visibleWidth() * _cvZoom)-1, int(visibleHeight() * _cvZoom)-1);
    if (0) qDebug("moving: (%d,%d) => (%d/%d - %dx%d)",
                x, y, z.x(), z.y(), z.width(), z.height());
    m_CompleteView->setZoomRect(z);
    if (!_noUpdateZoomerPos) {
        updateZoomerPos();
    }
}

void RevGraphView::zoomRectMoved(int dx,int dy)
{
  if (leftMargin()>0) dx = 0;
  if (topMargin()>0) dy = 0;
  _noUpdateZoomerPos = true;
  scrollBy(int(dx/_cvZoom),int(dy/_cvZoom));
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
    QCanvasView::resizeEvent(e);
    if (m_Canvas) updateSizes(e->size());
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
        m_Marker = new GraphMark(gtl,m_Canvas);
        m_Marker->setZ(-1);
        m_Marker->show();
        m_Selected->setSelected(true);
    }
    m_Canvas->update();
    m_CompleteView->updateCurrentRect();
}

void RevGraphView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
    setFocus();
    if (e->button() == Qt::LeftButton) {
        QCanvasItemList l = canvas()->collisions(e->pos());
        if (l.count()>0) {
            QCanvasItem* i = l.first();
            if (i->rtti()==GRAPHTREE_LABEL) {
                makeSelected( (GraphTreeLabel*)i);
                emit dispDetails(toolTip(((GraphTreeLabel*)i)->nodename(),true));
            }
        }
    }
}

void RevGraphView::contentsMousePressEvent ( QMouseEvent * e )
{
    setFocus();
    _isMoving = true;
    _lastPos = e->globalPos();
}

void RevGraphView::contentsMouseReleaseEvent ( QMouseEvent * )
{
    _isMoving = false;
    updateZoomerPos();
}

void RevGraphView::contentsMouseMoveEvent ( QMouseEvent * e )
{
    if (_isMoving) {
        int dx = e->globalPos().x() - _lastPos.x();
        int dy = e->globalPos().y() - _lastPos.y();
        _noUpdateZoomerPos = true;
        scrollBy(-dx, -dy);
        _noUpdateZoomerPos = false;
        _lastPos = e->globalPos();
    }
}

void RevGraphView::setNewDirection(int dir)
{
    if (dir<0)dir=3;
    else if (dir>3)dir=0;
    Kdesvnsettings::setTree_direction(dir);
    dumpRevtree();
}

void RevGraphView::contentsContextMenuEvent(QContextMenuEvent* e)
{
    if (!m_Canvas) return;
    QCanvasItemList l = canvas()->collisions(e->pos());
    QCanvasItem* i = (l.count() == 0) ? 0 : l.first();

    QPopupMenu popup;
    if (i && i->rtti()==GRAPHTREE_LABEL) {
        if (!((GraphTreeLabel*)i)->source().isEmpty() && getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.insertItem(i18n("Diff to previous"),301);
        }
        if (m_Selected && m_Selected != i && getAction(m_Selected->nodename())!='D'
            && getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.insertItem(i18n("Diff to selected item"),302);
        }
        if (getAction(((GraphTreeLabel*)i)->nodename())!='D') {
            popup.insertItem(i18n("Cat this version"),303);
        }
        if (m_Selected == i) {
            popup.insertItem(i18n("Unselect item"),401);
        } else {
            popup.insertItem(i18n("Select item"),402);
        }
        popup.insertSeparator();
        popup.insertItem(i18n("Display details"),403);
        popup.insertSeparator();
    }
    popup.insertItem(i18n("Rotate counter-clockwise"),101);
    popup.insertItem(i18n("Rotate clockwise"),102);
    popup.insertSeparator();
    popup.insertItem(i18n("Save tree as png"),201);

    int r = popup.exec(e->globalPos());

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
            QString fn = KFileDialog::getSaveFileName(":","*.png");
            if (!fn.isEmpty()) {
                if (m_Marker) {
                    m_Marker->hide();
                }
                if (m_Selected) {
                    m_Selected->setSelected(false);
                }
                QPixmap pix(m_Canvas->size());
                QPainter p(&pix);
                m_Canvas->drawArea( m_Canvas->rect(), &p );
                pix.save(fn,"PNG");
                if (m_Marker) {
                    m_Marker->show();
                }
                if (m_Selected) {
                    m_Selected->setSelected(true);
                    m_Canvas->update();
                    m_CompleteView->updateCurrentRect();
                }
            }
        }
        break;
        case 301:
            if (i && i->rtti()==GRAPHTREE_LABEL && !((GraphTreeLabel*)i)->source().isEmpty()) {
                makeDiffPrev((GraphTreeLabel*)i);
            }
        break;
        case 302:
            if (i && i->rtti()==GRAPHTREE_LABEL && m_Selected) {
                makeDiff(((GraphTreeLabel*)i)->nodename(),m_Selected->nodename());
            }
        break;
        case 303:
            if (i && i->rtti()==GRAPHTREE_LABEL) {
                makeCat((GraphTreeLabel*)i);
            }
        break;
        case 401:
            makeSelected(0);
        break;
        case 402:
            makeSelected((GraphTreeLabel*)i);
        break;
        case 403:
            emit dispDetails(toolTip(((GraphTreeLabel*)i)->nodename(),true));
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
    svn::Revision tr(it.data().rev);
    QString tp = _basePath+it.data().name;
    emit makeCat(tr,tp,it.data().name,tr,kapp->activeModalWidget());
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
    svn::Revision sr(it.data().rev);
    QString sp = _basePath+it.data().name;

    it = m_Tree.find(n1);
    if (it==m_Tree.end()) {
        return;
    }
    svn::Revision tr(it.data().rev);
    QString tp = _basePath+it.data().name;
    QByteArray ex;
    KTempDir tdir;
    tdir.setAutoDelete(true);
    kdDebug()<<"Non recourse diff"<<endl;
    QString tn = QString("%1/%2").arg(tdir.name()).arg("/svndiff");
    bool ignore_content = Kdesvnsettings::diff_ignore_content();
    try {
        StopDlg sdlg(m_Listener,kapp->activeModalWidget(),0,"Diffing","Diffing - hit cancel for abort");
        ex = m_Client->diff(svn::Path(tn),
            svn::Path(sp),svn::Path(tp),
            sr, tr,
            false,false,false,ignore_content);
    } catch (svn::ClientException e) {
//        emit clientException(e.msg());
        return;
    }
    emit dispDiff(QString::fromUtf8(ex));
}

void RevGraphView::setBasePath(const QString&_path)
{
    _basePath = _path;
}

#include "revgraphview.moc"
