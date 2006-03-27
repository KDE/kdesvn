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
            t->show();
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
        *stream << "  " << it1.key().latin1()<< " " <<
            "[label=\""<< it1.data().name.latin1() << "\", ";
        *stream << "fontname=\"serif\", ";
        *stream << "URL=\"" << it1.data().name.latin1() << "\", ";
        if (it1.data().Action=='D') {
            *stream << "color=red, style=filled, ";
            *stream << "shape=record];\n";
        } else if (it1.data().Action=='A') {
            *stream << "color=green, style=filled, ";
            *stream << "shape=record];\n";
        } else {
            *stream << "shape=box];\n";
        }
        for (unsigned j=0;j
        <it1.data().targets.count();++j) {
            *stream<<"  "<<it1.key().latin1()<< " "
                << "->"<<" "<<it1.data().targets[j].key
                << " [color=\"midnightblue\",fontsize=10,style=\"solid\", arrowtail=none];\n";
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
