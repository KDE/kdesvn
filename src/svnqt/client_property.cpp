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

// svncpp
#include "client_impl.h"

// subversion api
#include <svn_client.h>
#include <svn_path.h>

#include "path.h"
#include "exception.h"
#include "pool.h"
#include "revision.h"
#include "svnqt_defines.h"
#include "client_parameter.h"
#include "helper.h"
#include <QCoreApplication>
#include <QDir>


namespace svn
{

struct ProplistBaton {
    ContextWP m_context;
    PathPropertiesMapListPtr resultlist;
};

static svn_error_t *ProplistReceiver(void *baton, const char *path, apr_hash_t *prop_hash, apr_pool_t *pool)
{
    ProplistBaton *_baton = static_cast<ProplistBaton *>(baton);
    PathPropertiesMapListPtr mapList = _baton->resultlist;

    ContextP l_context = _baton->m_context;
    if (!l_context) {
        return svn_error_create(SVN_ERR_CANCELLED, nullptr, QCoreApplication::translate("svnqt", "Cancelled by user.").toUtf8());
    }
    svn_client_ctx_t *ctx = l_context->ctx();
    if (ctx && ctx->cancel_func) {
        SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
    }

    mapList->push_back(PathPropertiesMapEntry(QString::fromUtf8(path), svn::internal::Hash2Map(prop_hash, pool)));
    return SVN_NO_ERROR;
}

PathPropertiesMapListPtr
Client_impl::proplist(const Path &path,
                      const Revision &revision,
                      const Revision &peg,
                      Depth depth,
                      const StringArray &changelists)
{
    Pool pool;

    PathPropertiesMapListPtr path_prop_map_list = PathPropertiesMapListPtr(new PathPropertiesMapList);

    ProplistBaton baton;
    baton.m_context = m_context;
    baton.resultlist = path_prop_map_list;
    // todo svn 1.8: svn_client_proplist4
    svn_error_t *error =
        svn_client_proplist3(
            path.cstr(),
            peg.revision(),
            revision.revision(),
            internal::DepthToSvn(depth),
            changelists.array(pool),
            ProplistReceiver,
            &baton,
            *m_context,
            pool);
    if (error != nullptr) {
        throw ClientException(error);
    }

    return path_prop_map_list;
}

QPair<qlonglong, PathPropertiesMapList>
Client_impl::propget(const QString &propName,
                     const Path &path,
                     const Revision &revision,
                     const Revision &peg,
                     Depth depth,
                     const StringArray &changelists
                    )
{
    Pool pool;

    apr_hash_t *props;
    svn_revnum_t actual = svn_revnum_t(-1);
    // todo svn 1.8: svn_client_propget5
    svn_error_t *error = svn_client_propget4(&props,
                                             propName.toUtf8(),
                                             path.cstr(),
                                             peg.revision(),
                                             revision.revision(),
                                             &actual,
                                             internal::DepthToSvn(depth),
                                             changelists.array(pool),
                                             *m_context,
                                             pool,
                                             pool
                                            );

    if (error != nullptr) {
        throw ClientException(error);
    }

    PathPropertiesMapList path_prop_map_list;

    apr_hash_index_t *hi;
    for (hi = apr_hash_first(pool, props); hi;
            hi = apr_hash_next(hi)) {
        PropertiesMap prop_map;

        const void *key;
        void *val;

        apr_hash_this(hi, &key, nullptr, &val);
        prop_map[propName] = QString::fromUtf8(((const svn_string_t *)val)->data);
        QString filename = QString::fromUtf8((const char *)key);
        path_prop_map_list.push_back(PathPropertiesMapEntry(filename, prop_map));
    }

    return QPair<qlonglong, PathPropertiesMapList>(actual, path_prop_map_list);
}

void
Client_impl::propset(const PropertiesParameter &params)
{
    Pool pool;
    const svn_string_t *propval;

    if (params.propertyValue().isNull()) {
        propval = nullptr;
    } else {
        propval = svn_string_create(params.propertyValue().toUtf8(), pool);
    }

    svn_error_t *error = nullptr;
    const QByteArray tgtTmp = params.path().cstr();
    if (svn_path_is_url(tgtTmp)) {
        error = svn_client_propset_remote(params.propertyName().toUtf8(),
                                          propval,
                                          tgtTmp,
                                          params.skipCheck(),
                                          params.revision(),
                                          map2hash(params.revisionProperties(), pool),
                                          nullptr, // we don't need a commit info - ignore
                                          nullptr,
                                          *m_context,
                                          pool
                                          );
    } else {
        apr_array_header_t *targets = apr_array_make(pool, 1,
                                                     sizeof(const char *));
        APR_ARRAY_PUSH(targets, const char *) = tgtTmp;
        error = svn_client_propset_local(params.propertyName().toUtf8(),
                                         propval,
                                         targets,
                                         internal::DepthToSvn(params.depth()),
                                         params.skipCheck(),
                                         params.changeList().array(pool),
                                         *m_context,
                                         pool);

    }

    if (error != nullptr) {
        throw ClientException(error);
    }
}

//--------------------------------------------------------------------------------
//
//    revprop functions
//
//--------------------------------------------------------------------------------
/**
 * lists revision properties in @a path no matter whether local or
 * repository
 *
 * @param path
 * @param revision
 * @param recurse
 * @return PropertiesList
 */
QPair<qlonglong, PropertiesMap>
Client_impl::revproplist(const Path &path,
                         const Revision &revision)
{
    Pool pool;

    apr_hash_t *props;
    svn_revnum_t revnum;
    svn_error_t *error =
        svn_client_revprop_list(&props,
                                path.cstr(),
                                revision.revision(),
                                &revnum,
                                *m_context,
                                pool);
    if (error != nullptr) {
        throw ClientException(error);
    }

    PropertiesMap prop_map;

    apr_hash_index_t *hi;
    for (hi = apr_hash_first(pool, props); hi;
            hi = apr_hash_next(hi)) {
        const void *key;
        void *val;

        apr_hash_this(hi, &key, nullptr, &val);
        prop_map[ QString::fromUtf8((const char *)key) ] = QString::fromUtf8(((const svn_string_t *)val)->data);
    }

    return QPair<qlonglong, PropertiesMap>(revnum, prop_map);
}

/**
 * lists one revision property in @a path no matter whether local or
 * repository
 *
 * @param path
 * @param revision
 * @param recurse
 * @return PropertiesList
 */

QPair<qlonglong, QString>
Client_impl::revpropget(const QString &propName,
                        const Path &path,
                        const Revision &revision)
{
    Pool pool;

    svn_string_t *propval;
    svn_revnum_t revnum;
    svn_error_t *error =
        svn_client_revprop_get(
            propName.toUtf8(),
            &propval,
            path.cstr(),
            revision.revision(),
            &revnum,
            *m_context,
            pool);
    if (error != nullptr) {
        throw ClientException(error);
    }

    // if the property does not exist NULL is returned
    if (propval == nullptr) {
        return QPair<qlonglong, QString>(0, QString());
    }

    return QPair<qlonglong, QString>(revnum, QString::fromUtf8(propval->data));
}

qlonglong
Client_impl::revpropset(const PropertiesParameter &param)
{
    Pool pool;

    const svn_string_t *propval
        = param.propertyValue().isNull() ? nullptr : svn_string_create(param.propertyValue().toUtf8(), pool);

    svn_revnum_t revnum;

    const svn_string_t *oldpropval = param.propertyOriginalValue().isNull() ? nullptr : svn_string_create(param.propertyOriginalValue().toUtf8(), pool);
    svn_error_t *error = svn_client_revprop_set2(
                             param.propertyName().toUtf8(),
                             propval,
                             oldpropval,
                             param.path().cstr(),
                             param.revision().revision(),
                             &revnum,
                             param.force(),
                             *m_context,
                             pool);

    if (error != nullptr) {
        throw ClientException(error);
    }

    return revnum;
}

qlonglong
Client_impl::revpropdel(const QString &propName,
                        const Path &path,
                        const Revision &revision)
{
    Pool pool;

    svn_revnum_t revnum;
    svn_error_t *error =
        svn_client_revprop_set2(
            propName.toUtf8(),
            nullptr, // value = NULL
            nullptr,
            path.cstr(),
            revision.revision(),
            &revnum,
            false,
            *m_context,
            pool);

    if (error != nullptr) {
        throw ClientException(error);
    }

    return revnum;
}

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
