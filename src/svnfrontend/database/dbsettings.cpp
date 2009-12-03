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
#include "src/svnqt/shared_pointer.h"
#include "src/svnqt/cache/ReposConfig.h"
#include "src/svnfrontend/fronthelpers/createdlg.h"

class DbSettingsData
{
public:
    DbSettingsData(){}
    ~DbSettingsData(){}

    QString m_repository;
};

DbSettings::DbSettings(QWidget*parent,const char*name)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    _data = new DbSettingsData;
}

DbSettings::~DbSettings()
{
    delete _data;
}

void DbSettings::setRepository(const QString&repository)
{
    _data->m_repository = repository;
    dbcfg_exclude_box->clear();
    init();
}

void DbSettings::init()
{
    QStringList _v = svn::cache::ReposConfig::self()->readEntry(_data->m_repository,"tree_exclude_list",QStringList());
    dbcfg_exclude_box->setItems(_v);
    dbcfg_noCacheUpdate->setChecked(svn::cache::ReposConfig::self()->readEntry(_data->m_repository,"no_update_cache",false));
}

void DbSettings::store()
{
    QStringList _v = dbcfg_exclude_box->items();
    if (_v.count()>0) {
        svn::cache::ReposConfig::self()->setValue(_data->m_repository,"tree_exclude_list",_v);
    } else {
        svn::cache::ReposConfig::self()->eraseValue(_data->m_repository,"tree_exclude_list");
    }
    svn::cache::ReposConfig::self()->setValue(_data->m_repository,"no_update_cache",dbcfg_noCacheUpdate->isChecked());
}

void DbSettings::showSettings(const QString&repository)
{
    DbSettings*ptr = 0;
    static const char*cfg_text = "db_settings_dlg";
    KConfigGroup _kc(Kdesvnsettings::self()->config(),cfg_text);
    KDialog*dlg = createOkDialog(&ptr,i18n("Settings for %1",repository),true,"RepositorySettings",cfg_text);
    dlg->restoreDialogSize(_kc);
    ptr->setRepository(repository);
    if (dlg->exec()==QDialog::Accepted) {
        ptr->store();
    }
    dlg->saveDialogSize(_kc);
    _kc.sync();
    delete dlg;
}

#include "dbsettings.moc"
