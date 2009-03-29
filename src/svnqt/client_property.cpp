/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
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
#include "svnqt/client_impl.hpp"

// subversion api
#include "svn_client.h"
//#include "svn_utf.h"

#include "svnqt/path.hpp"
#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/svnqt_defines.hpp"

#include "svnqt/helper.hpp"


namespace svn
{

    static svn_error_t* ProplistReceiver(void*baton,const char*path,apr_hash_t*prop_hash,apr_pool_t*pool)
    {
        Client_impl::propBaton*_baton=(Client_impl::propBaton*)baton;
        PropertiesMap prop_map;
        PathPropertiesMapList*mapList = (PathPropertiesMapList*)_baton->resultlist;

        Context*l_context = _baton->m_context;
        svn_client_ctx_t*ctx = l_context->ctx();
        if (ctx&&ctx->cancel_func) {
            SVN_ERR(ctx->cancel_func(ctx->cancel_baton));
        }

        apr_hash_index_t *hi;
        for (hi = apr_hash_first (pool, prop_hash); hi;
             hi = apr_hash_next (hi))
        {
            const void *key;
            void *val;

            apr_hash_this (hi, &key, NULL, &val);
            prop_map[ QString::FROMUTF8( (const char *)key ) ] =
                    QString::FROMUTF8( ((const svn_string_t *)val)->data );
        }
        mapList->push_back(PathPropertiesMapEntry(QString::FROMUTF8(path), prop_map ));
        return 0;
    }

  PathPropertiesMapListPtr
  Client_impl::proplist(const Path &path,
                   const Revision &revision,
                   const Revision &peg,
                   Depth depth,
                   const StringArray&changelists)
  {
    Pool pool;

    PathPropertiesMapListPtr path_prop_map_list = PathPropertiesMapListPtr(new PathPropertiesMapList);

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    propBaton baton;
    baton.m_context=m_context;
    baton.resultlist=path_prop_map_list;
    svn_error_t * error =
            svn_client_proplist3(
                           path.cstr (),
                           peg.revision(),
                           revision.revision (),
                           internal::DepthToSvn(depth),
                           changelists.array(pool),
                           ProplistReceiver,
                           &baton,
                           *m_context,
                           pool);
#else
    Q_UNUSED(changelists);
    Q_UNUSED(ProplistReceiver);
    bool recurse=depth==DepthInfinity;
    apr_array_header_t * props;
    svn_error_t * error =
      svn_client_proplist2(&props,
                           path.cstr (),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           *m_context,
                           pool);

#endif
    if(error != NULL)
    {
      throw ClientException (error);
    }

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR < 5))
    for (int j = 0; j < props->nelts; ++j)
    {
      svn_client_proplist_item_t *item =
        ((svn_client_proplist_item_t **)props->elts)[j];

      PropertiesMap prop_map;

      apr_hash_index_t *hi;
      for (hi = apr_hash_first (pool, item->prop_hash); hi;
           hi = apr_hash_next (hi))
      {
        const void *key;
        void *val;

        apr_hash_this (hi, &key, NULL, &val);
        prop_map[ QString::FROMUTF8( (const char *)key ) ] =
             QString::FROMUTF8( ((const svn_string_t *)val)->data );
      }

      path_prop_map_list->push_back( PathPropertiesMapEntry( QString::FROMUTF8(item->node_name->data), prop_map ) );
    }
