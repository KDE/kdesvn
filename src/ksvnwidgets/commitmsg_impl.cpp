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
#include "commitmsg_impl.h"
#include "models/commitmodelhelper.h"
#include "models/commitmodel.h"
#include "settings/kdesvnsettings.h"
#include "depthselector.h"
#include "ksvnwidgets/ksvndialog.h"

#include <KConfig>
#include <KFile>
#include <KIO/FileCopyJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>
#include <KUrlRequesterDialog>

#include <QStringList>
#include <QTemporaryFile>
#include <QFile>

#define MAX_MESSAGE_HISTORY 10

QStringList Commitmsg_impl::sLogHistory = QStringList();
QString Commitmsg_impl::sLastMessage;

int Commitmsg_impl::smax_message_history = 0xFFFF;

Commitmsg_impl::Commitmsg_impl(QWidget *parent)
    : QWidget(parent), CommitMessage()
{
    setupUi(this);
    m_CurrentModel = nullptr;
    m_SortModel = nullptr;
    m_LogEdit->setFocus();
    m_Reviewlabel->hide();
    m_hidden = true;
    hideButtons(true);
    m_MainSplitter->insertWidget(0, m_EditFrame);
    delete m_ReviewFrame;
    m_Reviewlabel = nullptr;
    m_MarkUnversioned = nullptr;
    m_UnmarkUnversioned = nullptr;
    m_DiffItem = nullptr;
}

Commitmsg_impl::Commitmsg_impl(const svn::CommitItemList &_items, QWidget *parent)
    : QWidget(parent), CommitMessage()
{
    setupUi(this);
    m_CurrentModel = nullptr;
    m_SortModel = nullptr;
    m_LogEdit->setFocus();
    hideButtons(true);
    if (!_items.isEmpty()) {
        m_CurrentModel = new CommitModel(_items);
        setupModel();
        m_hidden = false;
    } else {
        m_Reviewlabel->hide();
        m_CommitItemTree->hide();
        m_hidden = true;
    }
    checkSplitterSize();
}

Commitmsg_impl::Commitmsg_impl(const CommitActionEntries &_activatedList,
                               const CommitActionEntries &_notActivatedList,
                               QWidget *parent)
    : QWidget(parent), CommitMessage()
{
    setupUi(this);
    m_CurrentModel = nullptr;
    m_SortModel = nullptr;
    m_LogEdit->setFocus();
    m_hidden = false;

    m_CurrentModel = new CommitModelCheckitem(_activatedList, _notActivatedList);
    setupModel();

    m_HideNewItems->setChecked(Kdesvnsettings::commit_hide_new());
    checkSplitterSize();
}

Commitmsg_impl::~Commitmsg_impl()
{
    QList<int> list = m_MainSplitter->sizes();
    if (!m_hidden && list.count() == 2) {
        Kdesvnsettings::setCommit_splitter_height(list);
        Kdesvnsettings::self()->save();
    }
    delete m_CurrentModel;
    delete m_SortModel;
}

void Commitmsg_impl::setupModel()
{
    m_SortModel = new CommitFilterModel(m_CommitItemTree);
    m_CommitItemTree->setModel(m_SortModel);
    m_SortModel->setSourceModel(m_CurrentModel);

    m_CommitItemTree->resizeColumnToContents(m_CurrentModel->ItemColumn());
    m_CommitItemTree->resizeColumnToContents(m_CurrentModel->ActionColumn());

    m_SortModel->setSortCaseSensitivity(Kdesvnsettings::case_sensitive_sort() ? Qt::CaseSensitive : Qt::CaseInsensitive);
    connect(m_CommitItemTree->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Commitmsg_impl::slotCurrentItemChanged);
    slotCurrentItemChanged(QModelIndex()); // update pushbuttons
}

