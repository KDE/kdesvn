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

// subversion api
#include "svn_wc.h"

// svncpp
#include "exception.hpp"
#include "path.hpp"
#include "pool.hpp"
#include "wc.hpp"

namespace svn
{
  const char * Wc::ADM_DIR_NAME = SVN_WC_ADM_DIR_NAME;

  bool
  Wc::checkWc (const QString& dir)
  {
    Pool pool;
    Path path (dir);
    int wc;

    svn_error_t * error = svn_wc_check_wc (
        path.path().TOUTF8(),
        &wc, pool);

    if ((error != NULL) || (wc == 0))
    {
      return false;
    }

    return true;
  }

  void
  Wc::ensureAdm (const QString& dir, const QString& uuid,
                 const QString& url, const Revision & revision) throw (ClientException)
  {
    Pool pool;
    Path dirPath (dir);
    Path urlPath (url);

    svn_error_t * error =
      svn_wc_ensure_adm (
                         dirPath.path().TOUTF8(),    // path
                         uuid.TOUTF8(),                // UUID
                         urlPath.path().TOUTF8(),    // url
                         revision.revnum (),  // revision
                         pool);
    if(error != NULL)
      throw ClientException (error);
  }

  const svn_wc_entry_t *Wc::getEntry( const QString &path ) throw ( ClientException )
  {
    Pool pool;
    Path itemPath(path);
    svn_error_t * error = 0;
    svn_wc_adm_access_t *adm_access;
    const svn_wc_entry_t *entry;
    error = svn_wc_adm_probe_open2(&adm_access,0,itemPath.path().TOUTF8(),false,0,pool);
    if (error!=0) {
        throw ClientException(error);
    }
    error = svn_wc_entry(&entry,itemPath.path().TOUTF8(),adm_access,false,pool);
    if (error!=0) {
        throw ClientException(error);
    }
    error = svn_wc_adm_close(adm_access);
    if (error!=0) {
        throw ClientException(error);
    }
    return entry;
  }

  QString Wc::getUrl(const QString&path) throw (ClientException)
  {
    QString result = "";
    const svn_wc_entry_t *entry;
    entry = getEntry( path );
    result = entry?QString::FROMUTF8(entry->url):"";

    return result;
  }

  QString Wc::getRepos(const QString&path) throw (ClientException)
  {
    QString result = "";
    const svn_wc_entry_t *entry;
    entry = getEntry( path );
    result = entry ? QString::FROMUTF8(entry->repos) : QString::fromLatin1("");

    return result;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
