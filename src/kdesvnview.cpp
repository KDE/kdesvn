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

#include "kdesvnview.h"
#include "svnfrontend/maintreewidget.h"
#include "svnfrontend/createrepodlg.h"
#include "svnfrontend/dumprepo_impl.h"
#include "svnfrontend/hotcopydlg_impl.h"
#include "svnfrontend/loaddmpdlg_impl.h"
#include "svnfrontend/stopdlg.h"
#include "svnfrontend/fronthelpers/propertylist.h"
#include "settings/kdesvnsettings.h"
#include "svnqt/url.h"
#include "svnqt/repository.h"
#include "svnqt/version_check.h"
#include "svnqt/svnqttypes.h"

#include <QFileInfo>
#include <QMenu>
#include <QProgressBar>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTextBrowser>

#include <KActionCollection>
#include <KIO/FileCopyJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

kdesvnView::kdesvnView(KActionCollection *aCollection, QWidget *parent, bool full)
    : QWidget(parent), svn::repository::RepositoryListener()
    , m_Collection(aCollection)
    , m_currentUrl()
    , m_CacheProgressBar(nullptr)
    , m_ReposCancel(false)
{
    Q_UNUSED(full);
    setFocusPolicy(Qt::StrongFocus);
    setupActions();

    m_topLayout = new QVBoxLayout(this);

    m_Splitter = new QSplitter(this);
    m_Splitter->setOrientation(Qt::Vertical);

    //m_TreeWidget=new kdesvnfilelist(m_Collection,m_Splitter);
    m_TreeWidget = new MainTreeWidget(m_Collection, m_Splitter);

    m_infoSplitter = new QSplitter(m_Splitter);
    m_infoSplitter->setOrientation(Qt::Horizontal);
    m_infoSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_LogWindow = new QTextBrowser(m_infoSplitter);
    m_LogWindow->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_LogWindow, &QWidget::customContextMenuRequested,
            this, &kdesvnView::onCustomLogWindowContextMenuRequested);
    Propertylist *pl = new Propertylist(m_infoSplitter);
    pl->setCommitchanges(true);
    connect(pl, &Propertylist::sigSetProperty,
            m_TreeWidget, &MainTreeWidget::slotChangeProperties);
    connect(m_TreeWidget, &MainTreeWidget::sigProplist,
            pl, &Propertylist::displayList);
    connect(m_TreeWidget, &MainTreeWidget::sigProplist,
            pl, &Propertylist::displayList);

    m_TreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_topLayout->addWidget(m_Splitter);
    connect(m_TreeWidget, &MainTreeWidget::sigLogMessage, this, &kdesvnView::slotAppendLog);
    connect(m_TreeWidget, &MainTreeWidget::changeCaption, this, &kdesvnView::slotSetTitle);
    connect(m_TreeWidget, &MainTreeWidget::sigShowPopup, this, &kdesvnView::slotDispPopup);
    connect(m_TreeWidget, &MainTreeWidget::sigUrlOpened, this, &kdesvnView::sigUrlOpened);
    connect(m_TreeWidget, &MainTreeWidget::sigSwitchUrl, this, &kdesvnView::sigSwitchUrl);
    connect(m_TreeWidget, &MainTreeWidget::sigUrlChanged, this, &kdesvnView::slotUrlChanged);
    connect(m_TreeWidget, &MainTreeWidget::sigCacheStatus, this, &kdesvnView::fillCacheStatus);
    connect(m_TreeWidget, &MainTreeWidget::sigExtraStatusMessage, this, &kdesvnView::sigExtraStatusMessage);

    connect(this, &kdesvnView::sigMakeBaseDirs, m_TreeWidget, &MainTreeWidget::slotMkBaseDirs);

    KConfigGroup cs(Kdesvnsettings::self()->config(), "kdesvn-mainlayout");
    QByteArray t1 = cs.readEntry("split1", QByteArray());
    if (!t1.isEmpty()) {
        m_Splitter->restoreState(t1);
    }
    if (m_infoSplitter) {
        t1 = cs.readEntry("infosplit", QByteArray());
        if (!t1.isEmpty()) {
            m_infoSplitter->restoreState(t1);
        }
    }
}

void kdesvnView::slotAppendLog(const QString &text)
{
    m_LogWindow->append(text);
}

kdesvnView::~kdesvnView()
{
}

void kdesvnView::slotSavestate()
{
    KConfigGroup cs(Kdesvnsettings::self()->config(), "kdesvn-mainlayout");
    cs.writeEntry("split1", m_Splitter->saveState());
    if (m_infoSplitter) {
        cs.writeEntry("infosplit", m_infoSplitter->saveState());
    }
}

