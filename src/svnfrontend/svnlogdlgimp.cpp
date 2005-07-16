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
#include <ktextbrowser.h>

class LogListViewItem:public KListViewItem
{
public:
    LogListViewItem (KListView *parent,const svn::LogEntry&);
    virtual int compare( QListViewItem* i, int col, bool ascending ) const;

    static const int COL_REV,COL_AUTHOR,COL_DATE;
    const QString&message()const;
protected:
    svn_revnum_t _revision;
    QDateTime fullDate;
    QString _message;
};

const int LogListViewItem::COL_REV = 1;
const int LogListViewItem::COL_AUTHOR = 0;
const int LogListViewItem::COL_DATE = 2;

LogListViewItem::LogListViewItem(KListView*_parent,const svn::LogEntry&_entry)
    : KListViewItem(_parent)
{
    setMultiLinesEnabled(true);
    _revision=_entry.revision;
    fullDate.setTime_t(_entry.date/(1000*1000),Qt::UTC);
    setText(COL_REV,QString("%1").arg(_revision));
    setText(COL_AUTHOR,QString::fromLocal8Bit(_entry.author.c_str()));
    setText(COL_DATE,fullDate.toString(Qt::LocalDate));
    _message = QString::fromLocal8Bit(_entry.message.c_str());
    //setText(COL_MSG,_entry.message.c_str());
}
const QString&LogListViewItem::message()const
{
    return _message;
}

int LogListViewItem::compare( QListViewItem* item, int col, bool ) const
{
    LogListViewItem* k = static_cast<LogListViewItem*>( item );
    if (col==COL_REV) {
        return k->_revision-_revision;
    }
    if (col==COL_DATE) {
        return fullDate.secsTo(k->fullDate);
    }
    return text(col).localeAwareCompare(k->text(col));
}

SvnLogDlgImp::SvnLogDlgImp(QWidget *parent, const char *name)
    :SvnLogDialogData(parent, name)
{
    m_LogView->setSorting(LogListViewItem::COL_REV);
}

void SvnLogDlgImp::dispLog(const svn::LogEntries*_log)
{
    if (!_log) return;
    svn::LogEntries::const_iterator lit;
    LogListViewItem * item;
    for (lit=_log->begin();lit!=_log->end();++lit) {
        item = new LogListViewItem(m_LogView,*lit);
    }
}

#include "svnlogdlgimp.moc"


/*!
    \fn SvnLogDlgImp::slotItemClicked(QListViewItem*)
 */
void SvnLogDlgImp::slotItemChanged(QListViewItem*_it)
{
    if (!_it) return;
    LogListViewItem* k = static_cast<LogListViewItem*>( _it );
    m_LogDisplay->setText(k->message());
}
