/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
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
#include "exception.h"
#include "path.h"
#include "pool.h"
#include "wc.h"
#include "context.h"

#include "entry.h"

#include "svnqt/helper.h"

namespace svn
{
    const char * Wc::ADM_DIR_NAME = SVN_WC_ADM_DIR_NAME;

    Wc::Wc(const ContextP&context)
        :_context(context)
    {
    }

    Wc::~Wc()
    {
        _context = 0;
    }

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
                const QString& url, const Revision & revision,
                const QString&repository, Depth depth) throw (ClientException)
  {
    Pool pool;
    Path dirPath (dir);
    Path urlPath (url);

    const char*rep = 0;

    if (!repository.isNull()) {
        rep = repository.TOUTF8();
    }

    svn_error_t * error =
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
      svn_wc_ensure_adm3(
                         dirPath.path().TOUTF8(),    // path
                         uuid.TOUTF8(),                // UUID
                         urlPath.path().TOUTF8(),    // url
                         rep,
                         revision.revnum (),  // revision
                         internal::DepthToSvn(depth),
                         pool);
#else
      svn_wc_ensure_adm2(
                         dirPath.path().TOUTF8(),    // path
                         uuid.TOUTF8(),                // UUID
                         urlPath.path().TOUTF8(),    // url
                         rep,
                         revision.revnum (),  // revision
                         pool);
#endif
    if(error != NULL)
      throw ClientException (error);
  }

  Entry Wc::getEntry( const QString &path )const throw ( ClientException )
  {
    Pool pool;
    Path itemPath(path);
    svn_error_t * error = 0;
    svn_wc_adm_access_t *adm_access;
    const svn_wc_entry_t *entry;

    svn_client_ctx_t*ctx = _context?_context->ctx():0;

    error = svn_wc_adm_probe_open3(&adm_access,0,itemPath.path().TOUTF8(),false,0,
        ctx?ctx->cancel_func:0,ctx?ctx->cancel_baton:0,pool);
    if (error!=0) {
        throw ClientException(error);
    }
    error = svn_wc_entry(&entry,itemPath.path().TOUTF8(),adm_access,false,pool);
    if (error!=0) {
        throw ClientException(error);
    }
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 6)) || (SVN_VER_MAJOR > 1)
    error = svn_wc_adm_close2(adm_access,pool);
#else
    error = svn_wc_adm_close(adm_access);
#endif
    if (error!=0) {
        throw ClientException(error);
    }
    return Entry(entry);
  }

  QString Wc::getUrl(const QString&path)const throw (ClientException)
  {
    QString result = "";
    Entry entry = getEntry( path );
    return entry.isValid()?entry.url():"";
  }

  QString Wc::getRepos(const QString&path)const throw (ClientException)
  {
    QString result = "";
    Entry entry = getEntry( path );
    return entry.isValid() ? entry.repos():QString::fromLatin1("");
  }
}