void kdesvnView::slotUrlChanged(const QUrl &url)
{
    m_currentUrl = url;
    slotSetTitle(url.toString());
    emit sigUrlChanged(url);
    slotOnURL(i18n("Repository opened"));
}

QUrl kdesvnView::currentUrl() const
{
    return m_currentUrl;
}

bool kdesvnView::openUrl(const QUrl &url)
{
    /* transform of url must be done in part! otherwise we will run into different troubles! */
    m_currentUrl.clear();
    QUrl _url(url);
    bool open = false;
    if (_url.isLocalFile()) {
        QString query = _url.query();
        _url.setQuery(QString());
        QString _f = _url.path();
        QFileInfo f(_f);
        if (!f.isDir()) {
            m_currentUrl.clear();
            return open;
        }
        if (query.length() > 1) {
            _url.setQuery(query);
        }
    } else {
        if (!svn::Url::isValid(url.scheme())) {
            return open;
        }
    }
    m_LogWindow->clear();
    slotSetTitle(url.toString());
    if (m_TreeWidget->openUrl(url)) {
        slotOnURL(i18n("Repository opened"));
        m_currentUrl = url;
        open = true;
    } else {
        QString t = m_TreeWidget->lastError();
        if (t.isEmpty()) {
            t = i18n("Could not open repository");
        }
        slotOnURL(t);
    }
    return open;
}

void kdesvnView::slotOnURL(const QString &url)
{
    emit signalChangeStatusbar(url);
}

void kdesvnView::slotSetTitle(const QString &title)
{
    //emit signalChangeCaption(title);
    emit setWindowCaption(title);
}

/*!
    \fn kdesvnView::closeMe()
 */
void kdesvnView::closeMe()
{
    m_TreeWidget->closeMe();
    m_LogWindow->clear();
    slotOnURL(i18n("No repository open"));
}

void kdesvnView::slotDispPopup(const QString &item, QWidget **target)
{
    emit sigShowPopup(item, target);
}

/*!
    \fn kdesvnView::refreshCurrentTree()
 */
void kdesvnView::refreshCurrentTree()
{
    m_TreeWidget->refreshCurrentTree();
}

/*!
    \fn kdesvnView::slotSettingsChanged()
 */
void kdesvnView::slotSettingsChanged()
{
    m_TreeWidget->slotSettingsChanged();
}

/*!
    \fn kdesvnView::slotCreateRepo()
 */
void kdesvnView::slotCreateRepo()
{
    QPointer<CreaterepoDlg> dlg(new CreaterepoDlg(this));
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    QScopedPointer<svn::repository::Repository> _rep(new svn::repository::Repository(this));
    bool ok = true;
    closeMe();
    try {
        _rep->CreateOpen(dlg->parameter());
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
        ok = false;
    }
    if (!ok) {
        delete dlg;
        return;
    }
    bool createdirs = dlg->createMain();
    // repo is created on a local path
    const QUrl path = QUrl::fromLocalFile(dlg->targetDir());
    delete dlg;
    openUrl(path);
    if (createdirs) {
        emit sigMakeBaseDirs();
    }
}

void kdesvnView::slotHotcopy()
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("hotcopy_repo_size"), QApplication::activeModalWidget()));
    dlg->setWindowTitle(i18nc("@title:window", "Hotcopy a Repository"));
    dlg->setWithCancelButton();

    HotcopyDlg_impl *ptr = new HotcopyDlg_impl(dlg);
    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    bool cleanlogs = ptr->cleanLogs();
    QString src = ptr->srcPath();
    QString dest = ptr->destPath();
    delete dlg;
    if (src.isEmpty() || dest.isEmpty()) {
        return;
    }
    try {
        svn::repository::Repository::hotcopy(src, dest, cleanlogs);
        slotAppendLog(i18n("Hotcopy finished."));
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
    }
}

