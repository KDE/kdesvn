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
#include <kapp.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <math.h>

#define LABEL_WIDTH 160
#define LABEL_HEIGHT 90

RevGraphView::RevGraphView(QWidget * parent, const char * name, WFlags f)
 : QCanvasView(parent,name,f)
{
    m_Canvas = 0L;
    dotTmpFile = 0;
    renderProcess = 0;
}

RevGraphView::~RevGraphView()
{
    setCanvas(0);
    delete m_Canvas;
    delete dotTmpFile;
}

void RevGraphView::addLabel(int row,int column, const QString&path,const QString&action,const svn::LogEntry&e)
{
    if (!m_Canvas) return;
    /* dummy for test*/
    int offset = (LABEL_HEIGHT+10)*row+30;
    int x_pos = 10+column*(LABEL_WIDTH+20);
    if (offset+LABEL_HEIGHT>m_Canvas->height()) {
        m_Canvas->resize(m_Canvas->width(),offset+LABEL_HEIGHT);
    }
    QRect r(x_pos,offset,LABEL_WIDTH,LABEL_HEIGHT);

    GraphTreeLabel*t=new GraphTreeLabel(path,action,e,r,m_Canvas);
    t->show();
    kdDebug()<<"addLabel "<<path << endl;
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
}

void RevGraphView::clear()
{
    if (!m_Canvas) return;
    delete m_Canvas;
    m_Canvas = 0;
    setCanvas(0);
}

void RevGraphView::beginInsert()
{
    viewport()->setUpdatesEnabled(false);
}

void RevGraphView::endInsert()
{
    if (m_Canvas) m_Canvas->update();
    viewport()->setUpdatesEnabled(true);
}

void RevGraphView::readDotOutput(KProcess*,char *   buffer,int   buflen)
{
    dotOutput+=QString::fromLatin1(buffer, buflen);
}

void RevGraphView::dotExit(KProcess*p)
{
    if (p!=renderProcess)return;
    double scale = 1.0, scaleX = 1.0, scaleY = 1.0;
    double dotWidth, dotHeight;
    QTextStream* dotStream;
    dotStream = new QTextStream(dotOutput, IO_ReadOnly);
    QString line,cmd;
    int lineno=0;
    int splinenr = 0;
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
//        kdDebug()<<"Command = "<<cmd << " at line "<<lineno<<endl;
        if (cmd == "stop") break;

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
            setCanvas(m_Canvas);
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
            QChar c;
            lineStream >> c;
            while (c.isSpace()) lineStream >> c;
            if (c != '\"') {
              lineStream >> label;
              label = c + label;
            } else {
                lineStream >> c;
                while(!c.isNull() && (c != '\"')) {
                //if (c == '\\') lineStream >> c;
                    label += c;
                    lineStream >> c;
                }
            }

            int xx = (int)(scaleX * x + _xMargin);
            int yy = (int)(scaleY * (dotHeight - y) + _yMargin);
            int w = (int)(scaleX * width);
            int h = (int)(scaleY * height);
            QRect r(xx-w/2, yy-h/2, w, h);
            GraphTreeLabel*t=new GraphTreeLabel(label,r,m_Canvas);
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
            for (i=0;i<points;i++) {
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
    endInsert();
    delete p;
    renderProcess=0;
}

bool RevGraphView::isStart(const QString&nodeName)
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

QColor RevGraphView::getBgColor(const QString&nodeName)
{
    trevTree::ConstIterator it;
    it = m_Tree.find(nodeName);
    QColor res = Qt::white;
    if (it==m_Tree.end()) {
        return res;
    }
    switch (it.data().Action) {
        case 'D':
            res = Qt::red;
            break;
        case 'M':
            res = Qt::gray;
            break;
        case 'A':
            res = Qt::green;
            break;
        case 'C':
        case 'R':
        case 1:
        case 2:
            res = Qt::lightGray;
        default:
            break;
    }
    return res;
}

void RevGraphView::dumpRevtree()
{
    delete dotTmpFile;
    dotOutput = "";
    dotTmpFile = new KTempFile(QString::null,".dot");
    dotTmpFile->setAutoDelete(true);

    QTextStream* stream = dotTmpFile->textStream();
    if (!stream) {
        kdDebug()<<"Could not open tempfile for writing"<<endl;
        return;
    }

    *stream << "digraph \"callgraph\" {\n";
    *stream << "  bgcolor=\"transparent\";\n";
    *stream << QString("  rankdir=BT;\n");
    //*stream << QString("  overlap=false;\n  splines=true;\n");

    RevGraphView::trevTree::ConstIterator it1;
    for (it1=m_Tree.begin();it1!=m_Tree.end();++it1) {
        *stream << "  " << it1.key().latin1()
            << "[ ";
            //"[label=\""<< it1.data().name.latin1() << "\", ";
//        *stream << "fontname=\"serif\", ";
        *stream << "shape=box, ";
        if (it1.data().Action=='D') {
            *stream << "label=\"Deleted at Revision "<<it1.data().rev
                << "\",";
        } else if (it1.data().Action=='A') {
            *stream << "label=\"Added at Revision "<<it1.data().rev
                    << " " << it1.data().name.latin1() << " "
                    <<"\",";
        } else if (it1.data().Action=='C'||it1.data().Action==(char)1){
            *stream << "label=\"Copy to "<< it1.data().name.latin1()
                    << " at Rev " << it1.data().rev
                    <<"\",";
        } else if (it1.data().Action=='R'||it1.data().Action==(char)2){
            *stream << "label=\"Renamed to "<< it1.data().name.latin1()
                    << " at Rev " << it1.data().rev
                    <<"\",";
        } else {
            *stream << "label=\"Modified at Revision "<<it1.data().rev<<"\",";
        }
        *stream << "];\n";
        for (unsigned j=0;j
        <it1.data().targets.count();++j) {
            *stream<<"  "<<it1.key().latin1()<< " "
                << "->"<<" "<<it1.data().targets[j].key
                << " [fontsize=10,style=\"solid\"];\n";
        }
    }
    *stream << "}\n"<<flush;
    renderProcess = new KProcess();
    *renderProcess << "dot";
    *renderProcess << dotTmpFile->name() << "-Tplain";
    connect(renderProcess,SIGNAL(processExited(KProcess*)),this,SLOT(dotExit(KProcess*)));
    connect(renderProcess,SIGNAL(receivedStdout(KProcess*,char*,int)),
        this,SLOT(readDotOutput(KProcess*,char*,int)) );
    if (!renderProcess->start(KProcess::NotifyOnExit,KProcess::Stdout)) {
        kdDebug()<<"Starting process " << renderProcess->args()<<endl;
        delete renderProcess;
    }
}

#include "revgraphview.moc"
