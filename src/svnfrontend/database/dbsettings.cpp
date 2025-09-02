/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht  ral@alwins-world.de        *
 *   https://kde.org/applications/development/org.kde.kdesvn               *
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
 * history and logs, available at https://commits.kde.org/kdesvn.          *
 ***************************************************************************/
#include "dbsettings.h"
#include "ui_dbsettings.h"

#include "svnqt/cache/ReposConfig.h"
#include <QPointer>

using namespace Qt::StringLiterals;

DbSettings::DbSettings(const QString &repository, QWidget *parent)
    : KSvnDialog(QLatin1String("db_settings_dlg"), parent)
    , m_repository(repository)
    , m_ui(new Ui::DbSettings)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Ok));
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &DbSettings::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &DbSettings::reject);
    setWindowTitle(i18nc("@title:window", "Settings for %1", repository));
    init();
}

DbSettings::~DbSettings()
{
    delete m_ui;
}

void DbSettings::init()
{
    m_ui->dbcfg_exclude_box->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, u"tree_exclude_list"_s, QStringList()));
    m_ui->dbcfg_exclude_userslog->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, u"exclude_log_users"_s, QStringList()));
    m_ui->dbcfg_exclude_log_pattern->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, u"exclude_log_pattern"_s, QStringList()));
    m_ui->dbcfg_noCacheUpdate->setChecked(svn::cache::ReposConfig::self()->readEntry(m_repository, u"no_update_cache"_s, false));
    m_ui->dbcfg_filter_empty_author->setChecked(svn::cache::ReposConfig::self()->readEntry(m_repository, u"filter_empty_author"_s, false));
}

void DbSettings::store_list(KEditListWidget *which, const QString &key)
{
    if (!which || key.isEmpty()) {
        return;
    }
    const QStringList _v = which->items();
    if (!_v.isEmpty()) {
        svn::cache::ReposConfig::self()->setValue(m_repository, key, _v);
    } else {
        svn::cache::ReposConfig::self()->eraseValue(m_repository, key);
    }
}

void DbSettings::accept()
{
    store_list(m_ui->dbcfg_exclude_box, u"tree_exclude_list"_s);
    store_list(m_ui->dbcfg_exclude_userslog, u"exclude_log_users"_s);
    store_list(m_ui->dbcfg_exclude_log_pattern, u"exclude_log_pattern"_s);
    svn::cache::ReposConfig::self()->setValue(m_repository, u"no_update_cache"_s, m_ui->dbcfg_noCacheUpdate->isChecked());
    svn::cache::ReposConfig::self()->setValue(m_repository, u"filter_empty_author"_s, m_ui->dbcfg_filter_empty_author->isChecked());
    KSvnDialog::accept();
}

void DbSettings::showSettings(const QString &repository, QWidget *parent)
{
    QPointer<DbSettings> dlg(new DbSettings(repository, parent ? parent : QApplication::activeModalWidget()));
    dlg->exec();
    delete dlg;
}

#include "moc_dbsettings.cpp"
