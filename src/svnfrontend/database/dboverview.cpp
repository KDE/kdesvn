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
#include "dbsettings.h"
#include "src/svnqt/cache/ReposConfig.h"
#include "src/svnqt/cache/LogCache.h"
#include "src/svnqt/cache/ReposLog.h"
#include "src/svnqt/cache/DatabaseException.h"
#include "src/svnfrontend/fronthelpers/createdlg.h"
#include "src/svnqt/client.h"
#include "helpers/stringhelper.h"

#include <QStringListModel>
#include <QItemSelectionModel>

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

class DbOverViewData
{

public:
    QStringListModel*repo_model;
    svn::Client*_Client;

    DbOverViewData()
    {
        repo_model = new QStringListModel();
        _Client = 0;
    }
    ~DbOverViewData()
    {
        delete repo_model;
    }
};

DbOverview::DbOverview(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    enableButtons(false);
    _data = new DbOverViewData;

    try {
        _data->repo_model->setStringList(svn::cache::LogCache::self()->cachedRepositories());
    }catch (const svn::cache::DatabaseException&e) {
        kDebug()<<e.msg()<<endl;
    }

    m_ReposListView->setModel(_data->repo_model);
    QItemSelectionModel * _sel = m_ReposListView->selectionModel();
    if (_sel) {
        connect(_sel,SIGNAL(selectionChanged (const QItemSelection&,const QItemSelection&)),this,SLOT(itemActivated(const QItemSelection&,const QItemSelection&)));
    }
}

DbOverview::~DbOverview()
{
    delete _data;
}

void DbOverview::showDbOverview(svn::Client*aClient)
{
    DbOverview*ptr = 0;
    static const char*cfg_text = "db_overview_dlg";
    KConfigGroup _kc(Kdesvnsettings::self()->config(),cfg_text);
    KDialog*dlg = createDialog(&ptr,QString(i18n("Overview about cache database content")),KDialog::Close,cfg_text,false);
    ptr->setClient(aClient);
    dlg->restoreDialogSize(_kc);
    dlg->exec();
    dlg->saveDialogSize(_kc);
    _kc.sync();
    delete dlg;
}

void DbOverview::setClient(svn::Client*aClient)
{
    _data->_Client = aClient;
}

void DbOverview::enableButtons(bool how)
{
    m_DeleteCacheButton->setEnabled(how);
    m_DeleteRepositoryButton->setEnabled(how);
    m_SettingsButton->setEnabled(how);
    m_StatisticButton->setEnabled(how);
}

void DbOverview::itemActivated(const QItemSelection&indexes,const QItemSelection&deindexes)
{
    Q_UNUSED(deindexes);

    enableButtons(false);
    QModelIndexList _indexes = indexes.indexes();
    if (_indexes.count()!=1) {
        kDebug()<<"Handle only with single selection"<<endl;
        return;
    }
    genInfo(_indexes[0].data().toString());
    enableButtons(true);
}

void DbOverview::genInfo(const QString&repo)
{
    svn::cache::ReposLog rl(_data->_Client,repo);
    const static QString info(i18n("Log cache holds %1 logentries and consumes %2 on disk."));
    QString msg = info.arg(rl.count()).arg(helpers::ByteToString(rl.fileSize()));
    m_RepostatusBrowser->setText(msg);
}

QString DbOverview::selectedRepository()const
{
    QModelIndexList _indexes = m_ReposListView->selectionModel()->selectedIndexes();
    if (_indexes.size()!=1) {
        return QString();
    }
    return _indexes[0].data().toString();
}

void DbOverview::deleteCacheItems()
{
    int i = KMessageBox::questionYesNo(this,i18n("Realy clean cache for repository\n%1?").arg(selectedRepository()),i18n("Clean repository cache"));
    if (i != KMessageBox::Yes) {
        return;
    }
    try
    {
        svn::cache::ReposLog rl(_data->_Client,selectedRepository());
        rl.cleanLogEntries();
    }
    catch (const svn::cache::DatabaseException&e)
    {
        kDebug()<<e.msg();
    }
    genInfo(selectedRepository());
}

void DbOverview::deleteRepository()
{
    int i = KMessageBox::questionYesNo(this,i18n("Realy clean cache and data for repository\n%1?").arg(selectedRepository()),i18n("Delete repository"));
    if (i != KMessageBox::Yes) {
        return;
    }

    try {
        svn::cache::LogCache::self()->deleteRepository(selectedRepository());
        _data->repo_model->setStringList(svn::cache::LogCache::self()->cachedRepositories());
    }catch (const svn::cache::DatabaseException&e) {
        kDebug()<<e.msg()<<endl;
    }
}

void DbOverview::repositorySettings()
{
    DbSettings::showSettings(selectedRepository());
}

#include "dboverview.moc"
