#include "dboverview.h"
#include "src/svnqt/cache/ReposConfig.hpp"
#include "src/svnqt/cache/LogCache.hpp"
#include "src/svnqt/cache/ReposLog.hpp"
#include "src/svnqt/cache/DatabaseException.hpp"
#include "src/svnfrontend/fronthelpers/createdlg.h"
#include "src/svnqt/client.hpp"
#include "helpers/stringhelper.h"

#include <QStringListModel>
#include <QItemSelectionModel>

#include <KDebug>

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
    KDialog*dlg = createDialog(&ptr,QString(i18n("Overview about cache database content")),false,"DatabaseOverview",cfg_text);
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

void DbOverview::itemActivated(const QItemSelection&indexes,const QItemSelection&deindexes)
{
    Q_UNUSED(deindexes);

    QModelIndexList _indexes = indexes.indexes();
    if (_indexes.count()!=1) {
        kDebug()<<"Handle only with single selection"<<endl;
        return;
    }
    QModelIndex index = _indexes[0];
    QString repo = index.data().toString();
    svn::cache::ReposLog rl(_data->_Client,repo);
    const static QString info("Log cache holds %1 logentries and consumes %2 on disk.");
    QString msg = info.arg(rl.count()).arg(helpers::ByteToString()(rl.fileSize()));
    m_RepostatusBrowser->setText(msg);
}

#include "dboverview.moc"
