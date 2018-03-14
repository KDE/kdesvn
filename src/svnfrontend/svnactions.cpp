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
#include "svnactions.h"
#include "checkoutinfo_impl.h"
#include "itemdisplay.h"
#include "svnitem.h"
#include "rangeinput_impl.h"
#include "propertiesdlg.h"
#include "ccontextlistener.h"
#include "tcontextlistener.h"
#include "modifiedthread.h"
#include "fillcachethread.h"
#include "svnlogdlgimp.h"
#include "stopdlg.h"
#include "blamedisplay.h"
#include "ksvnwidgets/commitmsg_impl.h"
#include "ksvnwidgets/models/commitmodelhelper.h"
#include "ksvnwidgets/diffbrowser.h"
#include "ksvnwidgets/encodingselector_impl.h"
#include "ksvnwidgets/revertform.h"
#include "graphtree/revisiontree.h"
#include "settings/kdesvnsettings.h"
#include "svnqt/client.h"
#include "svnqt/annotate_line.h"
#include "svnqt/context_listener.h"
#include "svnqt/dirent.h"
#include "svnqt/targets.h"
#include "svnqt/url.h"
#include "svnqt/svnqttypes.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/client_parameter.h"
#include "svnqt/client_commit_parameter.h"
#include "svnqt/client_annotate_parameter.h"
#include "svnqt/client_update_parameter.h"
#include "svnqt/cache/LogCache.h"
#include "svnqt/cache/ReposLog.h"
#include "svnqt/cache/ReposConfig.h"

#include "fronthelpers/watchedprocess.h"

#include "helpers/stringhelper.h"
#include "helpers/kdesvn_debug.h"
#include "helpers/ktranslateurl.h"
#include "helpers/windowgeometryhelper.h"
#include "fronthelpers/cursorstack.h"
#include "cacheentry.h"

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kmimetypetrader.h>
#include <krun.h>

#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMap>
#include <QMimeDatabase>
#include <QReadWriteLock>
#include <QString>
#include <QTimer>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <sys/time.h>
#include <unistd.h>

/// @todo has to be removed for a real fix of ticket #613
#include <svn_config.h>
// wait not longer than 10 seconds for a thread
#define MAX_THREAD_WAITTIME 10000

class SvnActionsData
{
public:
    SvnActionsData()
        : m_ParentList(nullptr)
        , m_SvnContextListener(nullptr)
        , m_Svnclient(svn::Client::getobject(svn::ContextP()))
        , runblocked(false)
    {
    }

    ~SvnActionsData()
    {
        cleanDialogs();
        delete m_SvnContextListener;
    }

    static bool isExternalDiff()
    {
        if (Kdesvnsettings::use_external_diff()) {
            const QString edisp = Kdesvnsettings::external_diff_display();
            const QVector<QStringRef> wlist = edisp.splitRef(QLatin1Char(' '));
            if (wlist.count() >= 3 && edisp.contains(QLatin1String("%1")) && edisp.contains(QLatin1String("%2"))) {
                return true;
            }
        }
        return false;
    }

    void clearCaches()
    {
        QWriteLocker wl(&(m_InfoCacheLock));
        m_PropertiesCache.clear();
        m_contextData.clear();
        m_InfoCache.clear();
    }

    void cleanDialogs()
    {
        if (m_DiffDialog) {
            delete m_DiffDialog;
            m_DiffDialog = nullptr;
        }
        if (m_LogDialog) {
            m_LogDialog->saveSize();
            delete m_LogDialog;
            m_LogDialog = nullptr;
        }
    }

    /// @todo set some standards options to svn::Context. This should made via a Config class in svnqt (future release 1.4)
    /// this is a workaround for ticket #613
    void setStandards()
    {
        if (!m_CurrentContext) {
            return;
        }
        svn_config_t *cfg_config = static_cast<svn_config_t *>(apr_hash_get(m_CurrentContext->ctx()->config, SVN_CONFIG_CATEGORY_CONFIG,
                                                                            APR_HASH_KEY_STRING));
        if (!cfg_config) {
            return;
        }
        svn_config_set(cfg_config, SVN_CONFIG_SECTION_HELPERS,
                       SVN_CONFIG_OPTION_DIFF_CMD, nullptr);
    }

    ItemDisplay *m_ParentList;

    CContextListener *m_SvnContextListener;
    svn::ContextP m_CurrentContext;
    svn::ClientP m_Svnclient;

    helpers::statusCache m_UpdateCache;
    helpers::statusCache m_Cache;
    helpers::statusCache m_conflictCache;
    helpers::statusCache m_repoLockCache;
    helpers::itemCache<svn::PathPropertiesMapListPtr> m_PropertiesCache;
    /// \todo as persistent cache (sqlite?)
    helpers::itemCache<svn::InfoEntry> m_InfoCache;
    helpers::itemCache<QVariant> m_MergeInfoCache;

    QPointer<DiffBrowser> m_DiffBrowserPtr;
    QPointer<KSvnSimpleOkDialog> m_DiffDialog;
    QPointer<SvnLogDlgImp> m_LogDialog;

    QMap<QString, QString> m_contextData;
    QReadWriteLock m_InfoCacheLock;

    bool runblocked;
};

#define EMIT_FINISHED emit sendNotify(i18n("Finished"))
#define EMIT_REFRESH emit sigRefreshAll()
#define DIALOGS_SIZES "display_dialogs_sizes"

SvnActions::SvnActions(ItemDisplay *parent, bool processes_blocked)
    : QObject(parent ? parent->realWidget() : nullptr)
    , SimpleLogCb()
    , m_CThread(nullptr)
    , m_UThread(nullptr)
    , m_FCThread(nullptr)
{
    m_Data.reset(new SvnActionsData);
    m_Data->m_ParentList = parent;
    m_Data->m_SvnContextListener = new CContextListener(this);
    m_Data->runblocked = processes_blocked;
    connect(m_Data->m_SvnContextListener, SIGNAL(sendNotify(QString)), this, SLOT(slotNotifyMessage(QString)));
}

svn::ClientP SvnActions::svnclient()
{
    return m_Data->m_Svnclient;
}

SvnActions::~SvnActions()
{
    killallThreads();
}

void SvnActions::slotNotifyMessage(const QString &aMsg)
{
    emit sendNotify(aMsg);
}

void SvnActions::reInitClient()
{
    m_Data->clearCaches();
    m_Data->cleanDialogs();
    if (m_Data->m_CurrentContext) {
        m_Data->m_CurrentContext->setListener(nullptr);
    }
    m_Data->m_CurrentContext = svn::ContextP(new svn::Context);
    m_Data->m_CurrentContext->setListener(m_Data->m_SvnContextListener);
    m_Data->m_Svnclient->setContext(m_Data->m_CurrentContext);
    ///@todo workaround has to be replaced
    m_Data->setStandards();
}

void SvnActions::makeLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &which, bool follow, bool list_files, int limit)
{
    svn::LogEntriesMapPtr logs = getLog(start, end, peg, which, list_files, limit, follow);
    if (!logs) {
        return;
    }
    svn::InfoEntry info;
    if (!singleInfo(which, peg, info)) {
        return;
    }
    const QString reposRoot = info.reposRoot().toString();
    bool need_modal = m_Data->runblocked || QApplication::activeModalWidget() != nullptr;
    if (need_modal || !m_Data->m_LogDialog) {
        m_Data->m_LogDialog = new SvnLogDlgImp(this, need_modal);
        connect(m_Data->m_LogDialog, SIGNAL(makeDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)),
                this, SLOT(makeDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)));
        connect(m_Data->m_LogDialog, SIGNAL(makeCat(svn::Revision,QString,QString,svn::Revision,QWidget*)),
                this, SLOT(slotMakeCat(svn::Revision,QString,QString,svn::Revision,QWidget*)));
    }

    if (m_Data->m_LogDialog) {
        m_Data->m_LogDialog->dispLog(logs, info.url().toString().mid(reposRoot.length()), reposRoot,
                                     (
                                         peg == svn::Revision::UNDEFINED ?
                                         (svn::Url::isValid(which) ? svn::Revision::HEAD : svn::Revision::UNDEFINED) :
                                         peg
                                     ), which);
        if (need_modal) {
            m_Data->m_LogDialog->exec();
            m_Data->m_LogDialog->saveSize();
            delete m_Data->m_LogDialog;
        } else {
            m_Data->m_LogDialog->show();
            m_Data->m_LogDialog->raise();
        }
    }
    EMIT_FINISHED;
}

svn::LogEntriesMapPtr SvnActions::getLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &which, bool list_files,
                                         int limit, QWidget *parent)
{
    return getLog(start, end, peg, which, list_files, limit, Kdesvnsettings::log_follows_nodes(), parent);
}