void Commitmsg_impl::checkSplitterSize()
{
    QList<int> list = Kdesvnsettings::commit_splitter_height();
    if (list.count() != 2) {
        return;
    }
    if (m_hidden) {
        list[1] = list[0] + list[1];
        list[0] = 0;
    }
    if (m_hidden || (list[0] > 0 || list[1] > 0)) {
        m_MainSplitter->setSizes(list);
    }
}

void Commitmsg_impl::slotHistoryActivated(int number)
{
    if (number < 1 || number > sLogHistory.size()) {
        m_LogEdit->clear();
    } else {
        m_LogEdit->setText(sLogHistory[number - 1]);
    }
}

/*!
    \fn Commitmsg_impl::getMessage()const
 */
QString Commitmsg_impl::getMessage()const
{
    return m_LogEdit->toPlainText();
}

/*!
    \fn Commitmsg_impl::isRecursive()const
 */
svn::Depth Commitmsg_impl::getDepth()const
{
    return m_DepthSelector->getDepth();
}

void Commitmsg_impl::keepsLocks(bool keeps_lock)
{
    if (keeps_lock) {
        m_keepLocksButton->show();
    } else {
        m_keepLocksButton->hide();
    }
}

/*!
    \fn Commitmsg_impl::isRecursive()const
 */
bool Commitmsg_impl::isKeeplocks()const
{
    return m_keepLocksButton->isChecked();
}

/*!
    \fn Commitmsg_impl::initHistory()
 */
void Commitmsg_impl::initHistory()
{
    if (smax_message_history == 0xFFFF) {
        smax_message_history = Kdesvnsettings::max_log_messages();
        KConfigGroup cs(Kdesvnsettings::self()->config(), "log_messages");
        QString s;
        int current = 0;
        QString key = QStringLiteral("log_%0").arg(current);
        s = cs.readEntry(key, QString());
        while (!s.isNull()) {
            if (current < smax_message_history) {
                sLogHistory.push_back(s);
            } else {
                cs.deleteEntry(key);
            }
            ++current;
            key = QStringLiteral("log_%0").arg(current);
            s = cs.readEntry(key, QString());
        }
    }
    QStringList::const_iterator it;
    for (const QString &historyEntry : qAsConst(sLogHistory)) {
        if (historyEntry.length() <= 40) {
            m_LogHistory->addItem(historyEntry);
        } else {
            m_LogHistory->addItem(historyEntry.leftRef(37) + QStringLiteral("..."));
        }
    }
    if (!sLastMessage.isEmpty()) {
        m_LogEdit->setText(sLastMessage);
        sLastMessage.clear();
    }
}

/*!
    \fn Commitmsg_impl::saveHistory()
 */
void Commitmsg_impl::saveHistory(bool canceld)
{
    QString _text = m_LogEdit->toPlainText();
    if (_text.isEmpty() || _text.length() > 512) {
        return;
    }
    /// @todo make static threadsafe
    if (!canceld) {
        int it;
        if ((it = sLogHistory.indexOf(_text)) != -1) {
            sLogHistory.removeAt(it);
        }
        sLogHistory.push_front(_text);
        if (sLogHistory.size() > smax_message_history) {
            sLogHistory.removeLast();
        }
        KConfigGroup cs(Kdesvnsettings::self()->config(), "log_messages");
        for (int i = 0; i < sLogHistory.size(); ++i) {
            cs.writeEntry(QStringLiteral("log_%0").arg(i), sLogHistory[i]);
        }
        cs.sync();
    } else {
        sLastMessage = _text;
    }
}

QString Commitmsg_impl::getLogmessage(bool *ok, svn::Depth *rec, bool *keep_locks, QWidget *parent)
{
    return getLogmessageInternal(new Commitmsg_impl, ok, rec, keep_locks, nullptr, parent);
}

QString Commitmsg_impl::getLogmessage(const svn::CommitItemList &items, bool *ok, svn::Depth *rec, bool *keep_locks, QWidget *parent)
{
    return getLogmessageInternal(new Commitmsg_impl(items), ok, rec, keep_locks, nullptr, parent);
}

