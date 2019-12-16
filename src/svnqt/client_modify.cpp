/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * https://kde.org/applications/development/org.kde.kdesvn
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library (in the file LGPL.txt); if not,
 * write to the Free Software Foundation, Inc., 51 Franklin St,
 * Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif
// svncpp
#include "svnqt/client_impl.h"

// subversion api
#include "svn_client.h"

#include "svnqt/exception.h"
#include "svnqt/pool.h"
#include "svnqt/targets.h"
#include "svnqt/svnqt_defines.h"
#include "svnqt/stringarray.h"
#include "svnqt/client_parameter.h"
#include "svnqt/client_commit_parameter.h"
#include "svnqt/client_update_parameter.h"
#include "svnqt/url.h"

#include "svnqt/helper.h"

#include <QCoreApplication>

namespace svn
{
struct mBaton {
    mBaton(): m_context(), m_revision(Revision::UNDEFINED), m_date(), author(), commit_error(), repos_root() {}
    ContextWP m_context;
    svn::Revision m_revision;
    QString m_date, author, commit_error, repos_root;
};

static svn_error_t *commit_callback2(const svn_commit_info_t *commit_info, void *baton, apr_pool_t *pool)
{
    Q_UNUSED(pool);
    mBaton *m_baton = (mBaton *)baton;
    ContextP m_context = m_baton->m_context;
    if (!m_context) {
        return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "Cancelled by user.").toUtf8());
    }
    svn_client_ctx_t *ctx = m_context->ctx();
    if (ctx && ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }
    m_baton->author = QString::fromUtf8(commit_info->author);
    m_baton->commit_error = QString::fromUtf8(commit_info->post_commit_err);
    m_baton->m_date = QString::fromUtf8(commit_info->date);
    m_baton->repos_root = QString::fromUtf8(commit_info->repos_root);
    m_baton->m_revision = commit_info->revision;
    return SVN_NO_ERROR;
}

Revision
Client_impl::checkout(const CheckoutParameter &parameters)
{
    Pool subPool;
    svn_revnum_t revnum = 0;
    svn_error_t *error = nullptr;
    error = svn_client_checkout3(&revnum,
                                 parameters.moduleName().cstr(),
                                 parameters.destination().cstr(),
                                 parameters.peg().revision(),
                                 parameters.revision().revision(),
                                 internal::DepthToSvn(parameters.depth()),
                                 parameters.ignoreExternals(),
                                 parameters.overWrite(),
                                 *m_context,
                                 subPool);
    if (error != nullptr) {
        throw ClientException(error);
    }
    return Revision(revnum);
}

Revision
Client_impl::remove(const Targets &targets,
                    bool force,
                    bool keep_local,
                    const PropertiesMap &revProps
                   )
{
    Pool pool;
    svn_error_t *error;

    mBaton _baton;
    _baton.m_context = m_context;
    error = svn_client_delete4(
                targets.array(pool),
                force,
                keep_local,
                map2hash(revProps, pool),
                commit_callback2,
                &_baton,
                *m_context,
                pool
            );

    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.m_revision;
}

void
Client_impl::revert(const Targets &targets,
                    Depth depth,
                    const StringArray &changelist
                   )
{
    Pool pool;

    svn_error_t *error =
        svn_client_revert2((targets.array(pool)),
                           internal::DepthToSvn(depth),
                           changelist.array(pool),
                           *m_context,
                           pool);
    if (error != nullptr) {
        throw ClientException(error);
    }
}

void
Client_impl::add(const Path &path,
                 svn::Depth depth, bool force, bool no_ignore, bool add_parents)
{
    Pool pool;
    // todo svn 1.8: svn_client_add5
    svn_error_t *error =
        svn_client_add4(path.cstr(),
                        internal::DepthToSvn(depth),
                        force,
                        no_ignore,
                        add_parents,
                        *m_context,
                        pool);
    if (error != nullptr) {
        throw ClientException(error);
    }
}

