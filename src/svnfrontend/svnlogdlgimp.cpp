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
#include "svnlogdlgimp.h"
#include "src/settings/kdesvnsettings.h"
#include "src/svnqt/log_entry.hpp"
#include "helpers/sub2qt.h"
#include "svnactions.h"

#include <klistview.h>
#include <ktextbrowser.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapp.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <ktabwidget.h>
#include <kdebug.h>

#include <qdatetime.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qtextstream.h>
#include <qpopupmenu.h>

#include <list>


const char* SvnLogDlgImp::groupName = "log_dialog_size";

class LogListViewItem:public KListViewItem
{
public:
    LogListViewItem (KListView *parent,const svn::LogEntry&);
    virtual int compare( QListViewItem* i, int col, bool ascending ) const;

    static const int COL_REV,COL_AUTHOR,COL_DATE,COL_MSG;
    const QString&message()const;
    svn_revnum_t rev()const{return _revision;}
    void showChangedEntries(KListView*);
    unsigned int numChangedEntries(){return changedPaths.count();}
    void setChangedEntries(const svn::LogEntry&);
    void setRealName(const QString&_n){_realName=_n;}
    const QString&realName()const{return _realName;}

    bool copiedFrom(QString&_n,long&rev)const;
    static bool isParent(const QString&_par,const QString&tar);

protected:
    svn_revnum_t _revision;
    QDateTime fullDate;
    QString _message,_realName;
    QValueList<svn::LogChangePathEntry> changedPaths;
};

const int LogListViewItem::COL_REV = 2;
const int LogListViewItem::COL_AUTHOR = 1;
const int LogListViewItem::COL_DATE = 3;
const int LogListViewItem::COL_MSG = 4;

class LogChangePathItem:public KListViewItem
{
public:
    LogChangePathItem(KListView*parent,const svn::LogChangePathEntry&);
    virtual ~LogChangePathItem(){}

    QChar action() const{return _action;}
    const QString& path() const{return _path;}
    const QString& source() const{return _source;}
    svn_revnum_t revision() const{ return _revision;}

protected:
    QString _path,_source;
    QChar _action;
    svn_revnum_t _revision;
};