QString Commitmsg_impl::getLogmessage(const CommitActionEntries &_on,
                                      const CommitActionEntries &_off,
                                      QObject *callback,
                                      CommitActionEntries &_result,
                                      bool *ok, bool *keep_locks, QWidget *parent)
{
    Commitmsg_impl *ptr = new Commitmsg_impl(_on, _off);
    if (callback) {
        connect(ptr, SIGNAL(makeDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)),
                callback, SLOT(makeDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)));
        connect(ptr, SIGNAL(sigRevertItem(QStringList)),
                callback, SLOT(slotRevertItems(QStringList)));
        connect(callback, SIGNAL(sigItemsReverted(QStringList)),
                ptr, SLOT(slotItemReverted(QStringList)));
    }
    return getLogmessageInternal(ptr, ok, nullptr, keep_locks, &_result, parent);
}

QString Commitmsg_impl::getLogmessageInternal(Commitmsg_impl *ptr, bool *ok, svn::Depth *rec, bool *keep_locks, CommitActionEntries *result, QWidget *parent)
{
    bool _ok, _keep_locks;
    svn::Depth _depth = svn::DepthUnknown;
    QString msg;

    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("logmsg_dlg_size"), parent));
    dlg->setWindowTitle(i18nc("@title:window", "Commit Log"));
    dlg->setWithCancelButton();
    dlg->addWidget(ptr);

    if (!rec) {
        ptr->m_DepthSelector->hide();
    }
    if (!keep_locks) {
        ptr->m_keepLocksButton->hide();
    }

    ptr->initHistory();
    if (dlg->exec() != QDialog::Accepted) {
        _ok = false;
        /* avoid compiler warnings */
        _keep_locks = false;
    } else {
        _ok = true;
        _depth = ptr->getDepth();
        _keep_locks = ptr->isKeeplocks();
        msg = ptr->getMessage();
    }
    if (dlg) {
        ptr->saveHistory(!_ok);
    }

    if (ok) {
        *ok = _ok;
    }
    if (rec) {
        *rec = _depth;
    }
    if (keep_locks) {
        *keep_locks = _keep_locks;
    }
    if (result) {
        *result = ptr->checkedEntries();
    }
    delete dlg;
    return msg;
}

/*!
    \fn Commitmsg_impl::setRecCheckboxtext(const QString&what)
 */
void Commitmsg_impl::addItemWidget(QWidget *aWidget)
{
    m_DepthSelector->addItemWidget(aWidget);
}

CommitActionEntries Commitmsg_impl::checkedEntries() const
{
    if (m_CurrentModel) {
        return m_CurrentModel->checkedEntries();
    }
    return CommitActionEntries();
}

void Commitmsg_impl::slotUnmarkUnversioned()
{
    markUnversioned(false);
}

void Commitmsg_impl::slotMarkUnversioned()
{
    markUnversioned(true);
}

void Commitmsg_impl::slotDiffSelected()
{
    CommitModelNodePtr ptr = currentCommitItem();
    if (!ptr) {
        return;
    }
    QString what = ptr->actionEntry().name();
    emit makeDiff(what, svn::Revision::BASE, what, svn::Revision::WORKING, parentWidget());
}

void Commitmsg_impl::slotRevertSelected()
{
    CommitModelNodePtr ptr = currentCommitItem();
    if (!ptr) {
        return;
    }
    QStringList what(ptr->actionEntry().name());
    emit sigRevertItem(what);
}

CommitModelNodePtr Commitmsg_impl::currentCommitItem(int column)
{
    CommitModelNodePtr res;
    if (!m_CurrentModel) {
        return res;
    }
    QModelIndexList _mi = m_CommitItemTree->selectionModel()->selectedRows(column);
    if (_mi.isEmpty()) {
        return res;
    }
    QModelIndex ind = m_SortModel->mapToSource(_mi[0]);
    if (ind.isValid()) {
        res = m_CurrentModel->node(ind);
    }
    return res;
}

