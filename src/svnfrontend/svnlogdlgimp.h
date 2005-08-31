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
#include "src/svncpp/log_entry.hpp"
#include "src/svncpp/client.hpp"

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
protected:
    QString _name;
    static const char* groupName;
};

#endif
