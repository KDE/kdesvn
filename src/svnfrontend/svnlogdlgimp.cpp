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
#include "svncpp/log_entry.hpp"
#include "helpers/sub2qt.h"
#include <qdatetime.h>
#include <klistview.h>
#include <ktextbrowser.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <kapp.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <ktabwidget.h>
#include <kdebug.h>
#include "kdesvn_part_config.h"

#include <list>


const char* SvnLogDlgImp::groupName = "log_dialog_size";

#define INHERITED KListViewItem
class LogListViewItem:public INHERITED
{
public:
    LogListViewItem (KListView *parent,const svn::LogEntry&);
    virtual int compare( QListViewItem* i, int col, bool ascending ) const;

    static const int COL_REV,COL_AUTHOR,COL_DATE;
    const QString&message()const;
    svn_revnum_t rev()const{return _revision;}

protected:
    svn_revnum_t _revision;
    QDateTime fullDate;
    QString _message;
};

const int LogListViewItem::COL_REV = 1;
const int LogListViewItem::COL_AUTHOR = 0;
const int LogListViewItem::COL_DATE = 2;

LogListViewItem::LogListViewItem(KListView*_parent,const svn::LogEntry&_entry)
    : INHERITED(_parent)
{
    setMultiLinesEnabled(true);
    _revision=_entry.revision;
    fullDate=helpers::sub2qt::apr_time2qt(_entry.date);
    setText(COL_REV,QString("%1").arg(_revision));
    setText(COL_AUTHOR,_entry.author);
    setText(COL_DATE,fullDate.toString(Qt::LocalDate));
    _message = _entry.message;
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
    :SvnLogDialogData(parent, name),_name("")
{
    m_LogView->setSorting(LogListViewItem::COL_REV);
    resize(dialogSize());
}

void SvnLogDlgImp::dispLog(const svn::LogEntries*_log,const QString & what)
{
    if (!_log) return;
    svn::LogEntries::const_iterator lit;
    LogListViewItem * item;
    for (lit=_log->begin();lit!=_log->end();++lit) {
        item = new LogListViewItem(m_LogView,*lit);
    }
    _name = what;
}

#include "svnlogdlgimp.moc"


/*!
    \fn SvnLogDlgImp::slotItemClicked(QListViewItem*)
 */
void SvnLogDlgImp::slotSelectionChanged(QListViewItem*_it)
{
    if (!_it) {
        m_DispPrevButton->setEnabled(false);
        return;
    }

    LogListViewItem* k = static_cast<LogListViewItem*>( _it );
    m_LogDisplay->setText(k->message());
    k = static_cast<LogListViewItem*>(_it->nextSibling());
    if (!k) {
        m_DispPrevButton->setEnabled(false);
    } else {
        m_DispPrevButton->setEnabled(true);
    }
}


/*!
    \fn SvnLogDlgImp::slotDispPrevious()
 */
void SvnLogDlgImp::slotDispPrevious()
{
    LogListViewItem* k = static_cast<LogListViewItem*>(m_LogView->selectedItem());
    if (!k) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    LogListViewItem* p = static_cast<LogListViewItem*>(k->nextSibling());
    if (!p) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    emit makeDiff(_name,p->rev(),k->rev());
}


/*!
    \fn SvnLogDlgImp::saveSize()
 */
void SvnLogDlgImp::saveSize()
{
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    KConfigGroupSaver cs(kdesvnPart_config::config(), groupName);
    QSize sizeToSave = size();
    kdesvnPart_config::config()->writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),
        QString::number( sizeToSave.width()), true, false);
    kdesvnPart_config::config()->writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),
        QString::number( sizeToSave.height()), true, false);
}

QSize SvnLogDlgImp::dialogSize()
{
    int w, h;
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    w = sizeHint().width();
    h = sizeHint().height();
    KConfigGroupSaver cs(kdesvnPart_config::config(), groupName);
    w = kdesvnPart_config::config()->readNumEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
    h = kdesvnPart_config::config()->readNumEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );
    return( QSize( w, h ) );
}