LogListViewItem::LogListViewItem(KListView*_parent,const svn::LogEntry&_entry)
    : KListViewItem(_parent),_realName(QString::null)
{
    setMultiLinesEnabled(false);
    _revision=_entry.revision;
    fullDate=svn::DateTime(_entry.date);
    setText(COL_REV,QString("%1").arg(_revision));
    setText(COL_AUTHOR,_entry.author);
    setText(COL_DATE,helpers::sub2qt::apr_time2qtString(_entry.date));
    _message = _entry.message;
    QStringList sp = QStringList::split("\n",_message);
    if (sp.count()==0) {
        setText(COL_MSG,_message);
    } else {
        setText(COL_MSG,sp[0]);
    }
    changedPaths = _entry.changedPaths;
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

void LogListViewItem::showChangedEntries(KListView*where)
{
    if (!where)return;
    where->clear();
    if (changedPaths.count()==0) {
        return;
    }
    for (unsigned i = 0; i < changedPaths.count();++i) {
        new LogChangePathItem(where,changedPaths[i]);
    }
}

LogChangePathItem::LogChangePathItem(KListView*parent,const svn::LogChangePathEntry&e)
    :KListViewItem(parent)
{
    _action = QChar(e.action);
    setText(0,_action);
    _path = e.path;
    setText(1,e.path);
    _revision = e.copyFromRevision;
    _source = e.copyFromPath;
    if (e.copyFromRevision>-1)
    {
        setText(2,i18n("%1 at revision %2").arg(e.copyFromPath).arg(e.copyFromRevision));
    }
}

void LogListViewItem::setChangedEntries(const svn::LogEntry&_entry)
{
    changedPaths = _entry.changedPaths;
}

bool LogListViewItem::copiedFrom(QString&_n,long&_rev)const
{
    for (unsigned i = 0; i < changedPaths.count();++i) {
        if (changedPaths[i].action=='A' &&
            !changedPaths[i].copyFromPath.isEmpty() &&
            isParent(changedPaths[i].path,_realName)) {
            kdDebug()<<_realName<< " - " << changedPaths[i].path << endl;
            QString tmpPath = _realName;
            QString r = _realName.mid(changedPaths[i].path.length());
            _n=changedPaths[i].copyFromPath;
            _n+=r;
            _rev = changedPaths[i].copyFromRevision;
            kdDebug()<<"Found switch from  "<< changedPaths[i].copyFromPath << " rev "<<changedPaths[i].copyFromRevision<<endl;
            kdDebug()<<"Found switch from  "<< _n << " rev "<<_rev<<endl;
            return true;
        }
    }
    return false;
}

bool LogListViewItem::isParent(const QString&_par,const QString&tar)
{
    if (_par==tar) return true;
    QString par = _par+(_par.endsWith("/")?"":"/");
    return tar.startsWith(par);
}

SvnLogDlgImp::SvnLogDlgImp(SvnActions*ac,QWidget *parent, const char *name)
    :SvnLogDialogData(parent, name),_name("")
{
    m_LogView->setSorting(LogListViewItem::COL_REV);
    m_LogView->header()->setLabel( 0, "");
    resize(dialogSize());
    m_ControlKeyDown = false;
    m_first = 0;
    m_second = 0;

    if (Kdesvnsettings::self()->log_always_list_changed_files()) {
        buttonListFiles->hide();
    } else {
        m_ChangedList->hide();
    }
    m_Actions = ac;
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    QString t1 = cs.readEntry("logsplitter",QString::null);
    if (!t1.isEmpty()) {
        if (cs.readBoolEntry("laststate",false)==m_ChangedList->isHidden()) {
            QTextStream st2(&t1,IO_ReadOnly);
            st2 >> *m_centralSplitter;
        }
    }
}

SvnLogDlgImp::~SvnLogDlgImp()
{
    QString t1,t2;
    QTextStream st1(&t1,IO_WriteOnly);
    st1 << *m_centralSplitter;
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    cs.writeEntry("logsplitter",t1);
    cs.writeEntry("laststate",m_ChangedList->isHidden());
}

void SvnLogDlgImp::dispLog(const svn::SharedPointer<svn::LogEntriesMap>&_log,const QString & what,const QString&root)
{
    if (!_log) return;
    _base = root;
    m_Entries = _log;
    kdDebug()<<"What: "<<what << endl;
    if (!what.isEmpty()){
        setCaption(i18n("SVN Log of %1").arg(what));
    } else {
        setCaption(i18n("SVN Log"));
    }
    svn::LogEntriesMap::const_iterator lit;
    LogListViewItem * item;
    QMap<long int,QString> namesMap;
    QMap<long int,LogListViewItem*> itemMap;
    long min,max;
    min = max = -1;
    for (lit=_log->begin();lit!=_log->end();++lit) {
        item = new LogListViewItem(m_LogView,(*lit));
        if ((*lit).revision>max) max = (*lit).revision;
        if ((*lit).revision<min || min == -1) min = (*lit).revision;
        itemMap[(*lit).revision]=item;
    }
    if (itemMap.count()==0) {
        return;
    }
    m_LogView->setSelected(m_LogView->firstChild(),true);
    QString bef = what;
    long rev;
    // YES! I'd checked it: this is much faster than getting list of keys
    // and iterating over that list!
    for (long c=max;c>-1;--c) {
        if (!itemMap.contains(c)) {
            continue;
        }
        if (itemMap[c]->realName().isEmpty()) {
            itemMap[c]->setRealName(bef);
        }
        itemMap[c]->copiedFrom(bef,rev);
    }
    _name = what;
}


/*!
    \fn SvnLogDlgImp::slotItemClicked(QListViewItem*)
 */
void SvnLogDlgImp::slotSelectionChanged(QListViewItem*_it)
{
    if (!_it) {
        m_DispPrevButton->setEnabled(false);
        buttonListFiles->setEnabled(false);
        buttonBlame->setEnabled(false);
        m_ChangedList->clear();
        return;
    }
    LogListViewItem* k = static_cast<LogListViewItem*>( _it );
    if (k->numChangedEntries()==0) {
        buttonListFiles->setEnabled(true);
        if (m_ChangedList->isVisible()){
            m_ChangedList->hide();
        }
    } else {
        buttonListFiles->setEnabled(false);
        if (!m_ChangedList->isVisible()){
            m_ChangedList->show();
        }
    }
    m_LogDisplay->setText(k->message());
    k->showChangedEntries(m_ChangedList);
    buttonBlame->setEnabled(true);

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
    QString s,e;
    s = _base+k->realName();
    e = _base+p->realName();
    emit makeDiff(e,p->rev(),s,k->rev(),this);
}


/*!
    \fn SvnLogDlgImp::saveSize()
 */
void SvnLogDlgImp::saveSize()
{
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    KConfigGroupSaver cs(Kdesvnsettings::self()->config(), groupName);
    QSize sizeToSave = size();
    Kdesvnsettings::self()->config()->writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),
        QString::number( sizeToSave.width()), true, false);
    Kdesvnsettings::self()->config()->writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),
        QString::number( sizeToSave.height()), true, false);
}

QSize SvnLogDlgImp::dialogSize()
{
    int w, h;
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    w = sizeHint().width();
    h = sizeHint().height();
    KConfigGroupSaver cs(Kdesvnsettings::self()->config(), groupName);
    w = Kdesvnsettings::self()->config()->readNumEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
    h = Kdesvnsettings::self()->config()->readNumEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );
    return( QSize( w, h ) );
}