svn::LogEntriesMapPtr SvnActions::getLog(const svn::Revision &start, const svn::Revision &end, const svn::Revision &peg, const QString &which, bool list_files,
                                         int limit, bool follow, QWidget *parent)
{
    svn::LogEntriesMapPtr logs;
    if (!m_Data->m_CurrentContext) {
        return logs;
    }

    bool mergeinfo = hasMergeInfo(m_Data->m_ParentList->baseUri().isEmpty() ? which : m_Data->m_ParentList->baseUri());

    svn::LogParameter params;
    params.targets(which).revisionRange(start, end).peg(peg).includeMergedRevisions(mergeinfo).limit(limit).discoverChangedPathes(list_files).strictNodeHistory(!follow);

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, (parent ? parent : m_Data->m_ParentList->realWidget()),
                     i18nc("@title:window", "Logs"), i18n("Getting logs - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        logs = svn::LogEntriesMapPtr(new svn::LogEntriesMap);
        if (doNetworking()) {
            if (!m_Data->m_Svnclient->log(params, *logs)) {
                logs.clear();
                return logs;
            }
        } else {
            svn::InfoEntry e;
            if (!singleInfo(m_Data->m_ParentList->baseUri(), svn::Revision::BASE, e)) {
                logs.clear();
                return logs;
            }
            if (svn::Url::isLocal(e.reposRoot().toString())) {
                if (!m_Data->m_Svnclient->log(params, *logs)) {
                    logs.clear();
                    return logs;
                }
            } else {
                svn::cache::ReposLog rl(m_Data->m_Svnclient, e.reposRoot().toString());
                QString what;
                const QString s1 = e.url().toString().mid(e.reposRoot().toString().length());
                if (which == QLatin1String(".")) {
                    what = s1;
                } else {
                    const QString s2 = which.mid(m_Data->m_ParentList->baseUri().length());
                    what = s1 + QLatin1Char('/') + s2;
                }
                rl.log(what, start, end, peg, *logs, !follow, limit);
            }
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        logs.clear();
    }
    if (logs && logs->isEmpty()) {
        logs.clear();
        emit clientException(i18n("Got no logs"));
    }
    return logs;
}

bool SvnActions::getSingleLog(svn::LogEntry &t, const svn::Revision &r, const QString &what, const svn::Revision &peg, QString &root)
{
    bool res = false;

    if (what.isEmpty()) {
        return res;
    }
    if (root.isEmpty()) {
        svn::InfoEntry inf;
        if (!singleInfo(what, peg, inf)) {
            return res;
        }
        root = inf.reposRoot().toString();
    }

    if (!svn::Url::isLocal(root)) {
        svn::LogEntriesMap _m;
        try {
            svn::cache::ReposLog rl(m_Data->m_Svnclient , root);
            if (rl.isValid() && rl.simpleLog(_m, r, r, true)) {
                const svn::LogEntriesMap::const_iterator it = _m.constFind(r.revnum());
                if (it != _m.constEnd()) {
                    t = it.value();
                    res = true;
                }
            }
        } catch (const svn::Exception &e) {
            emit clientException(e.msg());
        }
    }

    if (!res) {
        svn::LogEntriesMapPtr log = getLog(r, r, peg, root, true, 1);
        if (log) {
            const svn::LogEntriesMap::const_iterator it = log->constFind(r.revnum());
            if (it != log->constEnd()) {
                t = it.value();
                res = true;
            }
        }
    }
    return res;
}

bool SvnActions::hasMergeInfo(const QString &originpath)
{
    QVariant _m(false);
    QString path;

    svn::InfoEntry e;
    if (!singleInfo(originpath, svn::Revision::UNDEFINED, e)) {
        return false;
    }
    path = e.reposRoot().toString();
    if (!m_Data->m_MergeInfoCache.findSingleValid(path, _m)) {
        bool mergeinfo;
        try {
            mergeinfo = m_Data->m_Svnclient->RepoHasCapability(path, svn::CapabilityMergeinfo);
        } catch (const svn::ClientException &e) {
            emit sendNotify(e.msg());
            return false;
        }
        _m.setValue(mergeinfo);
        m_Data->m_MergeInfoCache.insertKey(_m, path);
    }
    return _m.toBool();
}

bool SvnActions::singleInfo(const QString &what, const svn::Revision &_rev, svn::InfoEntry &target, const svn::Revision &_peg)
{
    QString url;
    QString cacheKey;
    QTime d; d.start();
    svn::Revision peg = _peg;
    if (!m_Data->m_CurrentContext) {
        return false;
    }
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif

    if (!svn::Url::isValid(what)) {
        // working copy
        // url = svn::Wc::getUrl(what);
        url = what;
        if (_rev != svn::Revision::WORKING && url.contains(QLatin1Char('@'))) {
            url += QStringLiteral("@BASE");
        }
        peg = svn::Revision::UNDEFINED;
        cacheKey = url;
    } else {
        // valid url
        QUrl _uri(what);
        QString prot = svn::Url::transformProtokoll(_uri.scheme());
        _uri.setScheme(prot);
        url = _uri.toString();
        if (peg == svn::Revision::UNDEFINED) {
            peg = _rev;
        }
        if (peg == svn::Revision::UNDEFINED) {
            peg = svn::Revision::HEAD;
        }
        cacheKey = _rev.toString() + QLatin1Char('/') + url;
    }
    svn::InfoEntries e;
    bool must_write = false;

    {
        QReadLocker rl(&(m_Data->m_InfoCacheLock));
        if (cacheKey.isEmpty() || !m_Data->m_InfoCache.findSingleValid(cacheKey, target)) {
            must_write = true;
            try {
                e = (m_Data->m_Svnclient->info(url, svn::DepthEmpty, _rev, peg));
            } catch (const svn::Exception &ce) {
                qCDebug(KDESVN_LOG) << "single info: " << ce.msg() << endl;
                emit clientException(ce.msg());
                return false;
            }
            if (e.isEmpty() || e[0].reposRoot().isEmpty()) {
                emit clientException(i18n("Got no info."));
                return false;
            }
            target = e[0];
        }
    }
    if (must_write) {
        QWriteLocker wl(&(m_Data->m_InfoCacheLock));
        if (!cacheKey.isEmpty()) {
            m_Data->m_InfoCache.insertKey(e[0], cacheKey);
            if (peg != svn::Revision::UNDEFINED && peg.kind() != svn::Revision::NUMBER &&  peg.kind() != svn::Revision::DATE) {
                // for persistent storage, store head into persistent cache makes no sense.
                cacheKey = e[0].revision().toString() + QLatin1Char('/') + url;
                m_Data->m_InfoCache.insertKey(e[0], cacheKey);
            }
        }
    }
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Time getting info for " << cacheKey << ": " << _counttime.elapsed();
#endif

    return true;
}

void SvnActions::makeTree(const QString &what, const svn::Revision &_rev, const svn::Revision &startr, const svn::Revision &endr)
{
    svn::InfoEntry info;
    if (!singleInfo(what, _rev, info)) {
        return;
    }
    const QString reposRoot = info.reposRoot().toString();

    if (Kdesvnsettings::fill_cache_on_tree()) {
        stopFillCache();
    }

    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("revisiontree_dlg"), m_Data->m_ParentList->realWidget()));
    dlg->setWindowTitle(i18nc("@title:window", "History of %1", info.url().toString().mid(reposRoot.length())));

    RevisionTree *rt(new RevisionTree(m_Data->m_Svnclient, m_Data->m_SvnContextListener, reposRoot,
                                      startr, endr,
                                      info.url().toString().mid(reposRoot.length()), _rev, dlg));
    if (rt->isValid()) {
        QWidget *disp = rt->getView();
        if (disp) {
            dlg->addWidget(rt->getView());
            connect(
                disp, SIGNAL(makeNorecDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)),
                this, SLOT(makeNorecDiff(QString,svn::Revision,QString,svn::Revision,QWidget*))
            );
            connect(
                disp, SIGNAL(makeRecDiff(QString,svn::Revision,QString,svn::Revision,QWidget*)),
                this, SLOT(makeDiff(QString,svn::Revision,QString,svn::Revision,QWidget*))
            );
            connect(disp, SIGNAL(makeCat(svn::Revision,QString,QString,svn::Revision,QWidget*)),
                    this, SLOT(slotMakeCat(svn::Revision,QString,QString,svn::Revision,QWidget*)));
            dlg->exec();
        }
    }
    delete dlg;
}

void SvnActions::makeBlame(const svn::Revision &start, const svn::Revision &end, SvnItem *k)
{
    if (k) {
        makeBlame(start, end, k->fullName(), m_Data->m_ParentList->realWidget());
    }
}

void SvnActions::makeBlame(const svn::Revision &start, const svn::Revision &end, const QString &k, QWidget *_p, const svn::Revision &_peg, SimpleLogCb *_acb)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    svn::AnnotatedFile blame;
    QWidget *_parent = _p ? _p : m_Data->m_ParentList->realWidget();
    bool mergeinfo = hasMergeInfo(m_Data->m_ParentList->baseUri().isEmpty() ? k : m_Data->m_ParentList->baseUri());

    svn::AnnotateParameter params;
    params.path(k).pegRevision(_peg == svn::Revision::UNDEFINED ? end : _peg).revisionRange(svn::RevisionRange(start, end)).includeMerged(mergeinfo);

    try {
        CursorStack a(Qt::BusyCursor);
        StopDlg sdlg(m_Data->m_SvnContextListener, _parent,
                     i18nc("@title:window", "Annotate"), i18n("Annotate lines - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->annotate(blame, params);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    if (blame.isEmpty()) {
        QString ex = i18n("Got no annotate");
        emit clientException(ex);
        return;
    }
    EMIT_FINISHED;
    BlameDisplay::displayBlame(_acb ? _acb : this, k, blame, _p);
}

bool SvnActions::makeGet(const svn::Revision &start, const QString &what, const QString &target,
                         const svn::Revision &peg, QWidget *_dlgparent)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    CursorStack a(Qt::BusyCursor);
    QWidget *dlgp = _dlgparent ? _dlgparent : m_Data->m_ParentList->realWidget();
    svn::Path p(what);
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, dlgp,
                     i18nc("@title:window", "Content Get"), i18n("Getting content - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->get(p, target, start, peg);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    } catch (...) {
        QString ex = i18n("Error getting content");
        emit clientException(ex);
        return false;
    }
    return true;
}

void SvnActions::slotMakeCat(const svn::Revision &start, const QString &what, const QString &disp, const svn::Revision &peg, QWidget *_dlgparent)
{
    QTemporaryFile content;
    content.setAutoRemove(true);
    // required otherwise it will not generate a unique name...
    if (!content.open()) {
        emit clientException(i18n("Error while open temporary file"));
        return;
    }
    QString tname = content.fileName();
    content.close();
    QWidget *parent = _dlgparent ? _dlgparent : m_Data->m_ParentList->realWidget();

    if (!makeGet(start, what, tname, peg, parent)) {
        return;
    }
    EMIT_FINISHED;
    QMimeDatabase db;
    const QMimeType mimeType(db.mimeTypeForFile(tname));
    KService::List offers = KMimeTypeTrader::self()->query(mimeType.name(),
                                                           QLatin1String("Application"),
                                                           QLatin1String("Type == 'Application' or (exist Exec)"));
    if (offers.isEmpty() || offers.first()->exec().isEmpty()) {
        offers = KMimeTypeTrader::self()->query(mimeType.name(),
                                                QLatin1String("Application"),
                                                QLatin1String("Type == 'Application'"));
    }
    KService::List::ConstIterator it = offers.constBegin();
    for (; it != offers.constEnd(); ++it) {
        if ((*it)->noDisplay()) {
            continue;
        }
        break;
    }
    if (it != offers.constEnd()) {
        content.setAutoRemove(false);
        KRun::runService(**it, QList<QUrl>() << QUrl::fromLocalFile(tname), QApplication::activeWindow(), true);
        return;
    }

    QFile file(tname);
    file.open(QIODevice::ReadOnly);
    const QByteArray co = file.readAll();

    if (!co.isEmpty()) {
        QPointer<KSvnSimpleOkDialog> dlg = new KSvnSimpleOkDialog(QStringLiteral("cat_display_dlg"), parent);
        dlg->setWindowTitle(i18nc("@title:window", "Content of %1", disp));
        QTextBrowser *ptr = new QTextBrowser(dlg);
        ptr->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        ptr->setWordWrapMode(QTextOption::NoWrap);
        ptr->setReadOnly(true);
        ptr->setText(QString::fromUtf8(co, co.size()));
        dlg->addWidget(ptr);
        dlg->exec();
        delete dlg;
    } else {
        KMessageBox::information(parent, i18n("Got no content."));
    }
}

