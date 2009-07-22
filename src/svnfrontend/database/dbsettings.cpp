#include "dbsettings.h"
#include "src/svnqt/shared_pointer.hpp"
#include "src/svnqt/cache/ReposConfig.hpp"

class DbSettingsData
{
public:
    DbSettingsData(){}
    ~DbSettingsData(){}

    QString m_repository;
};

DbSettings::DbSettings(const QString&repository,QWidget*parent,const char*name)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    _data = new DbSettingsData;
    setRepository(repository);
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
    dbcfg_noCacheUpdate->setChecked(svn::cache::ReposConfig::self()->readEntry(_data->m_repository,"update_cache",false));
}

void DbSettings::store()
{
    QStringList _v = dbcfg_exclude_box->items();
    if (_v.count()>0) {
        svn::cache::ReposConfig::self()->setValue(_data->m_repository,"tree_exclude_list",_v);
    } else {
        svn::cache::ReposConfig::self()->eraseValue(_data->m_repository,"tree_exclude_list");
    }
    svn::cache::ReposConfig::self()->setValue(_data->m_repository,"update_cache",dbcfg_noCacheUpdate->isChecked());
}

#include "dbsettings.moc"