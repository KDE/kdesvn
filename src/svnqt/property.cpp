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

// subversion api
#include "svn_client.h"
//#include "svn_utf.h"

// svncpp
#include "exception.hpp"
#include "path.hpp"
#include "pool.hpp"
#include "property.hpp"
#include "revision.hpp"
#include "svncpp_defines.hpp"


namespace svn
{

  PropertyEntry::PropertyEntry (const char * name, const char * value)
  {
    this->name = QString::fromUtf8(name);
    this->value = QString::fromUtf8(value);
  }

  PropertyEntry::PropertyEntry (const QString& name, const QString& value)
  {
    this->name = name;
    this->value = value;
  }

#if 0
  Property::Property (Context * context, const Path & path)
    : m_context (context), m_path (path)
  {
    list ();
  }

  Property::~Property ()
  {
  }

  void
  Property::list ()
  {
    Pool pool;
    Revision revision;

    m_entries.clear ();
    apr_array_header_t * props;
    svn_error_t * error =
      svn_client_proplist (&props,
                           m_path.cstr (),
                           revision,
                           false, /* recurse */
                           *m_context,
                           pool);
    if(error != NULL)
    {
      throw ClientException (error);
    }

    for (int j = 0; j < props->nelts; ++j)
    {
      svn_client_proplist_item_t *item =
        ((svn_client_proplist_item_t **)props->elts)[j];

      apr_hash_index_t *hi;

      for (hi = apr_hash_first (pool, item->prop_hash); hi;
           hi = apr_hash_next (hi))
      {
        const void *key;
        void *val;

        apr_hash_this (hi, &key, NULL, &val);
        QString key_ = QString::fromUtf8((const char *)key);
        m_entries.push_back(PropertyEntry(key_, getValue(key_)));
      }
    }
  }

  QString
  Property::getValue (const QString& name)
  {
    Pool pool;
    Revision revision;

    apr_hash_t *props;
    svn_client_propget (&props,
                        name.TOUTF8(),
                        m_path.cstr(),
                        revision,
                        false, // recurse
                        *m_context,
                        pool);

    //svn_boolean_t is_svn_prop = svn_prop_is_svn_prop (name);

    apr_hash_index_t *hi;
    hi = apr_hash_first (pool, props);
    if( !hi )
    {
      return "";
    }

    const void *key;
    void *val;
    const svn_string_t *propval;
    apr_hash_this (hi, &key, NULL, &val);
    propval = (const svn_string_t *)val;
    return QString::fromUtf8(propval->data);
  }

  void
  Property::set (const QString& name, const QString& value)
  {
    Pool pool;

    const svn_string_t * propval
        = svn_string_create (value.TOUTF8(), pool);

    svn_error_t * error =
      svn_client_propset (
                          name.TOUTF8(),
                          propval, m_path.cstr (),
                          false, pool);
    if(error != NULL)
      throw ClientException (error);
  }

  void
  Property::remove (const QString& name)
  {
    Pool pool;

//    const char *pname_utf8;
  //  svn_utf_cstring_to_utf8 (&pname_utf8, name, pool);

    svn_error_t * error;
    error = svn_client_propset (
                                  name.TOUTF8(),
                                  0, // value = NULL
                                  m_path.cstr (),
                                  false, //dont recurse
                                  pool);
    if(error != NULL)
      throw ClientException (error);
  }

  /* REMOVE
  bool
  PropertyValue::isSvnProperty (const char * name)
  {
    Pool pool;

    const char *pname_utf8;
    svn_utf_cstring_to_utf8 (&pname_utf8, name, pool);
    svn_boolean_t is_svn_prop = svn_prop_is_svn_prop (pname_utf8);

    return is_svn_prop;
  }
  */
#endif
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