bool SvnActions::makeMkdir(const svn::Targets &targets, const QString &logMessage)
{
    if (!m_Data->m_CurrentContext || targets.targets().isEmpty()) {
        return false;
    }
    try {
        m_Data->m_Svnclient->mkdir(targets, logMessage);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

QString SvnActions::makeMkdir(const QString &parentDir)
{
    if (!m_Data->m_CurrentContext) {
        return QString();
    }
    bool isOk = false;
    const QString ex = QInputDialog::getText(m_Data->m_ParentList->realWidget(), i18n("New folder"), i18n("Enter folder name:"),
                                             QLineEdit::Normal, QString(), &isOk);
    if (!isOk || ex.isEmpty()) {
        return QString();
    }
    svn::Path target(parentDir);
    target.addComponent(ex);

    try {
        m_Data->m_Svnclient->mkdir(target, QString());
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return QString();
    }

    return target.path();
}

QString SvnActions::getInfo(const SvnItemList &lst, const svn::Revision &rev, const svn::Revision &peg, bool recursive, bool all)
{
    QString res;
    for (auto it = lst.cbegin(); it != lst.cend(); ++it) {
        if (all) {
            res += QStringLiteral("<h4 align=\"center\">%1</h4>").arg((*it)->fullName());
        }
        res += getInfo((*it)->fullName(), rev, peg, recursive, all);
    }
    return res;
}

QString SvnActions::getInfo(const QString &_what, const svn::Revision &rev, const svn::Revision &peg, bool recursive, bool all)
{
    if (!m_Data->m_CurrentContext) {
        return QString();
    }
    svn::InfoEntries entries;
    if (recursive) {
        try {
            StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                         i18nc("@title:window", "Details"), i18n("Retrieving information - hit Cancel for abort"));
            connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
            QString path = _what;
            if (_what.contains(QLatin1Char('@')) && !svn::Url::isValid(_what)) {
                path += QLatin1String("@BASE");
            }
            entries = (m_Data->m_Svnclient->info(path,
                                                 recursive ? svn::DepthInfinity : svn::DepthEmpty,
                                                 rev,
                                                 peg));
        } catch (const svn::Exception &e) {
            emit clientException(e.msg());
            return QString();
        }
    } else {
        svn::InfoEntry info;
        if (!singleInfo(_what, rev, info, peg)) {
            return QString();
        }
        entries.append(info);
    }
    return getInfo(entries, _what, all);
}

QString SvnActions::getInfo(const svn::InfoEntries &entries, const QString &_what, bool all)
{
    QString text;
    static QString rb(QStringLiteral("<tr><td><nobr>"));
    static QString re(QStringLiteral("</nobr></td></tr>\n"));
    static QString cs(QStringLiteral("</nobr>:</td><td><nobr>"));
    unsigned int val = 0;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (val > 0) {
            text += QStringLiteral("<hline>");
        }
        ++val;
        text += QStringLiteral("<p align=\"center\">");
        text += QStringLiteral("<table cellspacing=0 cellpadding=0>");
        if (!(*it).Name().isEmpty()) {
            text += rb + i18n("Name") + cs + ((*it).Name()) + re;
        }
        if (all) {
            text += rb + i18n("URL") + cs + ((*it).url().toDisplayString()) + re;
            if (!(*it).reposRoot().toString().isEmpty()) {
                text += rb + i18n("Canonical repository URL") + cs + ((*it).reposRoot().toDisplayString()) + re;
            }
            if (!(*it).checksum().isEmpty()) {
                text += rb + i18n("Checksum") + cs + ((*it).checksum()) + re;
            }
        }
        text += rb + i18n("Type") + cs;
        switch ((*it).kind()) {
        case svn_node_none:
            text += i18n("Absent");
            break;
        case svn_node_file:
            text += i18n("File");
            break;
        case svn_node_dir:
            text += i18n("Folder");
            break;
        case svn_node_unknown:
        default:
            text += i18n("Unknown");
            break;
        }
        text += re;
        if ((*it).kind() == svn_node_file) {
            text += rb + i18n("Size") + cs;
            if ((*it).size() != svn::InfoEntry::SVNQT_SIZE_UNKNOWN) {
                text += helpers::ByteToString((*it).size());
            } else if ((*it).working_size() != svn::InfoEntry::SVNQT_SIZE_UNKNOWN) {
                text += helpers::ByteToString((*it).working_size());
            }
            text += re;
        }
        if (all) {
            text += rb + i18n("Schedule") + cs;
            switch ((*it).Schedule()) {
            case svn_wc_schedule_normal:
                text += i18n("Normal");
                break;
            case svn_wc_schedule_add:
                text += i18n("Addition");
                break;
            case svn_wc_schedule_delete:
                text += i18n("Deletion");
                break;
            case svn_wc_schedule_replace:
                text += i18n("Replace");
                break;
            default:
                text += i18n("Unknown");
                break;
            }
            text += re;
            text += rb + i18n("UUID") + cs + ((*it).uuid()) + re;
        }
        text += rb + i18n("Last author") + cs + ((*it).cmtAuthor()) + re;
        if ((*it).cmtDate().IsValid()) {
            text += rb + i18n("Last committed") + cs + (*it).cmtDate().toString() + re;
        }
        text += rb + i18n("Last revision") + cs + (*it).cmtRev().toString() + re;
        if ((*it).textTime().IsValid()) {
            text += rb + i18n("Content last changed") + cs + (*it).textTime().toString() + re;
        }
        if (all) {
            if ((*it).propTime().IsValid()) {
                text += rb + i18n("Property last changed") + cs + (*it).propTime().toString() + re;
            }
            for (int _cfi = 0; _cfi < (*it).conflicts().size(); ++_cfi) {
                text += rb + i18n("New version of conflicted file") + cs + ((*it).conflicts()[_cfi]->theirFile());
            }
            if ((*it).prejfile().length()) {
                text += rb + i18n("Property reject file") +
                        cs + ((*it).prejfile()) + re;
            }

            if (!(*it).copyfromUrl().isEmpty()) {
                text += rb + i18n("Copy from URL") + cs + ((*it).copyfromUrl().toDisplayString()) + re;
            }
            if ((*it).lockEntry().Locked()) {
                text += rb + i18n("Lock token") + cs + ((*it).lockEntry().Token()) + re;
                text += rb + i18n("Owner") + cs + ((*it).lockEntry().Owner()) + re;
                text += rb + i18n("Locked on") + cs +
                        (*it).lockEntry().Date().toString() +
                        re;
                text += rb + i18n("Lock comment") + cs +
                        (*it).lockEntry().Comment() + re;
            } else {
                svn::StatusPtr d;
                if (checkReposLockCache(_what, d) && d && d->lockEntry().Locked()) {
                    text += rb + i18n("Lock token") + cs + (d->lockEntry().Token()) + re;
                    text += rb + i18n("Owner") + cs + (d->lockEntry().Owner()) + re;
                    text += rb + i18n("Locked on") + cs +
                            d->lockEntry().Date().toString() +
                            re;
                    text += rb + i18n("Lock comment") + cs +
                            d->lockEntry().Comment() + re;
                }
            }
        }
        text += QStringLiteral("</table></p>\n");
    }
    return text;
}

void SvnActions::makeInfo(const SvnItemList &lst, const svn::Revision &rev, const svn::Revision &peg, bool recursive)
{
    QStringList infoList;
    infoList.reserve(lst.size());
    for (int i = 0; i < lst.size(); ++i) {
        const QString text = getInfo(lst.at(i)->fullName(), rev, peg, recursive, true);
        if (!text.isEmpty()) {
            infoList += text;
        }
    }
    showInfo(infoList);
}

void SvnActions::makeInfo(const QStringList &lst, const svn::Revision &rev, const svn::Revision &peg, bool recursive)
{
    QStringList infoList;
    infoList.reserve(lst.size());
    for (int i = 0; i < lst.size(); ++i) {
        const QString text = getInfo(lst.at(i), rev, peg, recursive, true);
        if (!text.isEmpty()) {
            infoList += text;
        }
    }
    showInfo(infoList);
}

void SvnActions::showInfo(const QStringList &infoList)
{
    if (infoList.isEmpty()) {
        return;
    }
    QString text(QLatin1String("<html><head></head><body>"));
    for (int i = 0; i < infoList.count(); ++i) {
        text += QLatin1String("<h4 align=\"center\">") + infoList.at(i) + QLatin1String("</h4>");
    }
    text += QLatin1String("</body></html>");

    QPointer<KSvnSimpleOkDialog> dlg = new KSvnSimpleOkDialog(QStringLiteral("info_dialog"), QApplication::activeModalWidget());
    dlg->setWindowTitle(i18nc("@title:window", "Infolist"));
    QTextBrowser *ptr = new QTextBrowser(dlg);
    dlg->addWidget(ptr);
    ptr->setReadOnly(true);
    ptr->setText(text);
    dlg->exec();
    delete dlg;
}


void SvnActions::editProperties(SvnItem *k, const svn::Revision &rev)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    if (!k) {
        return;
    }
    QPointer<PropertiesDlg> dlg(new PropertiesDlg(k, svnclient(), rev));
    connect(dlg, SIGNAL(clientException(QString)), m_Data->m_ParentList->realWidget(), SLOT(slotClientException(QString)));
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    svn::PropertiesMap setList;
    QStringList delList;
    dlg->changedItems(setList, delList);
    changeProperties(setList, delList, k->fullName());
    k->refreshStatus();
    EMIT_FINISHED;
    delete dlg;
}

