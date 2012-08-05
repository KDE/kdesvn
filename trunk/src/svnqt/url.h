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

#ifndef SVNQT_URL_H
#define SVNQT_URL_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/pool.h"

#include <QString>
#include <QByteArray>

namespace svn
{
  class SVNQT_EXPORT Url
  {
  private:
      QByteArray m_Uri;
      Pool m_Pool;

  public:
    /** Constructor */
    Url ();
    Url(const QString&);
    Url(const QByteArray&);
    Url(const Url&);
    
    /** Destructor */
    ~Url ();

    void data(const QString&);
    void data(const QByteArray&);

    operator const char*()const;
    operator const QByteArray&()const;

    /**
     * Checks if @a url is valid
     *
     * Example of a valid URL:
     *   http://svn.collab.net/repos/svn
     * Example of an invalid URL:
     *   /home/foo/bar
     */
    static bool
    isValid (const QString& url);

    /**
     * Checks if @a url points to a local filesystem.
     *
     * @return true if url is accessed local without network.
     */
    static bool
    isLocal(const QString& url);

    static QString
    transformProtokoll(const QString&);

  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

