/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#ifndef REVTREEWIDGET_H
#define REVTREEWIDGET_H

#include <svnqt/revision.hpp>

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class RevGraphView;
class QSplitter;
class KTextBrowser;
class CContextListener;

namespace svn {
    class LogEntry;
    class Client;
}

class RevTreeWidget : public QWidget
{
    Q_OBJECT

public:
    RevTreeWidget(QObject*,svn::Client*,QWidget* parent = 0, const char* name = 0);
    ~RevTreeWidget();

    QSplitter* m_Splitter;
    RevGraphView* m_RevGraphView;

    void setBasePath(const QString&);
    void dumpRevtree();

protected:
    QVBoxLayout* RevTreeWidgetLayout;
    KTextBrowser* m_Detailstext;

signals:
    void makeCat(const svn::Revision&,const QString&,const QString&,const svn::Revision&,QWidget*);
    void makeNorecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);
    void makeRecDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);

protected slots:
    virtual void setDetailText(const QString&);

private:
    QPixmap image0;

};

#endif // REVTREEWIDGET_H
