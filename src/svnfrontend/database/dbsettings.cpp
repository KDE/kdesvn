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
#include "dbsettings.h"
#include "ui_dbsettings.h"

#include "svnqt/cache/ReposConfig.h"
#include <QPointer>

DbSettings::DbSettings(const QString &repository, QWidget *parent)
    : KSvnDialog(QLatin1String("db_settings_dlg"), parent)
    , m_repository(repository)
    , m_ui(new Ui::DbSettings)
{
    m_ui->setupUi(this);
    setDefaultButton(m_ui->buttonBox->button(QDialogButtonBox::Ok));
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    setWindowTitle(i18nc("@title:window", "Settings for %1", repository));
    init();
}

DbSettings::~DbSettings()
{
    delete m_ui;
}

void DbSettings::init()
{
    m_ui->dbcfg_exclude_box->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, "tree_exclude_list", QStringList()));
    m_ui->dbcfg_exclude_userslog->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, "exclude_log_users", QStringList()));
    m_ui->dbcfg_exclude_log_pattern->setItems(svn::cache::ReposConfig::self()->readEntry(m_repository, "exclude_log_pattern", QStringList()));
    m_ui->dbcfg_noCacheUpdate->setChecked(svn::cache::ReposConfig::self()->readEntry(m_repository, "no_update_cache", false));
    m_ui->dbcfg_filter_empty_author->setChecked(svn::cache::ReposConfig::self()->readEntry(m_repository, "filter_empty_author", false));
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
    store_list(m_ui->dbcfg_exclude_box, "tree_exclude_list");
    store_list(m_ui->dbcfg_exclude_userslog, "exclude_log_users");
    store_list(m_ui->dbcfg_exclude_log_pattern, "exclude_log_pattern");
    svn::cache::ReposConfig::self()->setValue(m_repository, "no_update_cache", m_ui->dbcfg_noCacheUpdate->isChecked());
    svn::cache::ReposConfig::self()->setValue(m_repository, "filter_empty_author", m_ui->dbcfg_filter_empty_author->isChecked());
    KSvnDialog::accept();
}

void DbSettings::showSettings(const QString &repository, QWidget *parent)
{
    QPointer<DbSettings> dlg(new DbSettings(repository, parent ? parent : QApplication::activeModalWidget()));
    dlg->exec();
    delete dlg;
}