bool SvnActions::changeProperties(const svn::PropertiesMap &setList, const QStringList &delList, const QString &path, const svn::Depth &depth)
{
    try {
        svn::PropertiesParameter params;
        params.path(path).depth(depth);
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Applying Properties"), i18n("<center>Applying<br/>hit cancel for abort</center>"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        // propertyValue == QString::null -> delete property
        for (int pos = 0; pos < delList.size(); ++pos) {
            m_Data->m_Svnclient->propset(params.propertyName(delList.at(pos)));
        }
        for (svn::PropertiesMap::ConstIterator it = setList.begin(); it != setList.end(); ++it) {
            m_Data->m_Svnclient->propset(params.propertyName(it.key()).propertyValue(it.value()));
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

/*!
    \fn SvnActions::slotCommit()
 */
void SvnActions::doCommit(const SvnItemList &which)
{
    if (!m_Data->m_CurrentContext || !m_Data->m_ParentList->isWorkingCopy()) {
        return;
    }
    SvnItemList::const_iterator liter = which.begin();

    svn::Paths targets;
    if (which.isEmpty()) {
        targets.push_back(svn::Path(QStringLiteral(".")));
    } else {
        targets.reserve(which.size());
        for (; liter != which.end(); ++liter) {
            targets.push_back(svn::Path(
                                  m_Data->m_ParentList->relativePath((*liter))
                              ));
        }
    }
    if (!m_Data->m_ParentList->baseUri().isEmpty()) {
        if (!QDir::setCurrent(m_Data->m_ParentList->baseUri())) {
            QString msg = i18n("Could not change to folder %1\n", m_Data->m_ParentList->baseUri())
                          + QString::fromLocal8Bit(strerror(errno));
            emit sendNotify(msg);
        }
    }
    if (makeCommit(svn::Targets(targets)) && Kdesvnsettings::log_cache_on_open()) {
        startFillCache(m_Data->m_ParentList->baseUri(), true);
    }
}

bool SvnActions::makeCommit(const svn::Targets &targets)
{
    bool ok, keeplocks;
    svn::Depth depth;
    svn::Revision nnum;
    bool review = Kdesvnsettings::review_commit();
    QString msg;

    if (!doNetworking()) {
        emit clientException(i18n("Not commit because networking is disabled"));
        return false;
    }

    svn::CommitParameter commit_parameters;
    stopFillCache();
    if (!review) {
        msg = Commitmsg_impl::getLogmessage(&ok, &depth, &keeplocks, m_Data->m_ParentList->realWidget());
        if (!ok) {
            return false;
        }
        commit_parameters.targets(targets);
    } else {
        CommitActionEntries _check, _uncheck, _result;
        svn::StatusEntries _Cache;
        depth = svn::DepthEmpty;
        svn::StatusParameter params;
        params.depth(svn::DepthInfinity).all(false).update(false).noIgnore(false).revision(svn::Revision::HEAD);
        /// @todo filter out double entries
        for (size_t j = 0; j < targets.size(); ++j) {
            try {
                StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                             i18nc("@title:window", "Status / List"), i18n("Creating list / check status"));
                _Cache = m_Data->m_Svnclient->status(params.path(targets.target(j).path()));
            } catch (const svn::Exception &e) {
                emit clientException(e.msg());
                return false;
            }
            for (int i = 0; i < _Cache.count(); ++i) {
                const svn::StatusPtr ptr = _Cache.at(i);
                const QString _p = ptr->path();
                // check the node status, not the text status (it does not cover the prop status)
                if (ptr->isRealVersioned() && (
                            ptr->nodeStatus() == svn_wc_status_modified ||
                            ptr->nodeStatus() == svn_wc_status_added ||
                            ptr->nodeStatus() == svn_wc_status_replaced ||
                            ptr->nodeStatus() == svn_wc_status_deleted ||
                            ptr->nodeStatus() == svn_wc_status_modified
                        )) {
                    if (ptr->nodeStatus() == svn_wc_status_deleted) {
                        _check.append(CommitActionEntry(_p, i18n("Delete"), CommitActionEntry::DELETE));
                    } else {
                        _check.append(CommitActionEntry(_p, i18n("Commit"), CommitActionEntry::COMMIT));
                    }
                } else if (ptr->nodeStatus() == svn_wc_status_missing) {
                    _uncheck.append(CommitActionEntry(_p, i18n("Delete and Commit"), CommitActionEntry::MISSING_DELETE));
                } else if (!ptr->isVersioned()) {
                    _uncheck.append(CommitActionEntry(_p, i18n("Add and Commit"), CommitActionEntry::ADD_COMMIT));
                }
            }
        }
        msg = Commitmsg_impl::getLogmessage(_check, _uncheck, this, _result, &ok, &keeplocks, m_Data->m_ParentList->realWidget());
        if (!ok || _result.isEmpty()) {
            return false;
        }
        svn::Paths _add, _commit, _delete;
        depth = svn::DepthInfinity;
        for (long i = 0; i < _result.count(); ++i) {
            _commit.append(_result[i].name());
            if (_result[i].type() == CommitActionEntry::ADD_COMMIT) {
                _add.append(_result[i].name());
            } else if (_result[i].type() == CommitActionEntry::MISSING_DELETE) {
                _delete.append(_result[i].name());
            }
        }
        if (!_add.isEmpty()) {
            if (!addItems(_add, svn::DepthEmpty)) {
                return false;
            }
        }
        if (!_delete.isEmpty()) {
            makeDelete(svn::Targets(_delete));
        }
        commit_parameters.targets(svn::Targets(_commit));
    }
    commit_parameters.keepLocks(keeplocks).depth(depth).message(msg);
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Commit"), i18n("Commit - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        nnum = m_Data->m_Svnclient->commit(commit_parameters);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    emit sendNotify(i18n("Committed revision %1.", nnum.toString()));
    return true;
}

void SvnActions::slotProcessDataRead(const QByteArray &data, WatchedProcess *)
{
    emit sendNotify(QString::fromLocal8Bit(data));
}

bool SvnActions::get(const QString &what, const QString &to, const svn::Revision &rev, const svn::Revision &peg, QWidget *p)
{
    svn::Revision _peg = peg;
    if (_peg == svn::Revision::UNDEFINED) {
        _peg = rev;
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, p ? p : m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Downloading"), i18n("Download - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->get(svn::Path(what),
                                 to, rev, _peg);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

/*!
    \fn SvnActions::makeDiff(const QString&,const svn::Revision&start,const svn::Revision&end)
 */
void SvnActions::makeDiff(const QString &what, const svn::Revision &start, const svn::Revision &end, const svn::Revision &_peg, bool isDir)
{
    makeDiff(what, start, what, end, _peg, isDir, m_Data->m_ParentList->realWidget());
}

void SvnActions::makeDiff(const QString &p1, const svn::Revision &start, const QString &p2, const svn::Revision &end)
{
    makeDiff(p1, start, p2, end, (QWidget *)nullptr);
}

void SvnActions::makeDiff(const QString &p1, const svn::Revision &start, const QString &p2, const svn::Revision &end, QWidget *p)
{
    if (!doNetworking() && start != svn::Revision::BASE && end != svn::Revision::WORKING) {
        emit sendNotify(i18n("Can not do this diff because networking is disabled."));
        return;
    }
    if (m_Data->isExternalDiff()) {
        svn::InfoEntry info;
        if (singleInfo(p1, start, info)) {
            makeDiff(p1, start, p2, end, end, info.isDir(), p);
        }
        return;
    }
    makeDiffinternal(p1, start, p2, end, p);
}

void SvnActions::makeDiffExternal(const QString &p1, const svn::Revision &start, const QString &p2, const svn::Revision &end, const svn::Revision &_peg,
                                  bool isDir, QWidget *p, bool rec)
{
    QFileInfo f1(p1);
    QFileInfo f2(p2);
    QTemporaryFile tfile(QDir::tempPath() + QLatin1Char('/') + f1.fileName() + QLatin1Char('-') + start.toString());
    QTemporaryFile tfile2(QDir::tempPath() + QLatin1Char('/') + f2.fileName() + QLatin1Char('-') + end.toString());

    QString s1 = f1.fileName() + QLatin1Char('-') + start.toString();
    QString s2 = f2.fileName() + QLatin1Char('-') + end.toString();
    if (f1.fileName() == f2.fileName() && p1 != p2) {
        s2.append(QStringLiteral("-sec"));
    }
    QTemporaryDir tdir1;
    tdir1.setAutoRemove(true);
    tfile.setAutoRemove(true);
    tfile2.setAutoRemove(true);

    tfile.open();
    tfile2.open();

    QString first, second;
    svn::Revision peg = _peg;

    if (start != svn::Revision::WORKING) {
        first = isDir ? tdir1.path() + QLatin1Char('/') + s1 : tfile.fileName();
    } else {
        first = p1;
    }
    if (end != svn::Revision::WORKING) {
        second = isDir ? tdir1.path() + QLatin1Char('/') + s2 : tfile2.fileName();
    } else {
        second = p2;
    }
    if (second == first) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("Both entries seems to be the same, can not diff."));
        return;
    }
    if (start != svn::Revision::WORKING) {
        if (!isDir) {
            if (!get(p1, tfile.fileName(), start, peg, p)) {
                return;
            }
        } else {
            if (!makeCheckout(p1, first, start, peg,
                              rec ? svn::DepthInfinity : svn::DepthFiles, true, false, false, false, false, p)) {
                return;
            }
        }
    }
    if (end != svn::Revision::WORKING) {
        if (!isDir) {
            if (!get(p2, tfile2.fileName(), end, peg, p)) {
                return;
            }
        } else {
            if (!makeCheckout(p2, second, end, peg,
                              rec ? svn::DepthInfinity : svn::DepthFiles, true, false, false, false, false, p)) {
                return;
            }
        }
    }

    const QString edisp = Kdesvnsettings::external_diff_display();
    const QVector<QStringRef> wlist = edisp.splitRef(QLatin1Char(' '));
    WatchedProcess *proc = new WatchedProcess(this);
    for (auto it = wlist.begin(); it != wlist.end(); ++it) {
        if (*it == QLatin1String("%1")) {
            *proc << first;
        } else if (*it == QLatin1String("%2")) {
            *proc << second;
        } else {
            *proc << (*it).toString();
        }
    }
    proc->setAutoDelete(true);
    proc->setOutputChannelMode(KProcess::MergedChannels);
    connect(proc, SIGNAL(dataStderrRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));
    connect(proc, SIGNAL(dataStdoutRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));
    if (!isDir) {
        tfile2.setAutoRemove(false);
        tfile.setAutoRemove(false);
        proc->appendTempFile(tfile.fileName());
        proc->appendTempFile(tfile2.fileName());
    } else {
        tdir1.setAutoRemove(false);
        proc->appendTempDir(tdir1.path());
    }
    tfile.close();
    tfile2.close();

    proc->start();
    if (proc->waitForStarted(-1)) {
        if (m_Data->runblocked) {
            proc->waitForFinished(-1);
        }
        return;
    } else {
        emit sendNotify(i18n("Diff-process could not started, check command."));
    }
}

void SvnActions::makeDiff(const QString &p1, const svn::Revision &start, const QString &p2, const svn::Revision &end, const svn::Revision &_peg, bool isDir, QWidget *p)
{
    if (m_Data->isExternalDiff()) {
        makeDiffExternal(p1, start, p2, end, _peg, isDir, p);
    } else {
        makeDiffinternal(p1, start, p2, end, p, _peg);
    }
}

void SvnActions::makeDiffinternal(const QString &p1, const svn::Revision &r1, const QString &p2, const svn::Revision &r2, QWidget *p, const svn::Revision &_peg)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    QByteArray ex;
    QTemporaryDir tdir;
    tdir.setAutoRemove(true);
    QString tn(tdir.path() + QLatin1String("/svndiff"));
    QDir d1(tdir.path());
    d1.mkdir(QStringLiteral("svndiff"));
    bool ignore_content = Kdesvnsettings::diff_ignore_content();
    bool gitformat = Kdesvnsettings::diff_gitformat_default();
    bool copy_as_add = Kdesvnsettings::diff_copies_as_add();
    QWidget *parent = p ? p : m_Data->m_ParentList->realWidget();
    QStringList extraOptions;
    if (Kdesvnsettings::diff_ignore_spaces()) {
        extraOptions.append(QStringLiteral("-b"));
    }
    if (Kdesvnsettings::diff_ignore_all_white_spaces()) {
        extraOptions.append(QStringLiteral("-w"));
    }
    svn::Revision peg = _peg == svn::Revision::UNDEFINED ? r2 : _peg;
    svn::DiffParameter _opts;
    _opts.path1(p1).path2(p2).tmpPath(tn).
        peg(peg).rev1(r1).rev2(r2).
        ignoreContentType(ignore_content).extra(svn::StringArray(extraOptions)).depth(svn::DepthInfinity).ignoreAncestry(false).noDiffDeleted(false).changeList(svn::StringArray()).
        git_diff_format(gitformat).copies_as_adds(copy_as_add);

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, parent,
                     i18nc("@title:window", "Diffing"), i18n("Diffing - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        if (p1 == p2 && (r1.isRemote() || r2.isRemote())) {
            ex = m_Data->m_Svnclient->diff_peg(_opts);
        } else {
            ex = m_Data->m_Svnclient->diff(_opts.relativeTo(p1 == p2 ? svn::Path(p1) : svn::Path()));
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    if (ex.isEmpty()) {
        emit clientException(i18n("No difference to display"));
        return;
    }
    dispDiff(ex);
}

void SvnActions::makeNorecDiff(const QString &p1, const svn::Revision &r1, const QString &p2, const svn::Revision &r2, QWidget *_p)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    if (m_Data->isExternalDiff()) {
        svn::InfoEntry info;
        if (singleInfo(p1, r1, info)) {
            makeDiffExternal(p1, r1, p2, r2, r2, info.isDir(), _p, false);
        }
        return;
    }
    QStringList extraOptions;
    if (Kdesvnsettings::diff_ignore_spaces()) {
        extraOptions.append(QStringLiteral("-b"));
    }
    if (Kdesvnsettings::diff_ignore_all_white_spaces()) {
        extraOptions.append(QStringLiteral("-w"));
    }
    QByteArray ex;
    QTemporaryDir tdir;
    tdir.setAutoRemove(true);
    QString tn(tdir.path() + QLatin1String("/svndiff"));
    QDir d1(tdir.path());
    d1.mkdir(QStringLiteral("svndiff"));
    bool ignore_content = Kdesvnsettings::diff_ignore_content();
    svn::DiffParameter _opts;
    // no peg revision required
    _opts.path1(p1).path2(p2).tmpPath(tn).
    rev1(r1).rev2(r2).
    ignoreContentType(ignore_content).extra(svn::StringArray(extraOptions)).depth(svn::DepthEmpty).ignoreAncestry(false).noDiffDeleted(false).changeList(svn::StringArray());

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, _p ? _p : m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Diffing"), i18n("Diffing - hit cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        ex = m_Data->m_Svnclient->diff(_opts);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    EMIT_FINISHED;
    if (ex.isEmpty()) {
        emit clientException(i18n("No difference to display"));
        return;
    }

    dispDiff(ex);
}

void SvnActions::dispDiff(const QByteArray &ex)
{
    QString what = Kdesvnsettings::external_diff_display();

    if (Kdesvnsettings::use_external_diff() && (!what.contains(QLatin1String("%1")) || !what.contains(QLatin1String("%2")))) {
        const QVector<QStringRef> wlist = what.splitRef(QLatin1Char(' '));
        WatchedProcess *proc = new WatchedProcess(this);
        bool fname_used = false;

        for (auto it = wlist.begin(); it != wlist.end(); ++it) {
            if (*it == QLatin1String("%f")) {
                QTemporaryFile tfile;
                tfile.setAutoRemove(false);
                tfile.open();
                fname_used = true;
                QDataStream ds(&tfile);
                ds.writeRawData(ex, ex.size());
                *proc << tfile.fileName();
                proc->appendTempFile(tfile.fileName());
                tfile.close();
            } else {
                *proc << (*it).toString();
            }
        }
        proc->setAutoDelete(true);
        proc->setOutputChannelMode(KProcess::MergedChannels);
        connect(proc, SIGNAL(dataStderrRead(QByteArray,WatchedProcess*)),
                this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));
        connect(proc, SIGNAL(dataStdoutRead(QByteArray,WatchedProcess*)),
                this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));

        proc->start();
        if (proc->waitForStarted(-1)) {
            if (!fname_used)  {
                proc->write(ex);
                proc->closeWriteChannel();
            }
            if (m_Data->runblocked) {
                proc->waitForFinished(-1);
            }
            return;
        } else {
            emit sendNotify(i18n("Display process could not started, check command."));
        }
    }
    bool need_modal = m_Data->runblocked || QApplication::activeModalWidget() != nullptr;
    if (need_modal || !m_Data->m_DiffBrowserPtr || !m_Data->m_DiffDialog) {

        if (!need_modal && m_Data->m_DiffBrowserPtr) {
            delete m_Data->m_DiffBrowserPtr;
        }
        QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("diff_display")));
        if (!need_modal) {
            dlg->setParent(nullptr);
        }
        dlg->setWindowTitle(i18nc("@title:window", "Diff Display"));
        DiffBrowser *ptr(new DiffBrowser(dlg));
        ptr->setText(ex);
        dlg->addWidget(ptr);
        EncodingSelector_impl *enc(new EncodingSelector_impl(dlg));
        dlg->addWidget(enc);
        connect(enc, SIGNAL(TextCodecChanged(QString)),
                ptr, SLOT(slotTextCodecChanged(QString)));
        enc->setCurrentEncoding(Kdesvnsettings::locale_for_diff());
        // saveAs
        QPushButton *pbSaveAs = new QPushButton(dlg->buttonBox());
        KStandardGuiItem::assign(pbSaveAs, KStandardGuiItem::SaveAs);
        dlg->buttonBox()->addButton(pbSaveAs, QDialogButtonBox::ActionRole);
        connect(pbSaveAs, SIGNAL(clicked(bool)), ptr, SLOT(saveDiff()));

        dlg->buttonBox()->setStandardButtons(QDialogButtonBox::Close);
        dlg->addButtonBox();
        if (need_modal) {
            ptr->setFocus();
            dlg->exec();
            delete dlg;
            return;
        } else {
            m_Data->m_DiffBrowserPtr = ptr;
            m_Data->m_DiffDialog = dlg;
        }
    } else {
        m_Data->m_DiffBrowserPtr->setText(ex);
        m_Data->m_DiffBrowserPtr->setFocus();
    }
    if (m_Data->m_DiffDialog) {
        m_Data->m_DiffDialog->show();
        m_Data->m_DiffDialog->raise();
    }
}


