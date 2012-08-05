/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include "helpers/sub2qt.h"
#include "svnactions.h"
#include "src/svnfrontend/fronthelpers/revisionbuttonimpl.h"
#include "src/svnfrontend/models/logitemmodel.h"
#include "src/svnfrontend/models/logmodelhelper.h"

#include <ktextbrowser.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <ktabwidget.h>
#include <kmenu.h>
#include <kdebug.h>

#include <QDateTime>
#include <QSplitter>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <QSortFilterProxyModel>
#include <QTextDocumentFragment>

const char* SvnLogDlgImp::groupName = "log_dialog_size";

SvnLogDlgImp::SvnLogDlgImp(SvnActions*ac,QWidget *parent, const char *name,bool modal)
    :KDialog(parent),_name("")
{
    setupUi(this);
    setMainWidget(mMainWidget);
    setObjectName(name);
    setModal(modal);
    setHelp("logdisplay-dlg","kdesvn");
    setButtons(Help|Close);
    QWidget * b = button(Help);
    if (b) {
        m_ButtonLayout->addWidget(b);
    }
    b = button(Close);
    if (b) {
        m_ButtonLayout->addWidget(b);
    }
    m_DispPrevButton->setIcon(KIcon("kdesvndiff"));
    m_DispSpecDiff->setIcon(KIcon("kdesvndiff"));
    buttonBlame->setIcon(KIcon("kdesvnblame"));
    m_SortModel = 0;
    m_CurrentModel=0;

    m_ControlKeyDown = false;

    if (Kdesvnsettings::self()->log_always_list_changed_files()) {
        buttonListFiles->hide();
    } else {
        m_ChangedList->hide();
    }
    m_Actions = ac;
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    QByteArray t1 = cs.readEntry("logsplitter",QByteArray());
    if (!t1.isEmpty()) {
        m_centralSplitter->restoreState(t1);
    }
    t1 = cs.readEntry("right_logsplitter",QByteArray());
    if (!t1.isEmpty()) {
        if (cs.readEntry("laststate",false)==m_ChangedList->isHidden()) {
            m_rightSplitter->restoreState(t1);
        }
    }
}

SvnLogDlgImp::~SvnLogDlgImp()
{
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    cs.writeEntry("right_logsplitter",m_rightSplitter->saveState());
    cs.writeEntry("logsplitter",m_centralSplitter->saveState());
    cs.writeEntry("laststate",m_ChangedList->isHidden());
    delete m_SortModel;
}

void SvnLogDlgImp::loadSize()
{
    KConfigGroup _k(Kdesvnsettings::self()->config(),groupName);
    restoreDialogSize(_k);
}

void SvnLogDlgImp::dispLog(const svn::SharedPointer<svn::LogEntriesMap>&_log,const QString & what,const QString&root,const svn::Revision&peg,const QString&pegUrl)
{
    m_peg = peg;
    m_PegUrl = pegUrl;
    m_startRevButton->setNoWorking(m_PegUrl.isUrl());
    m_endRevButton->setNoWorking(m_PegUrl.isUrl());
    if (!m_PegUrl.isUrl() || Kdesvnsettings::remote_special_properties()) {
        QString s = m_Actions->searchProperty(_bugurl,"bugtraq:url",pegUrl,peg,true);
        if (!s.isEmpty() ){
            QString reg;
            s = m_Actions->searchProperty(reg,"bugtraq:logregex",pegUrl,peg,true);
            if (!s.isNull() && !reg.isEmpty()) {
                QStringList s1 = reg.split('\n');
                if (s1.size()>0) {
                    _r1.setPattern(s1[0]);
                    if (s1.size()>1) {
                        _r2.setPattern(s1[1]);
                    }
                }
            }
        }
    }
    _base = root;
    m_Entries = _log;
    if (!what.isEmpty()){
        setWindowTitle(i18n("SVN Log of %1",what));
    } else {
        setWindowTitle(i18n("SVN Log"));
    }
    _name = what;
    dispLog(_log);
}

void SvnLogDlgImp::dispLog(const svn::SharedPointer<svn::LogEntriesMap>&_log)
{
    if (!_log) {
        return;
    }
    bool must_init = false;
    if (!m_SortModel) {
        m_SortModel = new QSortFilterProxyModel(m_LogTreeView);
        m_CurrentModel = new SvnLogModel(_log,_name,m_SortModel);
        m_SortModel->setSourceModel(m_CurrentModel);
        must_init = true;
    } else {
        m_CurrentModel->setLogData(_log,_name);
    }

    if (must_init) {
        m_LogTreeView->setModel(m_SortModel);
        m_LogTreeView->sortByColumn(SvnLogModel::Revision,Qt::DescendingOrder);
        connect(m_LogTreeView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this,SLOT(slotSelectionChanged(const QItemSelection&,const QItemSelection&)));
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Revision);
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Author);
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Date);
        loadSize();
    }
    m_startRevButton->setRevision(m_CurrentModel->max());
    m_endRevButton->setRevision(m_CurrentModel->min());
    QModelIndex ind = m_CurrentModel->index(m_CurrentModel->rowCount(QModelIndex())-1);
    if (ind.isValid()) {
        m_LogTreeView->selectionModel()->select(m_SortModel->mapFromSource(ind),
            QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    m_LogTreeView->setFocus();
}

QString SvnLogDlgImp::genReplace(const QString&r1match)
{
    static QString anf("<a href=\"");
    static QString mid("\">");
    static QString end("</a>");
    QString res("");
    if (_r2.pattern().length()<1) {
        res = _bugurl;
        res.replace("%BUGID%",_r1.cap(1));
        res = anf+res+mid+r1match+end;
        return res;
    }
    int pos=0;
    int count=0;
    int oldpos;

    while (pos > -1) {
        oldpos = pos+count;
        pos = r1match.indexOf(_r2,pos+count);
        if (pos==-1) {
            break;
        }
        count = _r2.matchedLength();
        res+=r1match.mid(oldpos,pos-oldpos);
        QString sub = r1match.mid(pos,count);
        QString _url = _bugurl;
        _url.replace("%BUGID%",sub);
        res+=anf+_url+mid+sub+end;
    }
    res+=r1match.mid(oldpos);
    return res;
}

void SvnLogDlgImp::replaceBugids(QString&msg)
{
    if (!_r1.isValid() || _r1.pattern().length()<1 || _bugurl.isEmpty()) {
        return;
    }
    int pos = 0;
    int count = 0;

    pos = _r1.indexIn(msg,pos+count);
    count = _r1.matchedLength();

    while (pos>-1) {
        QString s1 = msg.mid(pos,count);
        QString rep = genReplace(s1);
        msg = msg.replace(pos,count,rep);
        pos = _r1.indexIn(msg,pos+rep.length());
        count = _r1.matchedLength();
    }
}

void SvnLogDlgImp::slotSelectionChanged(const QItemSelection & current, const QItemSelection & previous)
{
    Q_UNUSED(previous);
    m_ChangedList->clear();
    QModelIndexList _l = current.indexes();
    if (_l.count()<1) {
        m_DispPrevButton->setEnabled(false);
        buttonListFiles->setEnabled(false);
        buttonBlame->setEnabled(false);
        m_ChangedList->clear();
        return;
    }
    QModelIndex _index = m_SortModel->mapToSource(_l[0]);
    m_CurrentModel->fillChangedPaths(_index,m_ChangedList);
    QTextDocumentFragment _m = QTextDocumentFragment::fromPlainText(m_CurrentModel->fullMessage(_index));
    QString msg = _m.toHtml();
    replaceBugids(msg);
    m_LogDisplay->setHtml(msg);
    if (_index.row()>0) {
        QModelIndex _it = m_CurrentModel->index(_index.row()-1);
        m_DispPrevButton->setEnabled(true);
    } else {
        m_DispPrevButton->setEnabled(false);
    }
    buttonBlame->setEnabled(true);
}


/*!
    \fn SvnLogDlgImp::slotDispPrevious()
 */
void SvnLogDlgImp::slotDispPrevious()
{
    QModelIndex _index = selectedRow();
    if (!_index.isValid() || _index.row()==0) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    QModelIndex _it = m_CurrentModel->index(_index.row()-1);
    if (!_it.isValid()) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    QString s,e;
    SvnLogModelNodePtr k = m_CurrentModel->indexNode(_index);
    SvnLogModelNodePtr p = m_CurrentModel->indexNode(_it);
    if (!k || !p) {
        m_DispPrevButton->setEnabled(false);
        return;
    }

    s = _base+k->realName();
    e = _base+p->realName();
    emit makeDiff(e,p->revision(),s,k->revision(),this);
}


/*!
    \fn SvnLogDlgImp::saveSize()
 */
void SvnLogDlgImp::saveSize()
{
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    QSize sizeToSave = size();
    cs.writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),QString::number( sizeToSave.width()));
    cs.writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),QString::number( sizeToSave.height()));
}

void SvnLogDlgImp::slotRevisionSelected()
{
    m_goButton->setFocus();
    //m_DispSpecDiff->setEnabled( m_first && m_second && m_first != m_second);
}

void SvnLogDlgImp::slotDispSelected()
{
    SvnLogModelNodePtr m_first = m_CurrentModel->indexNode(m_CurrentModel->index(m_CurrentModel->leftRow()));
    SvnLogModelNodePtr m_second = m_CurrentModel->indexNode(m_CurrentModel->index(m_CurrentModel->rightRow()));
    if (m_first && m_second) {
        emit makeDiff(_base+m_first->realName(),m_first->revision(),_base+m_second->realName(),m_second->revision(),this);
    }
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

void SvnLogDlgImp::slotGetLogs()
{
    svn::SharedPointer<svn::LogEntriesMap> lm = m_Actions->getLog(m_startRevButton->revision(),
            m_endRevButton->revision(),m_peg,
            _base+'/'+_name,Kdesvnsettings::self()->log_always_list_changed_files(),0,Kdesvnsettings::last_node_follow(),this);
    if (lm) {
        dispLog(lm);
    }
}

void SvnLogDlgImp::slotPrevFifty()
{
    svn::Revision now = m_CurrentModel->min();
    if (now == 1) {
        return;
    }
    svn::Revision begin=now.revnum()-1;
    if (begin.revnum()<1) {
        begin = 1;
    }
    svn::SharedPointer<svn::LogEntriesMap> lm = m_Actions->getLog(begin,
                            (begin.revnum()>50?svn::Revision::START:svn::Revision::HEAD),m_peg,
                            _base+'/'+_name,Kdesvnsettings::self()->log_always_list_changed_files(),50,Kdesvnsettings::last_node_follow(),this);
    if (lm) {
        dispLog(lm);
    }
}

void SvnLogDlgImp::slotBeginHead()
{
    svn::SharedPointer<svn::LogEntriesMap> lm = m_Actions->getLog(svn::Revision::HEAD,
                                        1,m_peg,
                                        _base+'/'+_name,Kdesvnsettings::self()->log_always_list_changed_files(),50,Kdesvnsettings::last_node_follow(),this);
    if (lm) {
        dispLog(lm);
    }
}

void SvnLogDlgImp::slotListEntries()
{
    QModelIndex _index = selectedRow();
    SvnLogModelNodePtr ptr=m_CurrentModel->indexNode(_index);
    if (!ptr) {
        buttonListFiles->setEnabled(false);
        return;
    }
    if (ptr->changedPaths().count()==0) {
        svn::SharedPointer<svn::LogEntriesMap>_log = m_Actions->getLog(ptr->revision(),ptr->revision(),ptr->revision(),
        _name,true,0,Kdesvnsettings::last_node_follow());
        if (!_log) {
            return;
        }
        if (_log->count()>0) {
            ptr->setChangedPaths((*_log)[ptr->revision()]);
        }
    }
    if (ptr->changedPaths().count()==0) {
        m_CurrentModel->fillChangedPaths(_index,m_ChangedList);
    }
    buttonListFiles->setEnabled(false);
}

void SvnLogDlgImp::keyPressEvent (QKeyEvent * e)
{
    if (!e) return;
    if (e->text().isEmpty()&&e->key()==Qt::Key_Control) {
        m_ControlKeyDown = true;
    }
    KDialog::keyPressEvent(e);
}

void SvnLogDlgImp::keyReleaseEvent (QKeyEvent * e)
{
    if (!e) return;
    if (e->text().isEmpty()&&e->key()==Qt::Key_Control) {
        m_ControlKeyDown = false;
    }
    KDialog::keyReleaseEvent(e);
}

void SvnLogDlgImp::slotBlameItem()
{
    QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        buttonBlame->setEnabled(false);
        return;
    }
    QLONG rev = m_CurrentModel->toRevision(ind);
    svn::Revision start(svn::Revision::START);
    m_Actions->makeBlame(start,rev,_base+m_CurrentModel->realName(ind),kapp->activeModalWidget(),rev,this);
}

/* it works 'cause we use single selection only */
QModelIndex SvnLogDlgImp::selectedRow(int column)
{
    QModelIndexList _mi = m_LogTreeView->selectionModel()->selectedRows(column);
    if (_mi.count()<1) {
        return QModelIndex();
    }
    return m_SortModel->mapToSource(_mi[0]);
}

