/*
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
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
#include "pool.hpp"
#include "url.hpp"

#include <qglobal.h>
#include <QtCore>

// subversion api
#include "svn_ra.h"

namespace svn
{
  static const char *
  VALID_SCHEMAS [] =
  {
    "http","https","file",
    "svn","svn+ssh","svn+http","svn+https","svn+file",
    "ksvn","ksvn+ssh","ksvn+http","ksvn+https","ksvn+file","ksvn",
    0
  };

  static bool mSchemasInitialized = false;
  QStringList mSchemas;

  Url::Url () {}

  Url::~Url () {}

  bool Url::isLocal(const QString& url)
  {
    Qt::CaseSensitivity cs=Qt::CaseInsensitive;
    if (
        url.startsWith("file://",cs) ||
        url.startsWith("/") ||
        url.startsWith("svn+file://",cs) ||
        url.startsWith("ksvn+file://",cs) )
    {
        return true;
    }
    return false;
  }

  bool Url::isValid (const QString& url)
  {
    QString urlTest(url);
    unsigned int index = 0;
    while (VALID_SCHEMAS[index]!=0)
    {
      QString schema = QString::FROMUTF8(VALID_SCHEMAS[index]);
      QString urlComp = urlTest.mid(0, schema.length());

      if (schema == urlComp)
      {
        return true;
      }
      ++index;
    }

    return false;
  }

  QString
  Url::transformProtokoll(const QString&prot)
  {
    QString _prot = prot.toLower();
    if (QString::compare(_prot,"svn+http")==0||
        QString::compare(_prot,"ksvn+http")==0) {
        return QString("http");
    } else if (QString::compare(_prot,"svn+https")==0||
               QString::compare(_prot,"ksvn+https")==0) {
        return QString("https");
    }else if (QString::compare(_prot,"svn+file")==0||
              QString::compare(_prot,"ksvn+file")==0) {
        return QString("file");
    } else if (QString::compare(_prot,"ksvn+ssh")==0) {
        return QString("svn+ssh");
    } else if (QString::compare(_prot,"ksvn")==0) {
        return QString("svn");
    }
    return _prot;
  }


  /**
   * the implementation of the function that pull the supported
   * url schemas out of the ra layer it rather dirty now since
   * we are lacking a higher level of abstraction
   */
  QStringList
  Url::supportedSchemas ()
  {
    if (mSchemasInitialized)
      return mSchemas;

    mSchemasInitialized = true;
    Pool pool;
    void * ra_baton;

    svn_error_t * error =
      svn_ra_init_ra_libs (&ra_baton, pool);
    if (error)
      return mSchemas;

    svn_stringbuf_t *descr;
    error =
      svn_ra_print_ra_libraries (&descr, ra_baton, pool);
    if (error)
      return mSchemas;

    // schemas are in the following form:
    // <schema>:<whitespace><description>\n...
    // find the f�st :
    QString descriptions (descr->data);
    int pos=0;
    const int not_found = -1;
    do
    {
      const QString tokenStart ("handles '");
      const QString tokenEnd ("' schem");
      pos = descriptions.indexOf( tokenStart, pos );
      if (pos == not_found)
        break;

      pos += tokenStart.length ();

      int posEnd = descriptions.indexOf( tokenEnd, pos );
      if (posEnd == not_found)
        break;

      // found
      QString schema (descriptions.mid(pos, posEnd-pos) + ":");
      mSchemas.push_back (schema);

      // forward to the next newline
      pos = posEnd + tokenEnd.length ();
    }
    while (pos != not_found);

    return mSchemas;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
