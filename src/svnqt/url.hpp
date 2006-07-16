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

#ifndef _SVNCPP_URL_H_
#define _SVNCPP_URL_H_

// qt
#include <qglobal.h>
#if QT_VERSION < 0x040000

#include <qstring.h>
#include <qvaluelist.h>

#else

#include <QtCore>

#endif


namespace svn
{
  class Url
  {
  public:
    /** Constructor */
    Url ();

    /** Destructor */
    virtual ~Url ();

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

    static QString
    transformProtokoll(const QString&);

    /**
     * returns a vector with url schemas that are
     * supported by svn
     *
     * @return vector with entries like "file:", "http:"
     */
#if QT_VERSION < 0x040000
    static QValueList<QString>
#else
    static QList<QString>
#endif
    supportedSchemas ();
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */

