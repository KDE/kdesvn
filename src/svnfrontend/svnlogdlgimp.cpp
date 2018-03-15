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
#include "settings/kdesvnsettings.h"
#include "svnactions.h"
#include "svnfrontend/fronthelpers/revisionbuttonimpl.h"
#include "svnfrontend/models/logitemmodel.h"
#include "svnfrontend/models/logmodelhelper.h"
#include "helpers/windowgeometryhelper.h"

#include <kconfig.h>

#include <KHelpClient>
#include <KStandardGuiItem>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTextDocumentFragment>

const QLatin1String groupName("log_dialog_size");

SvnLogDlgImp::SvnLogDlgImp(SvnActions *ac, bool modal, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setModal(modal);
    m_pbClose->setDefault(true);
    m_pbClose->setShortcut(Qt::CTRL | Qt::Key_Return);
    KStandardGuiItem::assign(m_pbClose, KStandardGuiItem::Close);
    KStandardGuiItem::assign(m_pbHelp, KStandardGuiItem::Help);
    m_DispPrevButton->setIcon(QIcon::fromTheme(QStringLiteral("kdesvndiff")));
    m_DispSpecDiff->setIcon(QIcon::fromTheme(QStringLiteral("kdesvndiff")));
    buttonBlame->setIcon(QIcon::fromTheme(QStringLiteral("kdesvnblame")));
    m_SortModel = nullptr;
    m_CurrentModel = nullptr;
    m_ControlKeyDown = false;

    if (Kdesvnsettings::self()->log_always_list_changed_files()) {
        buttonListFiles->hide();
    } else {
        m_ChangedList->hide();
    }
    m_Actions = ac;
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    QByteArray t1 = cs.readEntry("logsplitter", QByteArray());
    if (!t1.isEmpty()) {
        m_centralSplitter->restoreState(t1);
    }
    t1 = cs.readEntry("right_logsplitter", QByteArray());
    if (!t1.isEmpty()) {
        if (cs.readEntry("laststate", false) == m_ChangedList->isHidden()) {
            m_rightSplitter->restoreState(t1);
        }
    }
}

SvnLogDlgImp::~SvnLogDlgImp()
{
    KConfigGroup cs(Kdesvnsettings::self()->config(), groupName);
    cs.writeEntry("right_logsplitter", m_rightSplitter->saveState());
    cs.writeEntry("logsplitter", m_centralSplitter->saveState());
    cs.writeEntry("laststate", m_ChangedList->isHidden());
    delete m_SortModel;
}

void SvnLogDlgImp::dispLog(const svn::LogEntriesMapPtr &log, const QString &what, const QString &root, const svn::Revision &peg, const QString &pegUrl)
{
    m_peg = peg;
    m_PegUrl = pegUrl;
    m_startRevButton->setNoWorking(m_PegUrl.isUrl());
    m_endRevButton->setNoWorking(m_PegUrl.isUrl());
    if (!m_PegUrl.isUrl() || Kdesvnsettings::remote_special_properties()) {
        QString s = m_Actions->searchProperty(_bugurl, QStringLiteral("bugtraq:url"), pegUrl, peg, true);
        if (!s.isEmpty()) {
            QString reg;
            s = m_Actions->searchProperty(reg, QStringLiteral("bugtraq:logregex"), pegUrl, peg, true);
            if (!s.isNull() && !reg.isEmpty()) {
                const QVector<QStringRef> s1 = reg.splitRef(QLatin1Char('\n'));
                if (!s1.isEmpty()) {
                    _r1.setPattern(s1.at(0).toString());
                    if (s1.size() > 1) {
                        _r2.setPattern(s1.at(1).toString());
                    }
                }
            }
        }
    }
    _base = root;
    m_Entries = log;
    if (!what.isEmpty()) {
        setWindowTitle(i18nc("@title:window", "SVN Log of %1", what));
    } else {
        setWindowTitle(i18nc("@title:window", "SVN Log"));
    }
    _name = what;
    if (!_name.startsWith(QLatin1Char('/'))) {
        _name = QLatin1Char('/') + _name;
    }
    dispLog(log);
}

void SvnLogDlgImp::dispLog(const svn::LogEntriesMapPtr &_log)
{
    if (!_log) {
        return;
    }
    bool must_init = false;
    if (!m_SortModel) {
        m_SortModel = new SvnLogSortModel(m_LogTreeView);
        m_CurrentModel = new SvnLogModel(_log, _name, m_SortModel);
        m_SortModel->setSourceModel(m_CurrentModel);
        must_init = true;
    } else {
        m_CurrentModel->setLogData(_log, _name);
    }

    if (must_init) {
        m_LogTreeView->setModel(m_SortModel);
        m_LogTreeView->sortByColumn(SvnLogModel::Revision, Qt::DescendingOrder);
        connect(m_LogTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)));
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Revision);
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Author);
        m_LogTreeView->resizeColumnToContents(SvnLogModel::Date);
    }
    m_startRevButton->setRevision(m_CurrentModel->max());
    m_endRevButton->setRevision(m_CurrentModel->min());
    QModelIndex ind = m_CurrentModel->index(m_CurrentModel->rowCount(QModelIndex()) - 1);
    if (ind.isValid()) {
        m_LogTreeView->selectionModel()->select(m_SortModel->mapFromSource(ind),
                                                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    m_LogTreeView->setFocus();
}

QString SvnLogDlgImp::genReplace(const QString &r1match)
{
    static QString anf(QStringLiteral("<a href=\""));
    static QString mid(QStringLiteral("\">"));
    static QString end(QStringLiteral("</a>"));
    QString res;
    if (_r2.pattern().length() < 1) {
        res = _bugurl;
        res.replace(QStringLiteral("%BUGID%"), _r1.cap(1));
        res = anf + res + mid + r1match + end;
        return res;
    }
    int pos = 0;
    int count = 0;
    int oldpos;

    while (pos > -1) {
        oldpos = pos + count;
        pos = r1match.indexOf(_r2, pos + count);
        if (pos == -1) {
            break;
        }
        count = _r2.matchedLength();
        res += r1match.midRef(oldpos, pos - oldpos);
        QString sub = r1match.mid(pos, count);
        QString _url = _bugurl;
        _url.replace(QStringLiteral("%BUGID%"), sub);
        res += anf + _url + mid + sub + end;
    }
    res += r1match.midRef(oldpos);
    return res;
}

void SvnLogDlgImp::replaceBugids(QString &msg)
{
    if (!_r1.isValid() || _r1.pattern().length() < 1 || _bugurl.isEmpty()) {
        return;
    }
    int pos = 0;
    int count = 0;

    pos = _r1.indexIn(msg, pos + count);
    count = _r1.matchedLength();

    while (pos > -1) {
        QString s1 = msg.mid(pos, count);
        QString rep = genReplace(s1);
        msg = msg.replace(pos, count, rep);
        pos = _r1.indexIn(msg, pos + rep.length());
        count = _r1.matchedLength();
    }
}

void SvnLogDlgImp::slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous)
{
    Q_UNUSED(previous);
    m_ChangedList->clear();
    QModelIndexList _l = current.indexes();
    if (_l.count() < 1) {
        m_DispPrevButton->setEnabled(false);
        buttonListFiles->setEnabled(false);
        buttonBlame->setEnabled(false);
        m_ChangedList->clear();
        return;
    }
    QModelIndex _index = m_SortModel->mapToSource(_l[0]);
    m_CurrentModel->fillChangedPaths(_index, m_ChangedList);
    QTextDocumentFragment _m = QTextDocumentFragment::fromPlainText(m_CurrentModel->fullMessage(_index));
    QString msg = _m.toHtml();
    replaceBugids(msg);
    m_LogDisplay->setHtml(msg);
    m_DispPrevButton->setEnabled(_index.row() > 0);
    buttonBlame->setEnabled(true);
}


/*!
    \fn SvnLogDlgImp::slotDispPrevious()
 */
void SvnLogDlgImp::slotDispPrevious()
{
    QModelIndex _index = selectedRow();
    if (!_index.isValid() || _index.row() == 0) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    QModelIndex _it = m_CurrentModel->index(_index.row() - 1);
    if (!_it.isValid()) {
        m_DispPrevButton->setEnabled(false);
        return;
    }
    const SvnLogModelNodePtr k = m_CurrentModel->indexNode(_index);
    const SvnLogModelNodePtr p = m_CurrentModel->indexNode(_it);
    if (!k || !p) {
        m_DispPrevButton->setEnabled(false);
        return;
    }

    const QString s(_base + k->realName());
    const QString e(_base + p->realName());
    emit makeDiff(e, p->revision(), s, k->revision(), this);
}


/*!
    \fn SvnLogDlgImp::saveSize()
 */
void SvnLogDlgImp::saveSize()
{
    WindowGeometryHelper::save(this, groupName);
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
        emit makeDiff(_base + m_first->realName(), m_first->revision(), _base + m_second->realName(), m_second->revision(), this);
    }
}

bool SvnLogDlgImp::getSingleLog(svn::LogEntry &t, const svn::Revision &r, const QString &what, const svn::Revision &peg, QString &root)
{
    root = _base;
    const svn::LogEntriesMap::const_iterator it = m_Entries->constFind(r.revnum());
    if (it == m_Entries->constEnd()) {
        return m_Actions->getSingleLog(t, r, what, peg, root);
    }
    t = it.value();
    return true;
}

void SvnLogDlgImp::slotGetLogs()
{
    svn::LogEntriesMapPtr lm = m_Actions->getLog(m_startRevButton->revision(),
                                                 m_endRevButton->revision(), m_peg,
                                                 _base + _name, Kdesvnsettings::self()->log_always_list_changed_files(), 0, Kdesvnsettings::last_node_follow(), this);
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
    svn::Revision begin = now.revnum() - 1;
    if (begin.revnum() < 1) {
        begin = 1;
    }
    svn::LogEntriesMapPtr lm = m_Actions->getLog(begin,
                                                 (begin.revnum() > 50 ? svn::Revision::START : svn::Revision::HEAD), m_peg,
                                                 _base + _name, Kdesvnsettings::self()->log_always_list_changed_files(), 50, Kdesvnsettings::last_node_follow(), this);
    if (lm) {
        dispLog(lm);
    }
}

void SvnLogDlgImp::slotBeginHead()
{
    svn::LogEntriesMapPtr lm = m_Actions->getLog(svn::Revision::HEAD,
                                                 1, m_peg,
                                                 _base + _name, Kdesvnsettings::self()->log_always_list_changed_files(), 50, Kdesvnsettings::last_node_follow(), this);
    if (lm) {
        dispLog(lm);
    }
}

void SvnLogDlgImp::slotHelpRequested()
{
    KHelpClient::invokeHelp(QLatin1String("logdisplay-dlg"), QLatin1String("kdesvn"));
}


void SvnLogDlgImp::slotListEntries()
{
    QModelIndex _index = selectedRow();
    SvnLogModelNodePtr ptr = m_CurrentModel->indexNode(_index);
    if (!ptr) {
        buttonListFiles->setEnabled(false);
        return;
    }
    if (ptr->changedPaths().isEmpty()) {
        svn::LogEntriesMapPtr _log = m_Actions->getLog(ptr->revision(), ptr->revision(), ptr->revision(),
                                                       _name, true, 0, Kdesvnsettings::last_node_follow());
        if (!_log) {
            return;
        }
        if (!_log->isEmpty()) {
            ptr->setChangedPaths(_log->value(ptr->revision()));
        }
    }
    if (ptr->changedPaths().isEmpty()) {
        m_CurrentModel->fillChangedPaths(_index, m_ChangedList);
    }
    buttonListFiles->setEnabled(false);
}

void SvnLogDlgImp::keyPressEvent(QKeyEvent *e)
{
    if (!e) {
        return;
    }
    if (e->text().isEmpty() && e->key() == Qt::Key_Control) {
        m_ControlKeyDown = true;
    }
    QDialog::keyPressEvent(e);
}

void SvnLogDlgImp::keyReleaseEvent(QKeyEvent *e)
{
    if (!e) {
        return;
    }
    if (e->text().isEmpty() && e->key() == Qt::Key_Control) {
        m_ControlKeyDown = false;
    }
    QDialog::keyReleaseEvent(e);
}

void SvnLogDlgImp::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    WindowGeometryHelper::restore(this, groupName);
}


void SvnLogDlgImp::slotBlameItem()
{
    QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        buttonBlame->setEnabled(false);
        return;
    }
    qlonglong rev = m_CurrentModel->toRevision(ind);
    svn::Revision start(svn::Revision::START);
    m_Actions->makeBlame(start, rev, _base + m_CurrentModel->realName(ind), QApplication::activeModalWidget(), rev, this);
}

/* it works 'cause we use single selection only */
QModelIndex SvnLogDlgImp::selectedRow(int column)
{
    QModelIndexList _mi = m_LogTreeView->selectionModel()->selectedRows(column);
    if (_mi.count() < 1) {
        return QModelIndex();
    }
    return m_SortModel->mapToSource(_mi[0]);
}

void SvnLogDlgImp::slotCustomContextMenu(const QPoint &e)
{
    QModelIndex ind = m_LogTreeView->indexAt(e);
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

    qlonglong rev = -1;
    if (bel.isValid()) {
        bel = m_SortModel->mapToSource(bel);
        rev = m_CurrentModel->toRevision(bel);
    }
    QMenu popup;
    QAction *ac;
    bool unset = false;
    if (row != m_CurrentModel->rightRow()) {
        ac = popup.addAction(QIcon::fromTheme(QStringLiteral("kdesvnright")), i18n("Set version as right side of diff"));
        ac->setData(101);
    } else {
        unset = true;
    }
    if (row != m_CurrentModel->leftRow()) {
        ac = popup.addAction(QIcon::fromTheme(QStringLiteral("kdesvnleft")), i18n("Set version as left side of diff"));
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
        if (row != m_CurrentModel->leftRow()) {
            m_CurrentModel->setLeftRow(-1);
        }
        if (row != m_CurrentModel->rightRow()) {
            m_CurrentModel->setRightRow(-1);
        }
        break;
    case 104: {
        svn::Revision previous(rev);
        svn::Revision current(m_CurrentModel->toRevision(ind));
        QString _path = m_PegUrl.path();
        m_Actions->slotMergeWcRevisions(_path, current, previous, true, true, false, false, false);
    }
    break;
    }
    m_DispSpecDiff->setEnabled(m_CurrentModel->leftRow() != -1 && m_CurrentModel->rightRow() != -1 && m_CurrentModel->leftRow() != m_CurrentModel->rightRow());
}

void SvnLogDlgImp::slotChangedPathContextMenu(const QPoint &e)
{
    QTreeWidgetItem *_item = m_ChangedList->currentItem();
    if (!_item) {
        return;
    }

    LogChangePathItem *item = static_cast<LogChangePathItem *>(_item);
    if (item->action() == 'D') {
        return;
    }
    QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        return;
    }
    const qlonglong rev = m_CurrentModel->toRevision(ind);
    QMenu popup;
    const QString name = item->path();
    const QString source = item->revision() > -1 ? item->source() : item->path();
    QAction *ac;
    ac = popup.addAction(i18n("Annotate"));
    if (ac) {
        ac->setData(101);
    }
    if (item->action() != 'A' || item->revision() > -1) {
        ac = popup.addAction(i18n("Diff previous"));
        if (ac) {
            ac->setData(102);
        }
    }
    ac = popup.addAction(i18n("Cat this version"));
    if (ac) {
        ac->setData(103);
    }
    ac = popup.exec(m_ChangedList->viewport()->mapToGlobal(e));
    if (!ac) {
        return;
    }
    int r = ac->data().toInt();
    svn::Revision start(svn::Revision::START);
    switch (r) {
    case 101: {
        m_Actions->makeBlame(start, rev, _base + name, QApplication::activeModalWidget(), rev, this);
        break;
    }
    case 102: {
        const svn_revnum_t prev = item->revision() > 0 ? item->revision() : rev - 1;
        emit makeDiff(_base + source, prev, _base + name, rev, this);
        break;
    }
    case 103: {
        emit makeCat(rev, _base + source, source, rev, QApplication::activeModalWidget());
    }
    default:
        break;
    }
}

void SvnLogDlgImp::slotSingleDoubleClicked(QTreeWidgetItem *_item, int)
{
    if (!_item) {
        return;
    }

    const LogChangePathItem *item = static_cast<LogChangePathItem *>(_item);
    const QModelIndex ind = selectedRow();
    if (!ind.isValid()) {
        return;
    }
    svn::Revision start(svn::Revision::START);
    if (item->action() != 'D') {
        const QString name = item->path();
        const qlonglong rev = m_CurrentModel->toRevision(ind);
        m_Actions->makeBlame(start, rev, _base + name, QApplication::activeModalWidget(), rev, this);
    }
}
