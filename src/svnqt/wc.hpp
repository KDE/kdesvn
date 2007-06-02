/* 
 * Port for usage with qt-framework and development for kdesvn
 * (C) 2005-2007 by Rajko Albrecht
 * http://www.alwins-world.de/wiki/programs/kdesvn
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

#ifndef _SVNCPP_WC_HPP_
#define _SVNCPP_WC_HPP_

// Ignore MSVC 7 & 2005 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1410
#pragma warning (disable: 4290)
#endif

// svncpp
#include "svnqt/exception.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/svnqt_defines.hpp"

#include <qstring.h>

namespace svn
{
  /**
   * Class that deals with a working copy
   */
  class SVNQT_EXPORT Wc
  {
  public:
    /**
     * check if Path is a valid working directory
     *
     * @param dir path to a directory
     * @return true=valid working copy
     */
    static bool
    checkWc (const QString& dir);

    /**
     * ensure that an administrative area exists for @a dir, so that @a dir
     * is a working copy subdir based on @a url at @a revision.
     *
     * @param dir path to a directory
     * @param uuid
     * @param url corresponding url
     * @param revision expected working copy revision
     */
    static void
    ensureAdm (const QString& dir, const QString& uuid,
               const QString& url, const Revision & revision) throw (ClientException);

    /**
     * retrieve the url of a given working copy item
     * @param path the working copy item to check
     * @return the repository url of @a path
     */
    static QString getUrl(const QString&path) throw (ClientException);
    static QString getRepos(const QString&path) throw (ClientException);
    static const char * ADM_DIR_NAME;

  private:
    static const svn_wc_entry_t *getEntry( const QString &path ) throw ( ClientException );

  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