Revisions
Client_impl::update(const UpdateParameter &params)
{
    Pool pool;
    Revisions resulting;
    svn_error_t *error;

    apr_pool_t *apr_pool = pool.pool();
    apr_array_header_t *apr_revisions = apr_array_make(apr_pool,
                                                       params.targets().size(),
                                                       sizeof(svn_revnum_t));
    error = svn_client_update4(&apr_revisions, params.targets().array(pool), params.revision(),
                               internal::DepthToSvn(params.depth()), params.sticky_depth(),
                               params.ignore_externals(), params.allow_unversioned(),
                               params.add_as_modification(), params.make_parents(),
                               *m_context, pool
                              );

    if (error != nullptr) {
        throw ClientException(error);
    }
    for (int i = 0; i < apr_revisions->nelts; ++i) {
        svn_revnum_t *_rev =
            &APR_ARRAY_IDX(apr_revisions, i, svn_revnum_t);

        resulting.push_back((*_rev));
    }
    return resulting;
}

svn::Revision
Client_impl::commit(const CommitParameter &parameters)
{
    Pool pool;

    mBaton _baton;
    _baton.m_context = m_context;
    m_context->setLogMessage(parameters.message());
    svn_error_t *error =
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
        svn_client_commit6(
#else
        svn_client_commit5(
#endif
            parameters.targets().array(pool),
            internal::DepthToSvn(parameters.depth()),
            parameters.keepLocks(),
            parameters.keepChangeList(),
            parameters.commitAsOperations(),
#if SVN_API_VERSION >= SVN_VERSION_CHECK(1,8,0)
            false, /* file externals */
            false, /* dir externals */
#endif
            parameters.changeList().array(pool),
            map2hash(parameters.revisionProperties(), pool),
            commit_callback2,
            &_baton,
            *m_context,
            pool
        );

    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.m_revision;
}

Revision
Client_impl::copy(const CopyParameter &parameter)
{
    if (parameter.srcPath().size() < 1) {
        throw ClientException("Wrong size of sources.");
    }

    Pool pool;
    apr_array_header_t *sources = apr_array_make(pool, parameter.srcPath().size(), sizeof(svn_client_copy_source_t *));
    // not using .array() 'cause some extra information is needed for copy
    for (const Path &path : parameter.srcPath().targets()) {
        svn_client_copy_source_t *source = (svn_client_copy_source_t *)apr_palloc(pool, sizeof(svn_client_copy_source_t));
        source->path = apr_pstrdup(pool, path.path().toUtf8());
        source->revision = parameter.srcRevision().revision();
        source->peg_revision = parameter.pegRevision().revision();
        APR_ARRAY_PUSH(sources, svn_client_copy_source_t *) = source;
    }
    mBaton _baton;
    _baton.m_context = m_context;

    svn_error_t *error =
        svn_client_copy6(
            sources,
            parameter.destination().cstr(),
            parameter.asChild(), parameter.makeParent(), parameter.ignoreExternal(),
            map2hash(parameter.properties(), pool),
            commit_callback2, &_baton,
            *m_context, pool);

    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.m_revision;
}

Revision
Client_impl::copy(const Path &srcPath,
                  const Revision &srcRevision,
                  const Path &destPath)
{
    return copy(CopyParameter(srcPath, destPath).srcRevision(srcRevision).asChild(true).makeParent(false));
}

svn::Revision Client_impl::move(const CopyParameter &parameter)
{
    Pool pool;

    // todo svn 1.8: svn_client_move7
    mBaton _baton;
    _baton.m_context = m_context;
    svn_error_t *error = svn_client_move6(
                             parameter.srcPath().array(pool),
                             parameter.destination().cstr(),
                             parameter.asChild(),
                             parameter.makeParent(),
                             map2hash(parameter.properties(), pool),
                             commit_callback2,
                             &_baton,
                             *m_context,
                             pool
                         );

    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.m_revision;

}

svn::Revision
Client_impl::mkdir(const Targets &targets,
                   const QString &msg,
                   bool makeParent,
                   const PropertiesMap &revProps
                  )
{
    Pool pool;
    m_context->setLogMessage(msg);



    svn_error_t *error = nullptr;
    mBaton _baton;
    _baton.m_context = m_context;
    error = svn_client_mkdir4
            (
                const_cast<apr_array_header_t *>(targets.array(pool)),
                makeParent,
                map2hash(revProps, pool),
                commit_callback2, &_baton,
                *m_context, pool
            );
    /* important! otherwise next op on repository uses that logmessage again! */
    m_context->setLogMessage(QString());

    if (error != nullptr) {
        throw ClientException(error);
    }

    return _baton.m_revision;
}

void
Client_impl::cleanup(const Path &path)
{
    Pool subPool;
    apr_pool_t *apr_pool = subPool.pool();

    svn_error_t *error =
        svn_client_cleanup(path.cstr(), *m_context, apr_pool);

    if (error != nullptr) {
        throw ClientException(error);
    }
}

void Client_impl::resolve(const Path &path, Depth depth, const ConflictResult &resolution)
{
    Pool pool;
    const svn_wc_conflict_result_t *aResult = resolution.result(pool);
    svn_error_t *error = svn_client_resolve(path.cstr(), internal::DepthToSvn(depth), aResult->choice, *m_context, pool);

    if (error != nullptr) {
        throw ClientException(error);
    }
}

Revision
Client_impl::doExport(const CheckoutParameter &params)
{
    Pool pool;
    svn_revnum_t revnum = 0;
    QByteArray _neolBA;
    const char *_neol;
    if (params.nativeEol().isNull()) {
        _neol = nullptr;
    } else {
        _neolBA = params.nativeEol().toUtf8();
        _neol = _neolBA.constData();
    }
    svn_error_t *error =
        svn_client_export5(
                           &revnum,
                           params.moduleName().cstr(),
                           params.destination().cstr(),
                           params.peg().revision(),
                           params.revision().revision(),
                           params.overWrite(),
                           params.ignoreExternals(),
                           params.ignoreKeywords(),
                           internal::DepthToSvn(params.depth()),
                           _neol,
                           *m_context,
                           pool);
    if (error != nullptr) {
        throw ClientException(error);
    }
    return Revision(revnum);
}

Revision
Client_impl::doSwitch(
    const Path &path,
    const Url &url,
    const Revision &revision,
    Depth depth,
    const Revision &peg,
    bool sticky_depth,
    bool ignore_externals,
    bool allow_unversioned,
    bool ignore_ancestry
)
{
    Pool pool;
    svn_revnum_t revnum = 0;
    svn_error_t *error = svn_client_switch3(
                &revnum,
                path.cstr(),
                url.cstr(),
                peg.revision(),
                revision.revision(),
                internal::DepthToSvn(depth),
                sticky_depth,
                ignore_externals,
                allow_unversioned,
                ignore_ancestry,
                *m_context,
                pool
            );
    if (error != nullptr) {
        throw ClientException(error);
    }
    return Revision(revnum);
}

Revision
Client_impl::import(const Path &path,
                    const Url &importRepository,
                    const QString &message,
                    svn::Depth depth,
                    bool no_ignore, bool no_unknown_nodetype,
                    const PropertiesMap &revProps
                   )

{
    Pool pool;

    m_context->setLogMessage(message);
    // todo svn 1.8: svn_client_import5
    mBaton _baton;
    _baton.m_context = m_context;
    svn_error_t *error =
        svn_client_import4(path.cstr(),
                           importRepository.cstr(),
                           internal::DepthToSvn(depth), no_ignore, no_unknown_nodetype,
                           map2hash(revProps, pool),
                           commit_callback2, &_baton,
                           *m_context, pool);

    /* important! otherwise next op on repository uses that logmessage again! */
    m_context->setLogMessage(QString());

    if (error != nullptr) {
        throw ClientException(error);
    }
    return _baton.m_revision;
}

void
Client_impl::relocate(const Path &path,
                      const Url &from_url,
                      const Url &to_url,
                      bool recurse,
                      bool ignore_externals)
{
    Q_UNUSED(recurse);
    Pool pool;
    svn_error_t *error =
        svn_client_relocate2(path.cstr(),
                             from_url.cstr(),
                             to_url.cstr(),
                             ignore_externals,
                             *m_context,
                             pool);

    if (error != nullptr) {
        throw ClientException(error);
    }
}

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