void SvnLogDlgImp::slotItemClicked(int button,QListViewItem*item,const QPoint &,int)
{
    if (!item) {
        m_ChangedList->clear();
        return;
    }
    LogListViewItem*which = static_cast<LogListViewItem*>(item);
    /* left mouse */
    if (button == 1&&!m_ControlKeyDown) {
        if (m_first) m_first->setText(0,"");
        if (m_first == which) {
            m_first = 0;
        } else {
            m_first = which;
            m_first->setText(0,"1");
        }
        if (m_first==m_second) {
            m_second = 0;
        }
    /* other mouse or ctrl hold*/
    } else {
        if (m_second) m_second->setText(0,"");
        if (m_second == which) {
            m_second = 0;
        } else {
            m_second = which;
            m_second->setText(0,"2");
        }
        if (m_first==m_second) {
            m_first = 0;
        }
    }
    m_DispSpecDiff->setEnabled(m_first!=0 && m_second!=0);
}

void SvnLogDlgImp::slotDispSelected()
{
    if (!m_first || !m_second) return;
    emit makeDiff(_base+m_first->realName(),m_first->rev(),_base+m_second->realName(),m_second->rev(),this);
}

bool SvnLogDlgImp::getSingleLog(svn::LogEntry&t,const svn::Revision&r,const QString&what,const svn::Revision&peg,QString&root)
{
    root = _base;
    if (m_Entries->find(r.revnum()) == m_Entries->end())
    {
        return m_Actions->getSingleLog(t,r,what,peg,root);
    }
    t=(*m_Entries)[r.revnum()];
    return true;
}

void SvnLogDlgImp::slotListEntries()
{
    LogListViewItem * it = static_cast<LogListViewItem*>(m_LogView->selectedItem());
    if (!it||it->numChangedEntries()>0||!m_Actions) {
        buttonListFiles->setEnabled(false);
        return;
    }
    svn::SharedPointer<svn::LogEntriesMap>_log = m_Actions->getLog(it->rev(),it->rev(),_name,true,0);
    if (!_log) {
        return;
    }
    if (_log->count()>0) {
        it->setChangedEntries((*_log)[it->rev()]);
        it->showChangedEntries(m_ChangedList);
        if (!m_ChangedList->isVisible()) m_ChangedList->show();
    }
    buttonListFiles->setEnabled(false);
}

void SvnLogDlgImp::keyPressEvent (QKeyEvent * e)
{
    if (!e) return;
    if (e->text().isEmpty()&&e->key()==Key_Control) {
        m_ControlKeyDown = true;
    }
    SvnLogDialogData::keyPressEvent(e);
}

void SvnLogDlgImp::keyReleaseEvent (QKeyEvent * e)
{
    if (!e) return;
    if (e->text().isEmpty()&&e->key()==Qt::Key_Control) {
        m_ControlKeyDown = false;
    }
    SvnLogDialogData::keyReleaseEvent(e);
}

void SvnLogDlgImp::slotBlameItem()
{
    LogListViewItem* k = static_cast<LogListViewItem*>(m_LogView->selectedItem());
    if (!k) {
        buttonBlame->setEnabled(false);
        return;
    }
    svn::Revision start(svn::Revision::START);
    m_Actions->makeBlame(start,k->rev(),_base+k->realName(),kapp->activeModalWidget(),k->rev(),this);
}

void SvnLogDlgImp::slotEntriesSelectionChanged()
{
}

void SvnLogDlgImp::slotSingleContext(QListViewItem*_item, const QPoint & e, int)
{
    if (!_item)
    {
        return;
    }

    LogChangePathItem* item = static_cast<LogChangePathItem*>(_item);
    LogListViewItem* k = static_cast<LogListViewItem*>(m_LogView->selectedItem());
    if (!k) {
        kdDebug()<<"????"<<endl;
        return;
    }
    QPopupMenu popup;
    QString name = item->path();
    QString action = item->action();
    QString source =item->revision()>-1?item->source():item->path();
    svn_revnum_t prev = item->revision()>0?item->revision():k->rev()-1;
    if (action != "D") {
        popup.insertItem(i18n("Annotate"),101);
        if (action != "A" || item->revision()>-1) {
            popup.insertItem(i18n("Diff previous"),102);
        }
    }
    int r = popup.exec(e);
    svn::Revision start(svn::Revision::START);
    switch (r)
    {
        case 101:
        {
            m_Actions->makeBlame(start,k->rev(),_base+name,kapp->activeModalWidget(),k->rev(),this);
            break;
        }
        case 102:
        {
            emit makeDiff(_base+source,prev,_base+name,k->rev(),this);
            break;
        }
        default:
            break;
    }
}

void SvnLogDlgImp::slotSingleDoubleClicked(QListViewItem*_item)
{
    if (!_item)
    {
        return;
    }

    LogChangePathItem* item = static_cast<LogChangePathItem*>(_item);
    LogListViewItem* k = static_cast<LogListViewItem*>(m_LogView->selectedItem());
    if (!k) {
        kdDebug()<<"????"<<endl;
        return;
    }
    QString name = item->path();
    QString action = item->action();
    QString source =item->revision()>-1?item->source():item->path();
    //svn_revnum_t prev = item->revision()>0?item->revision():k->rev()-1;
    svn::Revision start(svn::Revision::START);
    if (action != "D") {
        m_Actions->makeBlame(start,k->rev(),_base+name,kapp->activeModalWidget(),k->rev(),this);
    }
}






#include "svnlogdlgimp.moc"
