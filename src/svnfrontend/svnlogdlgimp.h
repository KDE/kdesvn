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
#include "svncpp/log_entry.hpp"
#include "svncpp/client.hpp"

#include <qsize.h>

class LogListViewItem;

class SvnLogDlgImp: public SvnLogDialogData {
Q_OBJECT
public:
    SvnLogDlgImp(QWidget *parent = 0, const char *name = 0);
    void dispLog(const svn::LogEntries*,const QString&);
    void saveSize();
    QSize dialogSize();

signals:
    void makeDiff(const QString&,const svn::Revision&,const svn::Revision&);

protected slots:
    virtual void slotSelectionChanged(QListViewItem*);
protected slots:
    virtual void slotDispPrevious();
    virtual void slotDispSelected();
    virtual void slotItemClicked(int,QListViewItem*,const QPoint &,int);
protected:
    QString _name;
    static const char* groupName;
    LogListViewItem *m_first,*m_second;
};

#endif