#endif
    return path_prop_map_list;
  }

  QPair<QLONG,PathPropertiesMapList>
  Client_impl::propget(const QString& propName,
                  const Path &path,
                  const Revision &revision,
                  const Revision &peg,
                  Depth depth,
                  const StringArray&changelists
                      )
  {
    Pool pool;

    apr_hash_t *props;
    svn_revnum_t actual = svn_revnum_t(-1);
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    svn_error_t * error = svn_client_propget3(&props,
                                               propName.TOUTF8(),
                                               path.cstr (),
                                               peg.revision(),
                                               revision.revision (),
                                               &actual,
                                               internal::DepthToSvn(depth),
                                               changelists.array(pool),
                                               *m_context,
                                               pool
                                             );
#else
    bool recurse=depth==DepthInfinity;
    Q_UNUSED(changelists);
    svn_error_t * error =
      svn_client_propget2(&props,
                           propName.TOUTF8(),
                           path.cstr (),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           *m_context,
                           pool);
#endif

    if(error != NULL)
    {
      throw ClientException (error);
    }

    PathPropertiesMapList path_prop_map_list;


    apr_hash_index_t *hi;
    for (hi = apr_hash_first (pool, props); hi;
         hi = apr_hash_next (hi))
    {
      PropertiesMap prop_map;

      const void *key;
      void *val;

      apr_hash_this (hi, &key, NULL, &val);
      prop_map[propName] = QString::FROMUTF8( ((const svn_string_t *)val)->data );
      path_prop_map_list.push_back( PathPropertiesMapEntry(QString::FROMUTF8((const char *)key), prop_map ) );
    }

    return QPair<QLONG,PathPropertiesMapList>(actual,path_prop_map_list);
  }

  void
  Client_impl::propset(const QString& propName,
                  const QString& propValue,
                  const Path &path,
                  Depth depth,
                  bool skip_checks,
                  const Revision&base_revision,
                  const StringArray&changelists,
                  const PropertiesMap&revProps
                      )
    {
      Pool pool;
      const svn_string_t * propval;

      if (propValue.isNull()) {
          propval=0;
      } else {
          propval = svn_string_create (propValue.TOUTF8(),pool);
      }

      svn_error_t * error = 0;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
      svn_commit_info_t * commit_info;
      svn_client_propset3(
            &commit_info,
            propName.TOUTF8(),
            propval, path.cstr(),
            internal::DepthToSvn(depth),skip_checks,
                                 base_revision,
                                 changelists.array(pool),
                                 map2hash(revProps,pool),
                                 *m_context, pool);

#else
      Q_UNUSED(changelists);
      Q_UNUSED(base_revision);
      Q_UNUSED(revProps);
      bool recurse = depth==DepthInfinity;
        svn_client_propset2(
                            propName.TOUTF8(),
                            propval, path.cstr(),
                            recurse,skip_checks, *m_context, pool);
#endif
      if(error != NULL) {
          throw ClientException (error);
      }
    }

  void
  Client_impl::propdel(const QString& propName,
                       const Path &path,
                       Depth depth,
                       bool skip_checks,
                       const Revision&base_revision,
                       const StringArray&changelists)
  {
      propset(propName,QString(),path,depth,skip_checks,base_revision,changelists);
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
  QPair<QLONG,PropertiesMap>
  Client_impl::revproplist(const Path &path,
                      const Revision &revision)
  {
    Pool pool;

    apr_hash_t * props;
    svn_revnum_t revnum;
    svn_error_t * error =
      svn_client_revprop_list (&props,
                               path.cstr (),
                               revision.revision (),
                               &revnum,
                               *m_context,
                               pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    PropertiesMap prop_map;

    apr_hash_index_t *hi;
    for (hi = apr_hash_first (pool, props); hi;
         hi = apr_hash_next (hi))
    {
      const void *key;
      void *val;

      apr_hash_this (hi, &key, NULL, &val);
      prop_map[ QString::FROMUTF8( (const char *)key ) ] = QString::FROMUTF8( ((const svn_string_t *)val)->data );
    }

    return QPair<QLONG,PropertiesMap>( revnum, prop_map );
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

  QPair<QLONG,QString>
  Client_impl::revpropget(const QString& propName,
                     const Path &path,
                     const Revision &revision)
  {
    Pool pool;

    svn_string_t *propval;
    svn_revnum_t revnum;
    svn_error_t * error =
      svn_client_revprop_get (
                              propName.TOUTF8(),
                              &propval,
                              path.cstr (),
                              revision.revision (),
                              &revnum,
                              *m_context,
                              pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    // if the property does not exist NULL is returned
    if( propval == NULL )
        return QPair<QLONG,QString>( 0, QString() );

    return QPair<QLONG,QString>( revnum, QString::FROMUTF8(propval->data) );
  }

  /**
   * set property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @param revprop
   * @return PropertiesList
   */
  QLONG
  Client_impl::revpropset(const QString& propName,
                     const QString& propValue,
                     const Path &path,
                     const Revision &revision,
                     bool force)
  {
    Pool pool;

    const svn_string_t * propval
      = svn_string_create (
                            propValue.TOUTF8(),
                            pool);

    svn_revnum_t revnum;
    svn_error_t * error =
      svn_client_revprop_set (
                              propName.TOUTF8(),
                              propval,
                              path.cstr (),
                              revision.revision (),
                              &revnum,
                              force,
                              *m_context,
                              pool);
    if(error != NULL)
      throw ClientException (error);

    return revnum;
  }

  /**
   * delete property in @a path no matter whether local or
   * repository
   *
   * @param path
   * @param revision
   * @param propName
   * @param propValue
   * @param recurse
   * @param revprop
   * @return PropertiesList
   */
  QLONG
  Client_impl::revpropdel(const QString& propName,
                  const Path &path,
                  const Revision &revision,
                  bool force)
  {
    Pool pool;

    svn_revnum_t revnum;
    svn_error_t * error =
              svn_client_revprop_set (
                                      propName.TOUTF8(),
                                      0, // value = NULL
                                      path.cstr (),
                                      revision.revision (),
                                      &revnum,
                                      force,
                                      *m_context,
                                      pool);
    if(error != NULL)
      throw ClientException (error);

    return revnum;
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
