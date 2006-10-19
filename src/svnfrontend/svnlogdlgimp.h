/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#ifndef SVNLOGDLGIMP_H
#define SVNLOGDLGIMP_H

#include "svnlogdlg.h"
#include "src/svnqt/log_entry.hpp"
#include "src/svnqt/client.hpp"
#include "src/svnqt/shared_pointer.hpp"

#include <qsize.h>

class LogListViewItem;
class SvnActions;

class SvnLogDlgImp: public SvnLogDialogData {
Q_OBJECT
public:
    SvnLogDlgImp(SvnActions*,QWidget *parent = 0, const char *name = 0);
    virtual ~SvnLogDlgImp();
    void dispLog(const svn::SharedPointer<svn::LogEntries>&,const QString&,const QString&);
    void saveSize();
    QSize dialogSize();

signals:
    void makeDiff(const QString&,const svn::Revision&,const QString&,const svn::Revision&,QWidget*);

protected slots:
    virtual void slotSelectionChanged(QListViewItem*);
protected slots:
    virtual void slotDispPrevious();
    virtual void slotDispSelected();
    virtual void slotItemClicked(int,QListViewItem*,const QPoint &,int);
protected:
    QString _name;
    QString _base;
    static const char* groupName;
    LogListViewItem *m_first,*m_second;
    SvnActions*m_Actions;
    bool m_ControlKeyDown;
    virtual void keyPressEvent (QKeyEvent * e);
    virtual void keyReleaseEvent (QKeyEvent * e);
    virtual void slotBlameItem();
    svn::SharedPointer<svn::LogEntries> m_Entries;

protected slots:
    virtual void slotListEntries();
    virtual void slotEntriesSelectionChanged();
};

#endif
