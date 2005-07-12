/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht   *
 *   ral@alwins-world.de   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "svnlogdlgimp.h"
#include <qdatetime.h>
#include <klistview.h>

SvnLogDlgImp::SvnLogDlgImp(QWidget *parent, const char *name)
    :SvnLogDialogData(parent, name)
{
    m_LogView->setSorting(-1);
}

void SvnLogDlgImp::dispLog(const svn::LogEntries*_log)
{
    if (!_log) return;
    svn::LogEntries::const_iterator lit;
    KListViewItem * item,*pitem;
    pitem =0;
    for (lit=_log->begin();lit!=_log->end();++lit) {
        item = new KListViewItem(m_LogView,pitem);
        item->setMultiLinesEnabled(true);
        item->setText(0,QString("%1").arg(lit->revision));
        item->setText(1,lit->author.c_str());
        QDateTime t;
        t.setTime_t(lit->date/(1000*1000),Qt::UTC);
        item->setText(2,t.toString(Qt::LocalDate));
        item->setText(3,lit->message.c_str());
        pitem = item;
        //qDebug("***\n%i: %s\n---",lit->revision,lit->message.size()?lit->message.c_str():"*** empty log ***");
    }
}

#include "svnlogdlgimp.moc"