void SvnLogDlgImp::slotCustomContextMenu(const QPoint&e)
{
    QModelIndex ind=m_LogTreeView->indexAt(e);
    QModelIndex bel;
    if (ind.isValid()) {
        bel = m_LogTreeView->indexBelow(ind);
        ind = m_SortModel->mapToSource(ind);
    }
    int row = -1;
    if (ind.isValid()) {
        row = ind.row();
    } else {
        return;
    }
    
    QLONG rev = -1;
    if (bel.isValid()) {
        bel = m_SortModel->mapToSource(bel);
        rev = m_CurrentModel->toRevision(bel);
    }
    KMenu popup;
    QAction*ac;
    bool unset=false;
    if (row!=m_CurrentModel->rightRow()) {
        ac = popup.addAction(KIcon("kdesvnright"),i18n("Set version as right side of diff"));
        ac->setData(101);
    } else {
        unset = true;
    }
    if (row!=m_CurrentModel->leftRow()) {
        ac = popup.addAction(KIcon("kdesvnleft"),i18n("Set version as left side of diff"));
        ac->setData(102);
    } else {
        unset = true;
    }
    if (unset) {
        ac = popup.addAction(i18n("Unset version for diff"));
        ac->setData(103);
    }
    if (rev > -1 && !m_PegUrl.isUrl()) {
        ac = popup.addAction(i18n("Revert this commit"));
        ac->setData(104);
    }
    ac = popup.exec(m_LogTreeView->viewport()->mapToGlobal(e));
    if (!ac) {
        return;
    }
    int r = ac->data().toInt();
    switch (r) {
    case 101:
        m_CurrentModel->setRightRow(row);
        break;
    case 102:
        m_CurrentModel->setLeftRow(row);
        break;
    case 103:
        if (row!=m_CurrentModel->leftRow()) {
            m_CurrentModel->setLeftRow(-1);
        }
        if (row!=m_CurrentModel->rightRow()) {
            m_CurrentModel->setRightRow(-1);
        }
        break;
    case 104:
        {
            svn::Revision previous(rev);
            svn::Revision current(m_CurrentModel->toRevision(ind));
            QString _path = m_PegUrl;
            m_Actions->slotMergeWcRevisions(_path,current,previous,true,true,false,false);
        }
        break;
    }
    m_DispSpecDiff->setEnabled(m_CurrentModel->leftRow()!=-1 && m_CurrentModel->rightRow()!=-1 && m_CurrentModel->leftRow()!=m_CurrentModel->rightRow());
}

void SvnLogDlgImp::slotChangedPathContextMenu(const QPoint&e)
{
    QTreeWidgetItem*_item = m_ChangedList->currentItem();
    if (!_item)
    {
        return;
    }

    LogChangePathItem* item = static_cast<LogChangePathItem*>(_item);
    if (item->action()=='D') {
        return;
    }
    QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        return;
    }
    QLONG rev = m_CurrentModel->toRevision(ind);
    KMenu popup;
    QString name = item->path();
    QString action = item->action();
    QString source =item->revision()>-1?item->source():item->path();
    svn_revnum_t prev = item->revision()>0?item->revision():rev-1;
    QAction*ac;
    ac = popup.addAction(i18n("Annotate"));
    if (ac) {
        ac->setData(101);
    }
    if (action != "A" || item->revision()>-1) {
        ac=popup.addAction(i18n("Diff previous"));
        if (ac) {
            ac->setData(102);
        }
    }
    ac=popup.addAction(i18n("Cat this version"));
    if (ac) {
        ac->setData(103);
    }
    ac = popup.exec(m_ChangedList->viewport()->mapToGlobal(e));
    if (!ac) {
        return;
    }
    int r = ac->data().toInt();
    svn::Revision start(svn::Revision::START);
    switch (r)
    {
        case 101:
        {
            m_Actions->makeBlame(start,rev,_base+name,kapp->activeModalWidget(),rev,this);
            break;
        }
        case 102:
        {
            emit makeDiff(_base+source,prev,_base+name,rev,this);
            break;
        }
        case 103:
        {
            emit makeCat(rev,_base+source,source,rev,kapp->activeModalWidget());
        }
        default:
            break;
    }
}

void SvnLogDlgImp::slotSingleDoubleClicked(QTreeWidgetItem*_item,int)
{
    if (!_item)
    {
        return;
    }

    LogChangePathItem* item = static_cast<LogChangePathItem*>(_item);
    QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        return;
    }
    QLONG rev = m_CurrentModel->toRevision(ind);

    QString name = item->path();
    QString action = item->action();
    QString source =item->revision()>-1?item->source():item->path();
    svn::Revision start(svn::Revision::START);
    if (action != "D") {
        m_Actions->makeBlame(start,rev,_base+name,kapp->activeModalWidget(),rev,this);
    }
}

#include "svnlogdlgimp.moc"