/*!
    \fn SvnActions::makeUpdate(const QString&what,const svn::Revision&rev,bool recurse)
 */
void SvnActions::makeUpdate(const svn::Targets &targets, const svn::Revision &rev, svn::Depth depth)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    svn::Revisions ret;
    stopCheckUpdateThread();
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Making update"), i18n("Making update - hit Cancel for abort"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        svn::UpdateParameter _params;
        m_Data->m_SvnContextListener->cleanUpdatedItems();
        _params.targets(targets).revision(rev).depth(depth).ignore_externals(false).allow_unversioned(false).sticky_depth(true);
        ret = m_Data->m_Svnclient->update(_params);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    removeFromUpdateCache(m_Data->m_SvnContextListener->updatedItems(), true);
    //removeFromUpdateCache(what,depth==svn::DepthFiles);
    EMIT_REFRESH;
    EMIT_FINISHED;
    m_Data->clearCaches();
}

/*!
    \fn SvnActions::slotUpdateHeadRec()
 */
void SvnActions::slotUpdateHeadRec()
{
    prepareUpdate(false);
}


/*!
    \fn SvnActions::prepareUpdate(bool ask)
 */
void SvnActions::prepareUpdate(bool ask)
{
    if (!m_Data->m_ParentList || !m_Data->m_ParentList->isWorkingCopy()) {
        return;
    }
    const SvnItemList k = m_Data->m_ParentList->SelectionList();

    svn::Paths what;
    if (k.isEmpty()) {
        what.append(svn::Path(m_Data->m_ParentList->baseUri()));
    } else {
        what.reserve(k.size());
        Q_FOREACH(const SvnItem *item, k) {
            what.append(svn::Path(item->fullName()));
        }
    }
    svn::Revision r(svn::Revision::HEAD);
    if (ask) {
        Rangeinput_impl::revision_range range;
        if (!Rangeinput_impl::getRevisionRange(range, true, true)) {
            return;
        }
        r = range.first;
    }
    makeUpdate(svn::Targets(what), r, svn::DepthUnknown);
}


/*!
    \fn SvnActions::slotUpdateTo()
 */
void SvnActions::slotUpdateTo()
{
    prepareUpdate(true);
}


/*!
    \fn SvnActions::slotAdd()
 */
void SvnActions::slotAdd()
{
    makeAdd(false);
}

void SvnActions::slotAddRec()
{
    makeAdd(true);
}

void SvnActions::makeAdd(bool rec)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    if (!m_Data->m_ParentList) {
        return;
    }
    const SvnItemList lst = m_Data->m_ParentList->SelectionList();
    if (lst.isEmpty()) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("Which files or directories should I add?"));
        return;
    }
    svn::Paths items;
    items.reserve(lst.size());
    Q_FOREACH(const SvnItem *cur, lst) {
        if (cur->isVersioned()) {
            KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("<center>The entry<br/>%1<br/>is versioned - break.</center>",
                                                                        cur->fullName()));
            return;
        }
        items.push_back(svn::Path(cur->fullName()));
    }
    addItems(items, (rec ? svn::DepthInfinity : svn::DepthEmpty));
    emit sigRefreshCurrent(nullptr);
}

bool SvnActions::addItems(const svn::Paths &items, svn::Depth depth)
{
    try {
        svn::Paths::const_iterator piter;
        for (piter = items.begin(); piter != items.end(); ++piter) {
            m_Data->m_Svnclient->add((*piter), depth);
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

bool SvnActions::makeDelete(const QStringList &w)
{
    KMessageBox::ButtonCode answer = KMessageBox::questionYesNoList(nullptr,
                                                                    i18n("Really delete these entries?"),
                                                                    w,
                                                                    i18n("Delete from repository"));
    if (answer != KMessageBox::Yes) {
        return false;
    }
    return makeDelete(svn::Targets::fromStringList(w));
}

/*!
    \fn SvnActions::makeDelete()
 */
bool SvnActions::makeDelete(const svn::Targets &target, bool keep_local, bool force)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    try {
        m_Data->m_Svnclient->remove(target, force, keep_local);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_FINISHED;
    return true;
}

void SvnActions::slotCheckout()
{
    CheckoutExport(QUrl(), false);
}

void SvnActions::slotExport()
{
    CheckoutExport(QUrl(), true);
}

void SvnActions::slotCheckoutCurrent()
{
    CheckoutExportCurrent(false);
}

void SvnActions::slotExportCurrent()
{
    CheckoutExportCurrent(true);
}

void SvnActions::CheckoutExport(const QUrl &what, bool _exp, bool urlisTarget)
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("checkout_export_dialog")));
    CheckoutInfo_impl *ptr(new CheckoutInfo_impl(dlg));
    dlg->setWindowTitle(_exp ? i18nc("@title:window", "Export a Repository") : i18nc("@title:window", "Checkout a Repository"));
    dlg->setWithCancelButton();

    if (!what.isEmpty()) {
        if (!urlisTarget) {
            ptr->setStartUrl(what);
        } else {
            ptr->setTargetUrl(what);
        }
    }
    ptr->hideIgnoreKeywords(!_exp);
    ptr->hideOverwrite(!_exp);
    dlg->addWidget(ptr);
    if (dlg->exec() == QDialog::Accepted) {
        svn::Revision r = ptr->toRevision();
        bool openit = ptr->openAfterJob();
        bool ignoreExternal = ptr->ignoreExternals();
        if (!ptr->reposURL().isValid()) {
            KMessageBox::error(QApplication::activeModalWidget(), i18n("Invalid url given!"),
                               _exp ? i18n("Export repository") : i18n("Checkout a repository"));
            delete dlg;
            return;
        }
        // svn::Path should not take a QString but a QByteArray ...
        const QString rUrl(QString::fromUtf8(ptr->reposURL().toEncoded()));
        makeCheckout(rUrl,
                     ptr->targetDir(), r, r,
                     ptr->getDepth(),
                     _exp,
                     openit,
                     ignoreExternal,
                     ptr->overwrite(),
                     ptr->ignoreKeywords(),
                     nullptr);
    }
    delete dlg;
}

void SvnActions::CheckoutExportCurrent(bool _exp)
{
    // checkout export only on repo, not wc
    if (!m_Data->m_ParentList || m_Data->m_ParentList->isWorkingCopy()) {
        return;
    }
    SvnItem *k = m_Data->m_ParentList->Selected();
    if (k && !k->isDir()) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(), _exp ? i18n("Exporting a file?") : i18n("Checking out a file?"));
        return;
    }
    QUrl what;
    if (!k) {
        what = QUrl(m_Data->m_ParentList->baseUri());
    } else {
        what = QUrl(k->fullName());
    }
    // what is always remote, so QUrl(what) is fine
    CheckoutExport(QUrl(what), _exp);
}

bool SvnActions::makeCheckout(const QString &rUrl, const QString &tPath, const svn::Revision &r, const svn::Revision &_peg,
                              svn::Depth depth,
                              // kind of operation
                              bool _exp,
                              // open after job
                              bool openIt,
                              // ignore externals
                              bool ignoreExternal,
                              // overwrite/force not versioned items
                              bool overwrite,
                              // do not replace svn:keywords on export
                              bool ignoreKeywords,
                              QWidget *_p
                             )
{
    QString fUrl = rUrl;
    while (fUrl.endsWith(QLatin1Char('/'))) {
        fUrl.chop(1);
    }
    // can only be a local target dir
    svn::Path p(tPath);
    svn::Revision peg = _peg;
    if (r != svn::Revision::BASE && r != svn::Revision::WORKING && _peg == svn::Revision::UNDEFINED) {
        peg = r;
    }
    if (!_exp || !m_Data->m_CurrentContext) {
        reInitClient();
    }
    svn::CheckoutParameter cparams;
    cparams.moduleName(fUrl).destination(p).revision(r).peg(peg).depth(depth).ignoreExternals(ignoreExternal).overWrite(overwrite).ignoreKeywords(ignoreKeywords);

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, _p ? _p : m_Data->m_ParentList->realWidget(),
                     _exp ? i18nc("@title:window", "Export") : i18nc("@title:window", "Checkout"), _exp ? i18n("Exporting") : i18n("Checking out"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        if (_exp) {
            /// @todo setup parameter for export operation
            m_Data->m_Svnclient->doExport(cparams.nativeEol(QString()));
        } else {
            m_Data->m_Svnclient->checkout(cparams);
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    if (openIt) {
        const QUrl url(QUrl::fromLocalFile(tPath));
        if (!_exp) {
            emit sigGotourl(url);
        } else {
            QDesktopServices::openUrl(url);
        }
    }
    EMIT_FINISHED;

    return true;
}

void SvnActions::slotRevert()
{
    if (!m_Data->m_ParentList || !m_Data->m_ParentList->isWorkingCopy()) {
        return;
    }
    const SvnItemList lst = m_Data->m_ParentList->SelectionList();
    QStringList displist;
    if (!lst.isEmpty()) {
        displist.reserve(lst.size());
        Q_FOREACH(const SvnItem *cur, lst) {
            if (!cur->isVersioned()) {
                KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("<center>The entry<br/>%1<br/>is not versioned - break.</center>", cur->fullName()));
                return;
            }
            displist.append(cur->fullName());
        }
    } else {
        displist.push_back(m_Data->m_ParentList->baseUri());
    }
    slotRevertItems(displist);
    EMIT_REFRESH;
}

void SvnActions::slotRevertItems(const QStringList &displist)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    if (displist.isEmpty()) {
        return;
    }

    QPointer<RevertForm> dlg(new RevertForm(displist, QApplication::activeModalWidget()));
    if (dlg->exec() != QDialog::Accepted) {
        delete dlg;
        return;
    }
    const svn::Depth depth = dlg->getDepth();
    delete dlg;

    const svn::Targets target(svn::Targets::fromStringList(displist));
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Revert"), i18n("Reverting items"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->revert(target, depth);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    // remove them from cache
    for (size_t j = 0; j < target.size(); ++j) {
        m_Data->m_Cache.deleteKey(target[j].path(), depth != svn::DepthInfinity);
    }
    emit sigItemsReverted(displist);
    EMIT_FINISHED;
}

