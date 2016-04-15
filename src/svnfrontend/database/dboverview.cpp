/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   http://kdesvn.alwins-world.de/                                        *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU General Public        *
 * License along with this program (in the file GPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "dboverview.h"
#include "ui_dboverview.h"

#include "dbsettings.h"
#include "svnqt/cache/LogCache.h"
#include "svnqt/cache/ReposLog.h"
#include "svnqt/cache/DatabaseException.h"
#include "svnqt/client.h"
#include "helpers/stringhelper.h"
#include "helpers/kdesvn_debug.h"

#include <QPointer>
#include <QStringListModel>
#include <QItemSelectionModel>

#include <KMessageBox>
#include <KLocalizedString>

DbOverview::DbOverview(const svn::ClientP &aClient, QWidget *parent)
    : KSvnDialog(QLatin1String("db_overview_dlg"), parent)
    , m_clientP(aClient)
    , m_repo_model(new QStringListModel(this))
    , m_ui(new Ui::DBOverView)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Close));
    connect(m_ui->buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked(bool)),
            this, SLOT(accept()));

    enableButtons(false);

    try {
        m_repo_model->setStringList(svn::cache::LogCache::self()->cachedRepositories());
    } catch (const svn::cache::DatabaseException &e) {
        qCDebug(KDESVN_LOG) << e.msg() << endl;
    }

    m_ui->m_ReposListView->setModel(m_repo_model);
    QItemSelectionModel *_sel = m_ui->m_ReposListView->selectionModel();
    if (_sel) {
        connect(_sel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(itemActivated(QItemSelection,QItemSelection)));
    }
    connect(m_ui->m_DeleteCacheButton, SIGNAL(clicked(bool)),
            this, SLOT(deleteCacheItems()));
    connect(m_ui->m_DeleteRepositoryButton, SIGNAL(clicked(bool)),
            this, SLOT(deleteRepository()));
    connect(m_ui->m_SettingsButton, SIGNAL(clicked(bool)),
            this, SLOT(repositorySettings()));
    m_ui->m_StatisticButton->setVisible(false);
    // t.b.d
    //connect(m_ui->m_StatisticButton, SIGNAL(clicked(bool)),
    //        this, SLOT(repositoryStatistics()));
}

DbOverview::~DbOverview()
{
    delete m_ui;
}

void DbOverview::showDbOverview(const svn::ClientP &aClient, QWidget *parent)
{
//  i18n("Overview about cache database content")
    QPointer<DbOverview> dlg(new DbOverview(aClient, parent ? parent : QApplication::activeModalWidget()));
    dlg->exec();
    delete dlg;
}

void DbOverview::enableButtons(bool how)
{
    m_ui->m_DeleteCacheButton->setEnabled(how);
    m_ui->m_DeleteRepositoryButton->setEnabled(how);
    m_ui->m_SettingsButton->setEnabled(how);
    m_ui->m_StatisticButton->setEnabled(how);
}

void DbOverview::itemActivated(const QItemSelection &indexes, const QItemSelection &deindexes)
{
    Q_UNUSED(deindexes);

    enableButtons(false);
    QModelIndexList _indexes = indexes.indexes();
    if (_indexes.count() != 1) {
        qCDebug(KDESVN_LOG) << "Handle only with single selection" << endl;
        return;
    }
    genInfo(_indexes[0].data().toString());
    enableButtons(true);
}

void DbOverview::genInfo(const QString &repo)
{
    svn::cache::ReposLog rl(m_clientP, repo);
    QString msg = i18n("Log cache holds %1 log entries and consumes %2 on disk.", rl.count(), helpers::ByteToString(rl.fileSize()));
    m_ui->m_RepostatusBrowser->setText(msg);
}

QString DbOverview::selectedRepository()const
{
    const QModelIndexList _indexes = m_ui->m_ReposListView->selectionModel()->selectedIndexes();
    if (_indexes.size() != 1) {
        return QString();
    }
    return _indexes[0].data().toString();
}

void DbOverview::deleteCacheItems()
  {
    KMessageBox::ButtonCode i = KMessageBox::questionYesNo(this,
                                                           i18n("Really clean cache for repository\n%1?", selectedRepository()),
                                                           i18n("Clean repository cache"));
    if (i != KMessageBox::Yes) {
        return;
    }
    try {
        svn::cache::ReposLog rl(m_clientP, selectedRepository());
        rl.cleanLogEntries();
    } catch (const svn::cache::DatabaseException &e) {
        qCDebug(KDESVN_LOG) << e.msg();
    }
    genInfo(selectedRepository());
}

void DbOverview::deleteRepository()
{
    KMessageBox::ButtonCode i = KMessageBox::questionYesNo(this,
                                                           i18n("Really clean cache and data for repository\n%1?", selectedRepository()),
                                                           i18n("Delete repository"));
    if (i != KMessageBox::Yes) {
        return;
    }
    try {
        svn::cache::LogCache::self()->deleteRepository(selectedRepository());
        m_repo_model->setStringList(svn::cache::LogCache::self()->cachedRepositories());
    } catch (const svn::cache::DatabaseException &e) {
        qCDebug(KDESVN_LOG) << e.msg() << endl;
    }
}

void DbOverview::repositorySettings()
{
    DbSettings::showSettings(selectedRepository(), this);
}
