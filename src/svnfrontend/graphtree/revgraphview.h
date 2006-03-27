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

/**
	@author Rajko Albrecht <ral@alwins-world.de>
*/
class RevGraphView : public QCanvasView
{
    Q_OBJECT
public:
    friend class RevisionTree;

    RevGraphView(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
    virtual ~RevGraphView();

    void showText(const QString&s);
    void clear();
    void addLabel(int row,int column, const QString&,const QString&,const svn::LogEntry&);

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
        QString name;
        long rev;
        char Action;
        tlist targets;
    };

    typedef QMap<QString,keyData> trevTree;

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

    int _xMargin,_yMargin;
};

#endif
