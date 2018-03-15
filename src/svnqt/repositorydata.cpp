/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 * This program is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this program (in the file LGPL.txt); if not,         *
 * write to the Free Software Foundation, Inc., 51 Franklin St,            *
 * Fifth Floor, Boston, MA  02110-1301  USA                                *
 *                                                                         *
 * This software consists of voluntary contributions made by many          *
 * individuals.  For exact contribution history, see the revision          *
 * history and logs, available at http://kdesvn.alwins-world.de.           *
 ***************************************************************************/
#include "svnqt/repositorydata.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/exception.h"
#include "svnqt/repositorylistener.h"
#include "svnqt/svnfilestream.h"
#include "svnqt/repoparameter.h"
#include "svnqt/reposnotify.h"

#include <svn_fs.h>
#include <svn_path.h>
#include <svn_config.h>
#include <svn_dirent_uri.h>
#include <svn_version.h>
#include <QCoreApplication>

namespace svn
{

namespace repository
{

class SVNQT_NOEXPORT RepoOutStream: public stream::SvnStream
{
public:
    RepoOutStream(RepositoryData *);

    bool isOk()const override
    {
        return true;
    }
    long write(const char *data, const unsigned long max) override;

protected:
    RepositoryData *m_Back;
};

RepoOutStream::RepoOutStream(RepositoryData *aBack)
    : SvnStream(false, true)
{
    m_Back = aBack;
}

long RepoOutStream::write(const char *data, const unsigned long max)
{
    if (m_Back) {
        QString msg = QString::fromUtf8(data, max);
        m_Back->reposFsWarning(msg);
    }
    return max;
}

RepositoryData::RepositoryData(RepositoryListener *aListener)
{
    m_Repository = nullptr;
    m_Listener = aListener;
}

RepositoryData::~RepositoryData()
{
}

void RepositoryData::warning_func(void *baton, svn_error_t *err)
{
    RepositoryData *_r = (RepositoryData *)baton;

    if (_r) {
        QString msg = svn::Exception::error2msg(err);
        svn_error_clear(err);
        _r->reposFsWarning(msg);
    }
}

void RepositoryData::repo_notify_func(void *baton, const svn_repos_notify_t *notify, apr_pool_t *scratch_pool)
{
    Q_UNUSED(scratch_pool)
    RepositoryData *_r = (RepositoryData *)baton;
    if (!notify || !_r) {
        return;
    }
    ReposNotify _rn(notify);
    QString msg = _rn;

    if (msg.length() > 0) {
        _r->reposFsWarning(msg);
    }
}

void RepositoryData::reposFsWarning(const QString &msg)
{
    if (m_Listener) {
        m_Listener->sendWarning(msg);
    }
}

svn_error_t *RepositoryData::cancel_func(void *baton)
{
    RepositoryListener *m_L = (RepositoryListener *)baton;
    if (m_L && m_L->isCanceld()) {
        return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "Cancelled by user.").toUtf8());
    }
    return SVN_NO_ERROR;
}

/*!
    \fn svn::RepositoryData::close()
 */
void RepositoryData::Close()
{
    m_Pool.renew();
    m_Repository = nullptr;
}

/*!
    \fn svn::RepositoryData::Open(const QString&)
 */
svn_error_t *RepositoryData::Open(const QString &path)
{
    Close();
    svn_error_t *error = svn_repos_open2(&m_Repository, path.toUtf8(), nullptr, m_Pool);
    if (error != nullptr) {
        m_Repository = nullptr;
        return error;
    }
    svn_fs_set_warning_func(svn_repos_fs(m_Repository), RepositoryData::warning_func, this);
    return SVN_NO_ERROR;
}

/*!
    \fn svn::RepositoryData::CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync = false, bool _bdbautologremove = true, bool nosvn1diff=false)
 */