void Commitmsg_impl::hideKeepsLock(bool how)
{
    m_keepLocksButton->setVisible(!how);
}

void Commitmsg_impl::hideButtons(bool how)
{
    if (!m_MarkUnversioned) {
        return;
    }
    if (how) {
        m_MarkUnversioned->hide();
        m_UnmarkUnversioned->hide();
        m_DiffItem->hide();
        m_HideNewItems->hide();
        m_SelectAllButton->hide();
        m_UnselectAllButton->hide();
    } else {
        m_MarkUnversioned->show();
        m_UnmarkUnversioned->show();
        m_DiffItem->show();
        m_HideNewItems->show();
        m_SelectAllButton->show();
        m_UnselectAllButton->show();
    }
}

/*!
    \fn Commitmsg_impl::markUnversioned(bool mark)
 */
void Commitmsg_impl::markUnversioned(bool mark)
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->markItems(mark, CommitActionEntry::ADD_COMMIT);
}

void Commitmsg_impl::slotSelectAll()
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->markItems(true, CommitActionEntry::ALL);
}

void Commitmsg_impl::slotUnselectAll()
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->markItems(false, CommitActionEntry::ALL);
}

void Commitmsg_impl::hideNewItems(bool hide)
{
    if (!m_CurrentModel) {
        return;
    }
    Kdesvnsettings::setCommit_hide_new(hide);
    m_SortModel->hideItems(hide, CommitActionEntry::ADD_COMMIT);
    m_HideNewItems->setText(hide ? i18n("Show new items") : i18n("Hide new items"));
}

/*!
    \fn Commitmsg_impl::hideDepth(bool hide)
 */
void Commitmsg_impl::hideDepth(bool ahide)
{
    m_DepthSelector->hideDepth(ahide);
}

void Commitmsg_impl::insertFile(const QString &fname)
{
    QFile ifs(fname);
    if (ifs.open(QIODevice::ReadOnly)) {
        QTextStream ts(&ifs);
        QString _content = ts.readAll();
        m_LogEdit->textCursor().insertText(_content);
    }
}

void Commitmsg_impl::insertFile()
{
    QString windowTitle = i18nc("@title:window", "Select Text File to Insert");
    QPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(QUrl(), i18n("Select text file to insert:"), this));
    dlg->setWindowTitle(windowTitle);
    KFile::Mode mode = static_cast<KFile::Mode>(KFile::File);
    dlg->urlRequester()->setMode(mode);
    dlg->urlRequester()->setWindowTitle(windowTitle);

    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    QUrl _url = dlg->selectedUrl();
    delete dlg;
    if (_url.isEmpty() || !_url.isValid()) {
        return;
    }
    if (_url.isLocalFile()) {
        insertFile(_url.path());
    } else {
        QTemporaryFile tf;
        tf.open();
        KIO::FileCopyJob *job = KIO::file_copy(_url, QUrl::fromLocalFile(tf.fileName()));
        KJobWidgets::setWindow(job, this);
        if (job->exec()) {
            insertFile(tf.fileName());
        } else {
            KMessageBox::error(this, job->errorString());
        }
    }
}

/*!
    \fn Commitmsg_impl::slotItemReverted(const QStringList&)
 */
void Commitmsg_impl::slotItemReverted(const QStringList &items)
{
    if (!m_CurrentModel) {
        return;
    }
    m_CurrentModel->removeEntries(items);
}

void Commitmsg_impl::slotItemDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    slotDiffSelected();
}

void Commitmsg_impl::slotCurrentItemChanged(const QModelIndex &current)
{
    bool bDiffRevertEnabled = false;

    const CommitModelNodePtr node = m_CurrentModel->dataForRow(m_SortModel->mapToSource(current).row());
    if (!node.isNull()) {
        bDiffRevertEnabled = (node->actionEntry().type() == CommitActionEntry::COMMIT);
    }
    m_RevertItemButton->setEnabled(bDiffRevertEnabled);
    m_DiffItem->setEnabled(bDiffRevertEnabled);
}
