#include "dboverview.h"
#include "src/svnqt/cache/ReposConfig.hpp"
#include "src/svnqt/cache/LogCache.hpp"
#include "src/svnqt/cache/DatabaseException.hpp"
#include "src/svnfrontend/fronthelpers/createdlg.h"

#include <QStringListModel>

#include <KDebug>

class DbOverViewData
{

public:
    QStringListModel*repo_model;

    DbOverViewData()
    {
        repo_model = new QStringListModel();
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
    _data = new DbOverViewData;

    try {
        _data->repo_model->setStringList(svn::cache::LogCache::self()->cachedRepositories());
    }catch (const svn::cache::DatabaseException&e) {
        kDebug()<<e.msg()<<endl;
    }

    m_ReposListView->setModel(_data->repo_model);
}

DbOverview::~DbOverview()
{
    delete _data;
}

void DbOverview::showDbOverview()
{
    DbOverview*ptr = 0;
    static const char*cfg_text = "db_overview_dlg";
    KDialog*dlg = createDialog(&ptr,QString(i18n("Overview about cache database content")),false,"DatabaseOverview",cfg_text);
    dlg->exec();
    KConfigGroup _kc(Kdesvnsettings::self()->config(),cfg_text);
    dlg->saveDialogSize(_kc);
    delete dlg;
}

#include "dboverview.moc"