bool SvnActions::makeSwitch(const QUrl &rUrl,
                            const QString &tPath,
                            const svn::Revision &r,
                            svn::Depth depth,
                            const svn::Revision &peg,
                            bool stickydepth,
                            bool ignore_externals,
                            bool allow_unversioned)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    svn::Path p(tPath);
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Switch URL"), i18n("Switching URL"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->doSwitch(p, svn::Url(rUrl), r, depth, peg, stickydepth, ignore_externals, allow_unversioned);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    m_Data->clearCaches();
    EMIT_FINISHED;
    return true;
}

bool SvnActions::makeRelocate(const QUrl &fUrl, const QUrl &tUrl, const QString &path, bool recursive, bool ignore_externals)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    svn::Path p(path);
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Relocate Repository"), i18n("Relocate repository to new URL"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->relocate(p, svn::Url(fUrl), svn::Url(tUrl), recursive, ignore_externals);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    m_Data->clearCaches();
    EMIT_FINISHED;
    return true;
}

void SvnActions::slotSwitch()
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    if (!m_Data->m_ParentList || !m_Data->m_ParentList->isWorkingCopy()) {
        return;
    }

    const SvnItemList lst = m_Data->m_ParentList->SelectionList();
    if (lst.count() > 1) {
        KMessageBox::error(nullptr, i18n("Can only switch one item at time"));
        return;
    }

    SvnItem *k = m_Data->m_ParentList->SelectedOrMain();
    if (!k) {
        KMessageBox::error(nullptr, i18n("Error getting entry to switch"));
        return;
    }
    const QUrl what = k->Url();
    if (makeSwitch(k->fullName(), what)) {
        emit reinitItem(k);
    }
}

bool SvnActions::makeSwitch(const QString &path, const QUrl &what)
{
    QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("switch_url_dlg")));
    CheckoutInfo_impl *ptr(new CheckoutInfo_impl(dlg));
    dlg->setWindowTitle(i18nc("@title:window", "Switch URL"));
    dlg->setWithCancelButton();
    ptr->setStartUrl(what);
    ptr->disableAppend(true);
    ptr->disableTargetDir(true);
    ptr->disableOpen(true);
    dlg->addWidget(ptr);
    bool done = false;
    if (dlg->exec() == QDialog::Accepted) {
        if (!ptr->reposURL().isValid()) {
            KMessageBox::error(QApplication::activeModalWidget(), i18n("Invalid url given!"),
                               i18n("Switch URL"));
            delete dlg;
            return false;
        }
        svn::Revision r = ptr->toRevision();
        done = makeSwitch(ptr->reposURL(), path, r, ptr->getDepth(), r, true, ptr->ignoreExternals(), ptr->overwrite());
    }
    delete dlg;
    return done;
}

bool SvnActions::makeCleanup(const QString &path)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Cleanup"), i18n("Cleaning up folder"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->cleanup(svn::Path(path));
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

void SvnActions::slotResolved(const QString &path)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Resolve"), i18n("Marking resolved"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->resolve(svn::Path(path), svn::DepthEmpty);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    m_Data->m_conflictCache.deleteKey(path, false);
    emit sigRefreshItem(path);
}

void SvnActions::slotResolve(const QString &p)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    const QString eresolv = Kdesvnsettings::conflict_resolver();
    const QVector<QStringRef> wlist = eresolv.splitRef(QLatin1Char(' '));
    if (wlist.isEmpty()) {
        return;
    }
    svn::InfoEntry i1;
    if (!singleInfo(p, svn::Revision::UNDEFINED, i1)) {
        return;
    }
    QFileInfo fi(p);
    QString base;
    if (fi.isRelative()) {
        base = fi.absolutePath() + QLatin1Char('/');
    }
    if (i1.conflicts().isEmpty())
    {
        emit sendNotify(i18n("Could not retrieve conflict information - giving up."));
        return;
    }

    WatchedProcess *proc = new WatchedProcess(this);
    for (auto it = wlist.begin(); it != wlist.end(); ++it) {
        if (*it == QLatin1String("%o") || *it == QLatin1String("%l")) {
            *proc << i1.conflicts()[0]->baseFile();
        } else if (*it == QLatin1String("%m") || *it == QLatin1String("%w")) {
            *proc << i1.conflicts()[0]->myFile();
        } else if (*it == QLatin1String("%n") || *it == QLatin1String("%r")) {
            *proc << i1.conflicts()[0]->theirFile();
        } else if (*it == QLatin1String("%t")) {
            *proc << p;
        } else {
            *proc << (*it).toString();
        }
    }
    proc->setAutoDelete(true);
    proc->setOutputChannelMode(KProcess::MergedChannels);
    connect(proc, SIGNAL(dataStderrRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));
    connect(proc, SIGNAL(dataStdoutRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));

    proc->start();
    if (!proc->waitForStarted(-1)) {
        emit sendNotify(i18n("Resolve-process could not started, check command."));
    }
}

void SvnActions::slotImport(const QString &path, const QUrl &target, const QString &message, svn::Depth depth,
                            bool noIgnore, bool noUnknown)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Import"), i18n("Importing items"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->import(svn::Path(path), svn::Url(target), message, depth, noIgnore, noUnknown);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
}

void SvnActions::slotMergeExternal(const QString &_src1, const QString &_src2, const QString &_target,
                                   const svn::Revision &rev1, const svn::Revision &rev2, const svn::Revision &_peg, bool rec)
{
    Q_UNUSED(_peg);
    QTemporaryDir tdir1;
    tdir1.setAutoRemove(true);
    QString src1 = _src1;
    QString src2 = _src2;
    QString target = _target;
    bool singleMerge = false;

    if (rev1 == rev2 && (src2.isEmpty() || src1 == src2)) {
        singleMerge = true;
    }
    if (src1.isEmpty()) {
        emit clientException(i18n("Nothing to merge."));
        return;
    }
    if (target.isEmpty()) {
        emit clientException(i18n("No destination to merge."));
        return;
    }

    QFileInfo f1(src1);
    QFileInfo f2(src2);
    bool isDir = true;

    svn::InfoEntry i1, i2;

    if (!singleInfo(src1, rev1, i1)) {
        return;
    }
    isDir = i1.isDir();
    if (!singleMerge && src1 != src2) {
        if (!singleInfo(src2, rev2, i2)) {
            return;
        }
        if (i2.isDir() != isDir) {
            emit clientException(i18n("Both sources must be same type."));
            return;
        }
    }

    QFileInfo ti(target);
    if (ti.isDir() != isDir) {
        emit clientException(i18n("Target for merge must same type like sources."));
        return;
    }

    QString s1 = f1.fileName() + QLatin1Char('-') + rev1.toString();
    QString s2 = f2.fileName() + QLatin1Char('-') + rev2.toString();
    QString first, second;
    if (rev1 != svn::Revision::WORKING) {
        first = tdir1.path() + QLatin1Char('/') + s1;
    } else {
        first = src1;
    }
    if (!singleMerge) {
        if (rev2 != svn::Revision::WORKING) {
            second = tdir1.path() + QLatin1Char('/') + s2;
        } else {
            second = src2;
        }
    } else {
        // only two-way  merge
        second.clear();
    }
    if (second == first) {
        KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("Both entries seems to be the same, will not do a merge."));
        return;
    }

    if (rev1 != svn::Revision::WORKING) {
        if (isDir) {
            if (!makeCheckout(src1, first, rev1, svn::Revision::UNDEFINED,
                              rec ? svn::DepthInfinity : svn::DepthFiles,
                              true, false, false, false, false, nullptr)) {
                return;
            }
        } else {
            if (!get(src1, first, rev1, svn::Revision::UNDEFINED, m_Data->m_ParentList->realWidget())) {
                return;
            }
        }
    }

    if (!singleMerge) {
        if (rev2 != svn::Revision::WORKING) {
            if (isDir) {
                if (!makeCheckout(src2, second, rev2, svn::Revision::UNDEFINED,
                                  rec ? svn::DepthInfinity : svn::DepthFiles,
                                  true, false, false, false, false, nullptr)) {
                    return;
                }
            } else {
                if (!get(src2, second, rev2, svn::Revision::UNDEFINED, m_Data->m_ParentList->realWidget())) {
                    return;
                }
            }
        }
    }
    const QString edisp = Kdesvnsettings::external_merge_program();
    const QVector<QStringRef> wlist = edisp.splitRef(QLatin1Char(' '));
    WatchedProcess *proc = new WatchedProcess(this);
    for (auto it = wlist.begin(); it != wlist.end(); ++it) {
        if (*it == QLatin1String("%s1")) {
            *proc << first;
        } else if (*it == QLatin1String("%s2")) {
            if (!second.isEmpty()) {
                *proc << second;
            }
        } else if (*it == QLatin1String("%t")) {
            *proc << target;
        } else {
            *proc << (*it).toString();
        }
    }
    tdir1.setAutoRemove(false);
    proc->setAutoDelete(true);
    proc->appendTempDir(tdir1.path());
    proc->setOutputChannelMode(KProcess::MergedChannels);
    connect(proc, SIGNAL(dataStderrRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));
    connect(proc, SIGNAL(dataStdoutRead(QByteArray,WatchedProcess*)),
            this, SLOT(slotProcessDataRead(QByteArray,WatchedProcess*)));

    proc->start();
    if (proc->waitForStarted(-1)) {
        if (m_Data->runblocked) {
            proc->waitForFinished(-1);
        }
    } else {
        emit sendNotify(i18n("Merge process could not started, check command."));
    }
}

void SvnActions::slotMergeWcRevisions(const QString &_entry, const svn::Revision &rev1,
                                      const svn::Revision &rev2,
                                      bool rec, bool ancestry, bool forceIt, bool dry, bool allow_mixed_rev)
{
    slotMerge(_entry, _entry, _entry, rev1, rev2, svn::Revision::UNDEFINED, rec, ancestry, forceIt, dry, false, false, allow_mixed_rev);
}

void SvnActions::slotMerge(const QString &src1, const QString &src2, const QString &target,
                           const svn::Revision &rev1, const svn::Revision &rev2, const svn::Revision &_peg,
                           bool rec, bool ancestry, bool forceIt, bool dry, bool recordOnly, bool reintegrate, bool allow_mixed_rev)
{
    Q_UNUSED(_peg);
    if (!m_Data->m_CurrentContext) {
        return;
    }

    svn::Revision peg = svn::Revision::HEAD;
    svn::Revision tpeg;
    svn::RevisionRanges ranges;
    svn::Path p1;
    try {
        svn::Path::parsePeg(src1, p1, tpeg);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    if (tpeg != svn::Revision::UNDEFINED) {
        peg = tpeg;
    }
    svn::Path p2(src2);

    bool pegged_merge = false;

    // build merge Parameters
    svn::MergeParameter _merge_parameter;
    ranges.append(svn::RevisionRange(rev1, rev2));
    _merge_parameter.revisions(ranges).path1(p1).path2(p2).depth(rec ? svn::DepthInfinity : svn::DepthFiles).notice_ancestry(ancestry).force(forceIt)
    .dry_run(dry).record_only(recordOnly).reintegrate(reintegrate).allow_mixed_rev(allow_mixed_rev)
    .localPath(svn::Path(target)).merge_options(svn::StringArray());

    if (!reintegrate && (!p2.isSet() || src1 == src2)) {
        // pegged merge
        pegged_merge = true;
        if (peg == svn::Revision::UNDEFINED) {
            if (p1.isUrl()) {
                peg = rev2;
            } else {
                peg = svn::Revision::WORKING;
            }
        }
        _merge_parameter.peg(peg);
    }

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Merge"), i18n("Merging items"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        if (pegged_merge) {
            m_Data->m_Svnclient->merge_peg(_merge_parameter);
        } else {
            m_Data->m_Svnclient->merge(_merge_parameter);
        }
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    m_Data->clearCaches();
}

