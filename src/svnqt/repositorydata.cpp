/***************************************************************************
 *   Copyright (C) 2006 by Rajko Albrecht                                  *
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
#include "repositorydata.hpp"
#include "svncpp_defines.hpp"
#include "exception.hpp"
#include "repositorylistener.hpp"
#include "svnfilestream.hpp"

#include <svn_fs.h>
#include <svn_path.h>
#include <svn_config.h>

namespace svn {

namespace repository {

class RepoOutStream:public stream::SvnStream
{
public:
    RepoOutStream(RepositoryData*);
    virtual ~RepoOutStream(){}

    virtual bool isOk()const{return true;}
    virtual long write(const char*data,const unsigned long max);

protected:
    RepositoryData*m_Back;
};

RepoOutStream::RepoOutStream(RepositoryData*aBack)
    : SvnStream(false,true)
{
    m_Back = aBack;
}

long RepoOutStream::write(const char*data,const unsigned long max)
{
    if (m_Back) {
        QString msg = QString::fromUtf8(data,max);
        m_Back->reposFsWarning(msg);
    }
    return max;
}

RepositoryData::RepositoryData(RepositoryListener*aListener)
{
    m_Repository = 0;
    m_Listener = aListener;
}


RepositoryData::~RepositoryData()
{
}

void RepositoryData::warning_func(void *baton, svn_error_t *err)
{
    RepositoryData*_r = (RepositoryData*)baton;

    if (_r) {
        QString msg = svn::Exception::error2msg(err);
        svn_error_clear(err);
        _r->reposFsWarning(msg);
    }
}

void RepositoryData::reposFsWarning(const QString&msg)
{
    if (m_Listener) {
        m_Listener->sendWarning(msg);
    }
}

svn_error_t*RepositoryData::cancel_func(void*baton)
{
    RepositoryListener*m_L = (RepositoryListener*)baton;
    if (m_L && m_L->isCanceld()) {
        return svn_error_create (SVN_ERR_CANCELLED, 0, QString::fromUtf8("Cancelled by user.").TOUTF8());
    }
    return SVN_NO_ERROR;
}

/*!
    \fn svn::RepositoryData::close()
 */
void RepositoryData::Close()
{
    m_Pool.renew();
    m_Repository = 0;
}


/*!
    \fn svn::RepositoryData::Open(const QString&)
 */
svn_error_t * RepositoryData::Open(const QString&path)
{
    Close();
    svn_error_t * error = svn_repos_open(&m_Repository,path.TOUTF8(),m_Pool);
    if (error!=0L) {
        m_Repository=0;
        return error;
    }
    svn_fs_set_warning_func(svn_repos_fs(m_Repository), RepositoryData::warning_func, this);
    return SVN_NO_ERROR;
}


/*!
    \fn svn::RepositoryData::CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync = false, bool _bdbautologremove = true, bool nosvn1diff=false)
 */
svn_error_t * RepositoryData::CreateOpen(const QString&path, const QString&fstype, bool _bdbnosync, bool _bdbautologremove, bool _nosvn1diff)
{
    Close();
    const char* _type;
    if (fstype.lower()=="bdb") {
        _type="bdb";
    } else {
        _type="fsfs";
    }
    apr_hash_t *config;
    apr_hash_t *fs_config = apr_hash_make(m_Pool);

    apr_hash_set(fs_config, SVN_FS_CONFIG_BDB_TXN_NOSYNC,
                APR_HASH_KEY_STRING,
                (_bdbnosync ? "1" : "0"));
    apr_hash_set(fs_config, SVN_FS_CONFIG_BDB_LOG_AUTOREMOVE,
                APR_HASH_KEY_STRING,
                (_bdbautologremove ? "1" : "0"));
    apr_hash_set(fs_config, SVN_FS_CONFIG_FS_TYPE,
                 APR_HASH_KEY_STRING,
                 _type);

    /// @todo comes with 1.4!
    if (_nosvn1diff) {
#if (SVN_VER_MAJOR >= 1) && (SVN_VER_MINOR >= 4)
        apr_hash_set(fs_config, SVN_FS_CONFIG_NO_SVNDIFF1,
            APR_HASH_KEY_STRING,"1");
#endif
    }
    /// @todo config as extra paramter? Meanwhile default config only
    /// (see svn::ContextData)
    SVN_ERR(svn_config_get_config(&config, 0, m_Pool));
    const char*repository_path = apr_pstrdup (m_Pool,path.TOUTF8());

    repository_path = svn_path_internal_style(repository_path, m_Pool);

    if (svn_path_is_url(repository_path)) {
        return svn_error_createf(SVN_ERR_CL_ARG_PARSING_ERROR, NULL,
            "'%s' is an URL when it should be a path",repository_path);
    }
    SVN_ERR(svn_repos_create(&m_Repository, repository_path,
            NULL, NULL,config, fs_config,m_Pool));

    svn_fs_set_warning_func(svn_repos_fs(m_Repository), RepositoryData::warning_func, this);

    return SVN_NO_ERROR;
}


/*!
    \fn svn::RepositoryData::dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)
 */
svn_error_t* RepositoryData::dump(const QString&output,const svn::Revision&start,const svn::Revision&end, bool incremental, bool use_deltas)
{
    if (!m_Repository) {
        return svn_error_create(SVN_ERR_CANCELLED,0,"No repository selected.");
    }
    Pool pool;
    svn::stream::SvnFileOStream out(output);
    RepoOutStream backstream(this);
    svn_revnum_t _s,_e;
    _s = start.revnum();
    _e = end.revnum();
    SVN_ERR(svn_repos_dump_fs2(m_Repository,out,backstream,_s,_e,incremental,use_deltas,
        RepositoryData::cancel_func,m_Listener,pool));

    return SVN_NO_ERROR;
}

svn_error_t* RepositoryData::loaddump(const QString&dump,svn_repos_load_uuid uuida, const QString&parentFolder, bool usePre, bool usePost)
{
    if (!m_Repository) {
        return svn_error_create(SVN_ERR_CANCELLED,0,"No repository selected.");
    }
    svn::stream::SvnFileIStream infile(dump);
    RepoOutStream backstream(this);
    Pool pool;
    const char*src_path = apr_pstrdup (pool,dump.TOUTF8());
    const char*dest_path;
    if (parentFolder.isEmpty()) {
        dest_path=0;
    } else {
        dest_path=apr_pstrdup (pool,parentFolder.TOUTF8());
    }

    src_path = svn_path_internal_style(src_path, pool);
    SVN_ERR(svn_repos_load_fs2(m_Repository,infile,backstream,uuida,dest_path,usePre?1:0,usePost?1:0,RepositoryData::cancel_func,m_Listener,pool));
    return SVN_NO_ERROR;
}

svn_error_t* RepositoryData::hotcopy(const QString&src,const QString&dest,bool cleanlogs)
{
    Pool pool;
    const char*src_path = apr_pstrdup (pool,src.TOUTF8());
    const char*dest_path = apr_pstrdup (pool,dest.TOUTF8());
    src_path = svn_path_internal_style(src_path, pool);
    dest_path = svn_path_internal_style(dest_path, pool);
    SVN_ERR(svn_repos_hotcopy(src_path,dest_path,cleanlogs?1:0,pool));
    return SVN_NO_ERROR;
}

}

}
