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
#include "client_impl.hpp"

// subversion api
#include "svn_client.h"
//#include "svn_utf.h"

#include "path.hpp"
#include "exception.hpp"
#include "pool.hpp"
#include "revision.hpp"
#include "svnqt_defines.hpp"


namespace svn
{

  PathPropertiesMapList
  Client_impl::proplist(const Path &path,
                   const Revision &revision,
                   const Revision &peg,
                   bool recurse)
  {
    Pool pool;

    apr_array_header_t * props;
    svn_error_t * error =
      svn_client_proplist2(&props,
                           path.cstr (),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           *m_context,
                           pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    PathPropertiesMapList path_prop_map_list;
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

      path_prop_map_list.push_back( PathPropertiesMapEntry( QString::FROMUTF8(item->node_name->data), prop_map ) );
    }

    return path_prop_map_list;
  }

  PathPropertiesMapList
  Client_impl::propget(const QString& propName,
                  const Path &path,
                  const Revision &revision,
                  const Revision &peg,
                  bool recurse)
  {
    Pool pool;

    apr_hash_t *props;
    svn_error_t * error =
      svn_client_propget2(&props,
                           propName.TOUTF8(),
                           path.cstr (),
                           peg.revision(),
                           revision.revision (),
                           recurse,
                           *m_context,
                           pool);
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

    return path_prop_map_list;
  }

  void
  Client_impl::propset(const QString& propName,
                  const QString& propValue,
                  const Path &path,
                  const Revision &revision,
                  bool recurse,
                  bool skip_checks)
    {
      Pool pool;
      const svn_string_t * propval
        = svn_string_create (
                             propValue.TOUTF8(),
                             pool);

      svn_error_t * error =
        svn_client_propset2(
                            propName.TOUTF8(),
                            propval, path.cstr(),
                            recurse,skip_checks, *m_context, pool);
      if(error != NULL)
        throw ClientException (error);
    }

  void
  Client_impl::propdel(const QString& propName,
                  const Path &path,
                  const Revision &revision,
                  bool recurse)
  {
    Pool pool;
    svn_error_t * error =
              svn_client_propset2(
                                  propName.TOUTF8(),
                                  0, // value = NULL
                                  path.cstr (),
                                  recurse,
                                  false,
                                  *m_context,
                                  pool);
    if(error != NULL)
      throw ClientException (error);
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
  QPair<svn_revnum_t,PropertiesMap>
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

    return QPair<svn_revnum_t,PropertiesMap>( revnum, prop_map );
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

  QPair<svn_revnum_t,QString>
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
      return QPair<svn_revnum_t,QString>( 0, QString() );

    return QPair<svn_revnum_t,QString>( revnum, QString::FROMUTF8(propval->data) );
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
  svn_revnum_t
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
  svn_revnum_t
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