/*!
    \fn SvnActions::slotCopyMove(bool,const QString&,const QString&)
 */
bool SvnActions::makeMove(const QString &Old, const QString &New)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    svn::CopyParameter params(Old, New);
    svn::Revision nnum;

    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Move"), i18n("Moving/Rename item"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        nnum = m_Data->m_Svnclient->move(params.asChild(false).makeParent(false));
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    if (nnum != svn::Revision::UNDEFINED) {
        emit sendNotify(i18n("Committed revision %1.", nnum.toString()));
    }
    EMIT_REFRESH;
    return true;
}

bool SvnActions::makeMove(const QList<QUrl> &Old, const QString &New)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Move"), i18n("Moving entries"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        const svn::Path pNew(New);
        // either both are local paths -> move in wc, or both are urls -> move in repository
        const svn::Targets t(svn::Targets::fromUrlList(Old, pNew.isUrl() ? svn::Targets::UrlConversion::KeepUrl : svn::Targets::UrlConversion::PreferLocalPath));
        m_Data->m_Svnclient->move(svn::CopyParameter(t, pNew).asChild(true).makeParent(false));
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

bool SvnActions::makeCopy(const QString &Old, const QString &New, const svn::Revision &rev)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Copy / Move"), i18n("Copy or Moving entries"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        m_Data->m_Svnclient->copy(svn::Path(Old), rev, svn::Path(New));
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    EMIT_REFRESH;
    return true;
}

bool SvnActions::makeCopy(const QList<QUrl> &Old, const QString &New, const svn::Revision &rev)
{
    try {
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Copy / Move"), i18n("Copy or Moving entries"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        const svn::Path pNew(New);
        // either both are local paths -> copy in wc, or both are urls -> copy in repository
        const svn::Targets t(svn::Targets::fromUrlList(Old, pNew.isUrl() ? svn::Targets::UrlConversion::KeepUrl : svn::Targets::UrlConversion::PreferLocalPath));
        m_Data->m_Svnclient->copy(svn::CopyParameter(t, pNew).srcRevision(rev).pegRevision(rev).asChild(true));
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

/*!
    \fn SvnActions::makeLock(const QStringList&)
 */
void SvnActions::makeLock(const QStringList &what, const QString &_msg, bool breakit)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    try {
        m_Data->m_Svnclient->lock(svn::Targets::fromStringList(what), _msg, breakit);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
}


/*!
    \fn SvnActions::makeUnlock(const QStringList&)
 */
void SvnActions::makeUnlock(const QStringList &what, bool breakit)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    try {
        m_Data->m_Svnclient->unlock(svn::Targets::fromStringList(what), breakit);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return;
    }
    for (long i = 0; i < what.count(); ++i) {
        m_Data->m_repoLockCache.deleteKey(what[i], true);
    }
//    m_Data->m_repoLockCache.dump_tree();
}


/*!
    \fn SvnActions::makeStatus(const QString&what, svn::StatusEntries&dlist)
 */
bool SvnActions::makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, bool rec, bool all)
{
    bool display_ignores = Kdesvnsettings::display_ignored_files();
    return makeStatus(what, dlist, where, rec, all, display_ignores);
}

bool SvnActions::makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, bool rec, bool all, bool display_ignores, bool updates)
{
    svn::Depth _d = rec ? svn::DepthInfinity : svn::DepthImmediates;
    return makeStatus(what, dlist, where, _d, all, display_ignores, updates);
}