svn_error_t *RepositoryData::CreateOpen(const CreateRepoParameter &params)
{
    Close();
    const char *_type;
    if (params.fstype().compare(QLatin1String("bdb"), Qt::CaseInsensitive) == 0) {
        _type = "bdb";
    } else {
        _type = "fsfs";
    }
    apr_hash_t *config;
    apr_hash_t *fs_config = apr_hash_make(m_Pool);

    apr_hash_set(fs_config, SVN_FS_CONFIG_BDB_TXN_NOSYNC,
                 APR_HASH_KEY_STRING,
                 (params.bdbnosync() ? "1" : "0"));
    apr_hash_set(fs_config, SVN_FS_CONFIG_BDB_LOG_AUTOREMOVE,
                 APR_HASH_KEY_STRING,
                 (params.bdbautologremove() ? "1" : "0"));
    apr_hash_set(fs_config, SVN_FS_CONFIG_FS_TYPE,
                 APR_HASH_KEY_STRING,
                 _type);

    if (params.pre15_compat()) {
        apr_hash_set(fs_config, SVN_FS_CONFIG_PRE_1_5_COMPATIBLE,
                     APR_HASH_KEY_STRING, "1");
    }
    if (params.pre16_compat()) {
        apr_hash_set(fs_config, SVN_FS_CONFIG_PRE_1_6_COMPATIBLE,
                     APR_HASH_KEY_STRING, "1");
    }
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
    if (params.pre18_compat()) {
        apr_hash_set(fs_config, SVN_FS_CONFIG_PRE_1_8_COMPATIBLE,
                     APR_HASH_KEY_STRING, "1");
    }
#endif

    /// @todo config as extra parameter? Meanwhile default config only
    /// (see svn::ContextData)
    SVN_ERR(svn_config_get_config(&config, nullptr, m_Pool));
    const char *repository_path = apr_pstrdup(m_Pool, params.path().toUtf8());

    repository_path = svn_dirent_internal_style(repository_path, m_Pool);

    if (svn_path_is_url(repository_path)) {
        return svn_error_create(SVN_ERR_CL_ARG_PARSING_ERROR, nullptr,
                                QCoreApplication::translate("svnqt", "'%1' is an URL when it should be a path").arg(params.path()).toUtf8());
    }
    SVN_ERR(svn_repos_create(&m_Repository, repository_path,
                             nullptr, nullptr, config, fs_config, m_Pool));

    svn_fs_set_warning_func(svn_repos_fs(m_Repository), RepositoryData::warning_func, this);

    return SVN_NO_ERROR;
}

/*!
    \fn svn::RepositoryData::dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)
 */
svn_error_t *RepositoryData::dump(const QString &output, const svn::Revision &start, const svn::Revision &end, bool incremental, bool use_deltas)
{
    if (!m_Repository) {
        return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "No repository selected.").toUtf8());
    }
    Pool pool;
    svn::stream::SvnFileOStream out(output);
    svn_revnum_t _s, _e;
    _s = start.revnum();
    _e = end.revnum();

    SVN_ERR(svn_repos_dump_fs3(m_Repository,
                               out, _s, _e, incremental, use_deltas,
                               RepositoryData::repo_notify_func,
                               this,
                               RepositoryData::cancel_func,
                               m_Listener,
                               pool));
    return SVN_NO_ERROR;
}

svn_error_t *RepositoryData::loaddump(const QString &dump, svn_repos_load_uuid uuida, const QString &parentFolder, bool usePre, bool usePost, bool validateProps)
{
    if (!m_Repository) {
        return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "No repository selected.").toUtf8());
    }
    svn::stream::SvnFileIStream infile(dump);
    RepoOutStream backstream(this);
    Pool pool;
    const char *src_path = apr_pstrdup(pool, dump.toUtf8());
    const char *dest_path;
    if (parentFolder.isEmpty()) {
        dest_path = nullptr;
    } else {
        dest_path = apr_pstrdup(pool, parentFolder.toUtf8());
    }
    src_path = svn_path_internal_style(src_path, pool);

    // todo svn 1.8: svn_repos_load_fs4
    SVN_ERR(svn_repos_load_fs3(m_Repository, infile, uuida, dest_path, usePre ? 1 : 0, usePost ? 1 : 0, validateProps ? 1 : 0,
                               RepositoryData::repo_notify_func,
                               this, RepositoryData::cancel_func, m_Listener, pool));
    return SVN_NO_ERROR;
}

svn_error_t *RepositoryData::hotcopy(const QString &src, const QString &dest, bool cleanlogs)
{
    Pool pool;
    const char *src_path = apr_pstrdup(pool, src.toUtf8());
    const char *dest_path = apr_pstrdup(pool, dest.toUtf8());
    src_path = svn_dirent_internal_style(src_path, pool);
    dest_path = svn_dirent_internal_style(dest_path, pool);
    // todo svn 1.8: svn_repos_hotcopy2
    SVN_ERR(svn_repos_hotcopy(src_path, dest_path, cleanlogs ? 1 : 0, pool));
    return SVN_NO_ERROR;
}

}

}