void kdesvnView::slotLoaddump()
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("loaddump_repo_size"), this));
    dlg->setWindowTitle(i18nc("@title:window", "Load a Repository From an svndump"));
    dlg->setWithCancelButton();

    LoadDmpDlg_impl *ptr(new LoadDmpDlg_impl(dlg));
    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::repository::Repository _rep(this);
    m_ReposCancel = false;

    try {
        _rep.Open(ptr->repository());
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
        return ;
    }

    svn::repository::Repository::LOAD_UUID _act;
    switch (ptr->uuidAction()) {
    case 1:
        _act = svn::repository::Repository::UUID_IGNORE_ACTION;
        break;
    case 2:
        _act = svn::repository::Repository::UUID_FORCE_ACTION;
        break;
    case 0:
    default:
        _act = svn::repository::Repository::UUID_DEFAULT_ACTION;
        break;
    }

    const QUrl _uri = ptr->dumpFile();
    QString _input;
    QTemporaryFile tmpfile;
    if (_uri.isLocalFile()) {
        _input = _uri.toLocalFile();
    } else {
        tmpfile.open();
        KIO::FileCopyJob *job = KIO::file_copy(_uri, QUrl::fromLocalFile(tmpfile.fileName()));
        KJobWidgets::setWindow(job, this);
        if (!job->exec()) {
            KMessageBox::error(this, job->errorString());
            return;
        }
        _input = tmpfile.fileName();
    }

    try {
        StopDlg sdlg(nullptr, this, i18nc("@title:window", "Load Dump"), i18n("Loading a dump into a repository."));
        _rep.loaddump(_input, _act, ptr->parentPath(), ptr->usePre(), ptr->usePost(), ptr->validateProps());
        slotAppendLog(i18n("Loading dump finished."));
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
    }
    delete dlg;
}

void kdesvnView::slotDumpRepo()
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("dump_repo_size"), QApplication::activeModalWidget()));
    dlg->setWindowTitle(i18nc("@title:window", "Dump a Repository"));
    dlg->setWithCancelButton();

    DumpRepo_impl *ptr(new DumpRepo_impl(dlg));
    dlg->addWidget(ptr);
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }

    const QString re = ptr->reposPath();
    const QString out = ptr->targetFile();
    const bool incr = ptr->incremental();
    const bool diffs = ptr->use_deltas();
    const int s = ptr->startNumber();
    const int e = ptr->endNumber();

    delete dlg;

    m_ReposCancel = false;
    svn::Revision st = svn::Revision::UNDEFINED;
    svn::Revision en = svn::Revision::UNDEFINED;

    if (s > -1) {
        st = s;
    }
    if (e > -1) {
        en = e;
    }

    svn::repository::Repository *_rep(new svn::repository::Repository(this));
    try {
        _rep->Open(re);
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
        delete _rep;
        return;
    }

    try {
        StopDlg sdlg(nullptr, this, i18nc("@title:window", "Dump"), i18n("Dumping a repository"));
        _rep->dump(out, st, en, incr, diffs);
        slotAppendLog(i18n("Dump finished."));
    } catch (const svn::ClientException &e) {
        slotAppendLog(e.msg());
    }
    delete _rep;
}

/*!
    \fn kdesvnView::setupActions()
 */
void kdesvnView::setupActions()
{
}

void kdesvnView::sendWarning(const QString &aMsg)
{
    slotAppendLog(aMsg);
}

void kdesvnView::sendError(const QString &aMsg)
{
    slotAppendLog(aMsg);
}

bool kdesvnView::isCanceld()
{
    if (!m_ReposCancel) {
        emit tickProgress();
        return false;
    }
    return true;
}

void kdesvnView::setCanceled(bool how)
{
    m_ReposCancel = how;
}

void kdesvnView::fillCacheStatus(qlonglong current, qlonglong max)
{
    if (current > -1 && max > -1) {
        if (!m_CacheProgressBar) {
            m_CacheProgressBar = new QProgressBar(this);
            m_CacheProgressBar->setRange(0, (int)max);
            m_topLayout->addWidget(m_CacheProgressBar);
            m_CacheProgressBar->setFormat(i18n("Inserted %v not cached log entries of %m."));
        }
        if (!m_CacheProgressBar->isVisible()) {
            m_CacheProgressBar->show();
        }
        m_CacheProgressBar->setValue((int)current);
    } else {
        delete m_CacheProgressBar;
        m_CacheProgressBar = nullptr;
    }
}

void kdesvnView::stopCacheThreads()
{
    m_TreeWidget->stopLogCache();
}

void kdesvnView::onCustomLogWindowContextMenuRequested(const QPoint &pos)
{
    QPointer<QMenu> menu = m_LogWindow->createStandardContextMenu();
    QAction *clearAction = new QAction(tr("Clear"), menu.data());
    clearAction->setEnabled(!m_LogWindow->toPlainText().isEmpty());
    connect(clearAction, &QAction::triggered,
            m_LogWindow, &QTextEdit::clear);
    menu->addAction(clearAction);
    menu->exec(m_LogWindow->mapToGlobal(pos));
    delete menu;
}