bool SvnActions::makeStatus(const QString &what, svn::StatusEntries &dlist, const svn::Revision &where, svn::Depth _d, bool all, bool display_ignores, bool updates)
{
    bool disp_remote_details = Kdesvnsettings::details_on_remote_listing();
    try {
#ifdef DEBUG_TIMER
        QTime _counttime;
        _counttime.start();
#endif
        svn::StatusParameter params(what);
        StopDlg sdlg(m_Data->m_SvnContextListener, m_Data->m_ParentList->realWidget(),
                     i18nc("@title:window", "Status / List"), i18n("Creating list / check status"));
        connect(this, SIGNAL(sigExtraLogMsg(QString)), &sdlg, SLOT(slotExtraMessage(QString)));
        //                                      rec all  up     noign
        dlist = m_Data->m_Svnclient->status(params.depth(_d).all(all).update(updates).noIgnore(display_ignores).revision(where).detailedRemote(disp_remote_details).ignoreExternals(false));
#ifdef DEBUG_TIMER
        qCDebug(KDESVN_LOG) << "Time for getting status: " << _counttime.elapsed();
#endif

    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    return true;
}

void SvnActions::checkAddItems(const QString &path, bool print_error_box)
{
    svn::StatusEntries dlist;
    svn::StatusEntries rlist;
    QStringList displist;
    svn::Revision where = svn::Revision::HEAD;
    if (!makeStatus(path, dlist, where, true, true, false, false)) {
        return;
    }
    for (int i = 0; i < dlist.size(); ++i) {
        const svn::StatusPtr &ptr = dlist.at(i);
        if (!ptr->isVersioned()) {
            rlist.append(ptr);
            displist.append(ptr->path());
        }
    }
    if (rlist.isEmpty()) {
        if (print_error_box) {
            KMessageBox::error(m_Data->m_ParentList->realWidget(), i18n("No unversioned items found."));
        }
    } else {
        QPointer<KSvnSimpleOkDialog> dlg(new KSvnSimpleOkDialog(QStringLiteral("add_items_dlg")));
        dlg->setWindowTitle(i18nc("@title:window", "Add Unversioned Items"));
        dlg->setWithCancelButton();
        QTreeWidget *ptr(new QTreeWidget(dlg));
        ptr->headerItem()->setText(0, i18n("Item"));
        for (int j = 0; j < displist.size(); ++j) {
            QTreeWidgetItem *n = new QTreeWidgetItem(ptr);
            n->setText(0, displist[j]);
            n->setCheckState(0, Qt::Checked);
        }
        ptr->resizeColumnToContents(0);
        dlg->addWidget(ptr);
        if (dlg->exec() == QDialog::Accepted) {
            QTreeWidgetItemIterator it(ptr);
            displist.clear();
            while (*it) {
                QTreeWidgetItem *t = (*it);
                if (t->checkState(0) == Qt::Checked) {
                    displist.append(t->text(0));
                }
                ++it;
            }
            if (!displist.isEmpty()) {
                addItems(svn::Targets::fromStringList(displist).targets(), svn::DepthEmpty);
            }
        }
        delete dlg;
    }
}

void SvnActions::stopCheckModifiedThread()
{
    if (m_CThread) {
        m_CThread->cancelMe();
        if (!m_CThread->wait(MAX_THREAD_WAITTIME)) {
            m_CThread->terminate();
            m_CThread->wait(MAX_THREAD_WAITTIME);
        }
        delete m_CThread;
        m_CThread = nullptr;
    }
}

void SvnActions::stopCheckUpdateThread()
{
    if (m_UThread) {
        m_UThread->cancelMe();
        if (!m_UThread->wait(MAX_THREAD_WAITTIME)) {
            m_UThread->terminate();
            m_UThread->wait(MAX_THREAD_WAITTIME);
        }
        delete m_UThread;
        m_UThread = nullptr;
    }
}

void SvnActions::stopFillCache()
{
    if (m_FCThread) {
        m_FCThread->cancelMe();
        if (!m_FCThread->wait(MAX_THREAD_WAITTIME)) {
            m_FCThread->terminate();
            m_FCThread->wait(MAX_THREAD_WAITTIME);
        }
        delete m_FCThread;
        m_FCThread = nullptr;
        emit sigThreadsChanged();
        emit sigCacheStatus(-1, -1);
    }
}

void SvnActions::stopMain()
{
    if (m_Data->m_CurrentContext) {
        m_Data->m_SvnContextListener->setCanceled(true);
        sleep(1);
        m_Data->m_SvnContextListener->contextCancel();
    }
}

void SvnActions::killallThreads()
{
    stopMain();
    stopCheckModifiedThread();
    stopCheckUpdateThread();
    stopFillCache();
}

bool SvnActions::createModifiedCache(const QString &what)
{
    stopCheckModifiedThread();
    m_CThread = new CheckModifiedThread(this, what, false);
    connect(m_CThread, SIGNAL(checkModifiedFinished()),
            this, SLOT(checkModifiedThread()));
    m_CThread->start();
    return true;
}

void SvnActions::checkModifiedThread()
{
    if (!m_CThread) {
        return;
    }
    if (m_CThread->isRunning()) {
        QTimer::singleShot(2, this, SLOT(checkModifiedThread()));
        return;
    }
    m_Data->m_Cache.clear();
    m_Data->m_conflictCache.clear();
    const svn::StatusEntries &sEntries = m_CThread->getList();
    for (int i = 0; i < sEntries.size(); ++i) {
        const svn::StatusPtr ptr = sEntries.at(i);
        if (ptr->isRealVersioned() && (
                    ptr->nodeStatus() == svn_wc_status_modified ||
                    ptr->nodeStatus() == svn_wc_status_added ||
                    ptr->nodeStatus() == svn_wc_status_deleted ||
                    ptr->nodeStatus() == svn_wc_status_replaced ||
                    ptr->nodeStatus() == svn_wc_status_modified
                )) {
            m_Data->m_Cache.insertKey(ptr, ptr->path());
        } else if (ptr->nodeStatus() == svn_wc_status_conflicted) {
            m_Data->m_conflictCache.insertKey(ptr, ptr->path());
        }
        emit sigRefreshItem(ptr->path());
    }
    sigExtraStatusMessage(i18np("Found %1 modified item", "Found %1 modified items", sEntries.size()));
    delete m_CThread;
    m_CThread = nullptr;
    emit sigCacheDataChanged();
}

void SvnActions::checkUpdateThread()
{
    if (!m_UThread || m_UThread->isRunning()) {
        if (m_UThread) {
            QTimer::singleShot(2, this, SLOT(checkUpdateThread()));
        }
        return;
    }
    bool newer = false;
    const svn::StatusEntries &sEntries = m_UThread->getList();
    for (int i = 0; i < sEntries.size(); ++i) {
        const svn::StatusPtr ptr = sEntries.at(i);
        if (ptr->validReposStatus()) {
            m_Data->m_UpdateCache.insertKey(ptr, ptr->path());
            if (!(ptr->validLocalStatus())) {
                newer = true;
            }
        }
        if (ptr->isLocked() &&
                !(ptr->entry().lockEntry().Locked())) {
            m_Data->m_repoLockCache.insertKey(ptr, ptr->path());
        }
        emit sigRefreshItem(ptr->path());
    }
    emit sigExtraStatusMessage(i18n("Checking for updates finished"));
    if (newer) {
        emit sigExtraStatusMessage(i18n("There are new items in repository"));
    }
    delete m_UThread;
    m_UThread = nullptr;
    emit sigCacheDataChanged();
}

void SvnActions::getaddedItems(const QString &path, svn::StatusEntries &target)
{
    helpers::ValidRemoteOnly vro;
    m_Data->m_UpdateCache.listsubs_if(path, vro);
    target = vro.liste();
}

bool SvnActions::checkUpdatesRunning()
{
    return m_UThread && m_UThread->isRunning();
}

void SvnActions::addModifiedCache(const svn::StatusPtr &what)
{
    if (what->nodeStatus() == svn_wc_status_conflicted) {
        m_Data->m_conflictCache.insertKey(what, what->path());
        emit sigRefreshItem(what->path());
    } else {
        m_Data->m_Cache.insertKey(what, what->path());
    }
}

void SvnActions::deleteFromModifiedCache(const QString &what)
{
    m_Data->m_Cache.deleteKey(what, true);
    m_Data->m_conflictCache.deleteKey(what, true);
    //m_Data->m_Cache.dump_tree();
    emit sigRefreshItem(what);
}

bool SvnActions::checkModifiedCache(const QString &path) const
{
    return m_Data->m_Cache.find(path);
}

bool SvnActions::checkReposLockCache(const QString &path) const
{
    return m_Data->m_repoLockCache.findSingleValid(path, false);
}

bool SvnActions::checkReposLockCache(const QString &path, svn::StatusPtr &t) const
{
    /// @todo create a method where svn::Status* will be a parameter so no copy is needed but just reading content
    return m_Data->m_repoLockCache.findSingleValid(path, t);
}

bool SvnActions::checkConflictedCache(const QString &path) const
{
    return m_Data->m_conflictCache.find(path);
}

void SvnActions::startFillCache(const QString &path, bool startup)
{
#ifdef DEBUG_TIMER
    QTime _counttime;
    _counttime.start();
#endif
    stopFillCache();
#ifdef DEBUG_TIMER
    qCDebug(KDESVN_LOG) << "Stopped cache " << _counttime.elapsed();
    _counttime.restart();
#endif
    if (!doNetworking()) {
        emit sendNotify(i18n("Not filling log cache because networking is disabled"));
        return;
    }

    m_FCThread = new FillCacheThread(this, path, startup);
    connect(m_FCThread, SIGNAL(fillCacheStatus(qlonglong,qlonglong)),
            this, SIGNAL(sigCacheStatus(qlonglong,qlonglong)));
    connect(m_FCThread, SIGNAL(fillCacheFinished()),
            this, SLOT(stopFillCache()));
    m_FCThread->start();
}

bool SvnActions::doNetworking()
{
    // if networking is allowd we don't need extra checks, second is just for avoiding segfaults
    if (Kdesvnsettings::network_on() || !m_Data->m_ParentList) {
        return true;
    }
    bool is_url = false;
    if (m_Data->m_ParentList->isNetworked()) {
        // if called http:// etc.pp.
        is_url = true;
    } else if (m_Data->m_ParentList->baseUri().startsWith(QLatin1Char('/'))) {
        // if opened a working copy we must check if it points to a networking repository
        svn::InfoEntry e;
        if (!singleInfo(m_Data->m_ParentList->baseUri(), svn::Revision::UNDEFINED, e)) {
            return false;
        }
        is_url = !e.reposRoot().isLocalFile();
    }
    return !is_url;
}

/*!
    \fn SvnActions::createUpdateCache(const QString&what)
 */
bool SvnActions::createUpdateCache(const QString &what)
{
    clearUpdateCache();
    m_Data->m_repoLockCache.clear();
    stopCheckUpdateThread();
    if (!doNetworking()) {
        emit sigExtraStatusMessage(i18n("Not checking for updates because networking is disabled"));
        return false;
    }
    m_UThread = new CheckModifiedThread(this, what, true);
    connect(m_UThread, SIGNAL(checkModifiedFinished()),
            this, SLOT(checkUpdateThread()));
    m_UThread->start();
    emit sigExtraStatusMessage(i18n("Checking for updates started in background"));
    return true;
}

bool SvnActions::checkUpdateCache(const QString &path)const
{
    return m_Data->m_UpdateCache.find(path);
}

void SvnActions::removeFromUpdateCache(const QStringList &what, bool exact_only)
{
    for (int i = 0; i < what.size(); ++i) {
        m_Data->m_UpdateCache.deleteKey(what.at(i), exact_only);
    }
}

bool SvnActions::isUpdated(const QString &path)const
{
    svn::StatusPtr d;
    return getUpdated(path, d);
}

bool SvnActions::getUpdated(const QString &path, svn::StatusPtr &d)const
{
    return m_Data->m_UpdateCache.findSingleValid(path, d);
}

void SvnActions::clearUpdateCache()
{
    m_Data->m_UpdateCache.clear();
}

bool SvnActions::makeIgnoreEntry(const svn::Path &item, const QStringList &ignorePattern, bool unignore)
{
    svn::Revision r(svn::Revision::UNDEFINED);

    QPair<qlonglong, svn::PathPropertiesMapList> pmp;
    try {
        pmp = m_Data->m_Svnclient->propget(QStringLiteral("svn:ignore"), item, r, r);
    } catch (const svn::Exception &e) {
        emit clientException(e.msg());
        return false;
    }
    svn::PathPropertiesMapList pm = pmp.second;
    QString data;
    if (!pm.isEmpty()) {
        const svn::PropertiesMap &mp = pm[0].second;
        data = mp[QStringLiteral("svn:ignore")];
    }
    bool result = false;
    QStringList lst = data.split(QLatin1Char('\n'), QString::SkipEmptyParts);

    for (int _current = 0; _current < ignorePattern.size(); ++_current) {
        int it = lst.indexOf(ignorePattern[_current]);
        if (it != -1) {
            if (unignore) {
                lst.removeAt(it);
                result = true;
            }
        } else {
            if (!unignore) {
                lst.append(ignorePattern[_current]);
                result = true;
            }
        }
    }
    if (result) {
        data = lst.join(QLatin1Char('\n'));
        try {
            m_Data->m_Svnclient->propset(svn::PropertiesParameter().propertyName(QStringLiteral("svn:ignore")).propertyValue(data).path(item));
        } catch (const svn::Exception &e) {
            emit clientException(e.msg());
            return false;
        }
    }
    return result;
}

bool SvnActions::makeIgnoreEntry(SvnItem *which, bool unignore)
{
    if (!which) {
        return false;
    }
    QString parentName = which->getParentDir();
    if (parentName.isEmpty()) {
        return false;
    }
    QString name = which->shortName();
    return makeIgnoreEntry(svn::Path(parentName), QStringList(name), unignore);
}

svn::PathPropertiesMapListPtr SvnActions::propList(const QString &which, const svn::Revision &where, bool cacheOnly)
{
    svn::PathPropertiesMapListPtr pm;
    if (!which.isEmpty()) {
        QString fk = where.toString() + QLatin1Char('/') + which;
        svn::Path p(which);

        if (where != svn::Revision::WORKING) {
            m_Data->m_PropertiesCache.findSingleValid(fk, pm);
        }
        if (!pm && !cacheOnly) {
            try {
                pm = m_Data->m_Svnclient->proplist(p, where, where);
            } catch (const svn::Exception &e) {
                /* no messagebox needed */
                if (e.apr_err() != SVN_ERR_WC_NOT_DIRECTORY) {
                    sendNotify(e.msg());
                }
            }
            if (where != svn::Revision::WORKING && pm) {
                m_Data->m_PropertiesCache.insertKey(pm, fk);
            }
        }
    }
    return pm;
}

bool SvnActions::isLockNeeded(SvnItem *which, const svn::Revision &where)
{
    if (!which) {
        return false;
    }
    svn::Path p(which->fullName());

    QPair<qlonglong, svn::PathPropertiesMapList> pmp;
    try {
        pmp = m_Data->m_Svnclient->propget(QStringLiteral("svn:needs-lock"), p, where, where);
    } catch (const svn::Exception &e) {
        /* no messagebox needed */
        //emit clientException(e.msg());
        return false;
    }
    const svn::PathPropertiesMapList pm = pmp.second;
    if (!pm.isEmpty()) {
        const svn::PropertiesMap &mp = pm.at(0).second;
        if (mp.contains(QStringLiteral("svn:needs-lock"))) {
            return true;
        }
    }
    return false;
}

QString SvnActions::searchProperty(QString &Store, const QString &property, const QString &start, const svn::Revision &where, bool up)
{
    svn::Path pa(start);
    svn::InfoEntry inf;

    if (!singleInfo(start, where, inf)) {
        return QString();
    }
    while (pa.length() > 0) {
        const svn::PathPropertiesMapListPtr pm = propList(pa.path(), where, false);
        if (!pm) {
            return QString();
        }
        if (!pm->isEmpty()) {
            const svn::PropertiesMap &mp = pm->at(0).second;
            const svn::PropertiesMap::ConstIterator it = mp.find(property);
            if (it != mp.end()) {
                Store = *it;
                return pa.path();
            }
        }
        if (up) {
            pa.removeLast();
            if (pa.isUrl() && inf.reposRoot().toString().length() > pa.path().length()) {
                break;
            }

        } else {
            break;
        }
    }
    return QString();
}

bool SvnActions::makeList(const QString &url, svn::DirEntries &dlist, const svn::Revision &where, svn::Depth depth)
{
    if (!m_Data->m_CurrentContext) {
        return false;
    }
    try {
        dlist = m_Data->m_Svnclient->list(url, where, where, depth, false);
    } catch (const svn::Exception &e) {
        qCDebug(KDESVN_LOG) << "List fehler: " << e.msg();
        emit clientException(e.msg());
        return false;
    }
    return true;
}

bool SvnActions::isLocalWorkingCopy(const QString &path, QUrl &repoUrl)
{
    if (path.isEmpty()) {
        return false;
    }
    const QUrl url = helpers::KTranslateUrl::string2Uri(path);
    if (!url.isLocalFile()) {
        qCDebug(KDESVN_LOG) << "isLocalWorkingCopy no local file: " << path << " - " << url.toString();
        return false;
    }

    QString cleanpath = url.adjusted(QUrl::StripTrailingSlash|QUrl::NormalizePathSegments).path();
    qCDebug(KDESVN_LOG) << "isLocalWorkingCopy for " << cleanpath;
    repoUrl.clear();
    svn::Revision peg(svn_opt_revision_unspecified);
    svn::Revision rev(svn_opt_revision_unspecified);
    svn::InfoEntries e;
    try {
        e = m_Data->m_Svnclient->info(cleanpath, svn::DepthEmpty, rev, peg);
    } catch (const svn::Exception &e) {

        if (SVN_ERR_WC_NOT_DIRECTORY == e.apr_err()) {
            return false;
        }
        return true;
    }
    if (!e.isEmpty())
        repoUrl = e.at(0).url();
    return true;
}

void SvnActions::slotExtraLogMsg(const QString &msg)
{
    emit sigExtraLogMsg(msg);
}

void SvnActions::slotCancel(bool how)
{
    if (!m_Data->m_CurrentContext) {
        return;
    }
    m_Data->m_SvnContextListener->setCanceled(how);
}

void SvnActions::setContextData(const QString &aKey, const QString &aValue)
{
    if (aValue.isNull()) {
        QMap<QString, QString>::iterator it = m_Data->m_contextData.find(aKey);
        if (it != m_Data->m_contextData.end()) {
            m_Data->m_contextData.remove(aKey);
        }
    } else {
        m_Data->m_contextData[aKey] = aValue;
    }
}

void SvnActions::clearContextData()
{
    m_Data->m_contextData.clear();
}

QString SvnActions::getContextData(const QString &aKey)const
{
    if (m_Data->m_contextData.find(aKey) != m_Data->m_contextData.end()) {
        return m_Data->m_contextData[aKey];
    }
    return QString();
}

bool SvnActions::threadRunning(ThreadType which) const
{
    switch (which) {
    case checkupdatethread:
        return (m_UThread && m_UThread->isRunning());
    case fillcachethread:
        return (m_FCThread && m_FCThread->isRunning());
    case checkmodifiedthread:
        return (m_CThread && m_CThread->isRunning());
    }
    return false;
}
