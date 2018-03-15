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
#ifndef REVTREEWIDGET_H
#define REVTREEWIDGET_H

#include <svnqt/client.h>
#include <svnqt/revision.h>

#include <qwidget.h>

class QVBoxLayout;
class RevGraphView;
class QSplitter;
class QTextBrowser;

namespace svn
{
class LogEntry;
}

class RevTreeWidget : public QWidget
{
    Q_OBJECT

public:
    RevTreeWidget(const svn::ClientP &cl, QWidget *parent = nullptr);
    ~RevTreeWidget();

    QSplitter *m_Splitter;
    RevGraphView *m_RevGraphView;

    void setBasePath(const QString &);
    void dumpRevtree();

protected:
    QVBoxLayout *RevTreeWidgetLayout;
    QTextBrowser *m_Detailstext;

signals:
    void makeCat(const svn::Revision &, const QString &, const QString &, const svn::Revision &, QWidget *);
    void makeNorecDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);
    void makeRecDiff(const QString &, const svn::Revision &, const QString &, const svn::Revision &, QWidget *);

protected slots:
    virtual void setDetailText(const QString &);
};

#endif // REVTREEWIDGET_H
