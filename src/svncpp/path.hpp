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

#ifndef _SVNCPP_PATH_HPP_
#define _SVNCPP_PATH_HPP_

#include <qstring.h>


namespace svn
{
  /**
   * Encapsulation for Subversion Path handling
   */
  class Path
  {
  private:
    QString m_path;

    /**
     * initialize the class
     *
     * @param path Path string
     */
    void init (const QString& path);

  public:
    /**
     * Constructor that takes a string as parameter.
     * The string is converted to subversion internal
     * representation. The string is copied.
     *
     * @param path Path string
     */
    Path (const QString & path = "");

    /**
     * Constructor
     *
     * @see Path::Path (const QString &)
     * @param path Path string
     */
    Path (const char * path);

    /**
     * Copy constructor
     *
     * @param path Path to be copied
     */
    Path (const Path & path);

    /**
     * Assignment operator
     */
    Path& operator=(const Path&);

    /**
     * @return Path string
     */
    const QString &
    path () const;

    /**
     * @return Path string as c string
     */
    const QByteArray cstr() const;

    /**
     * check whether a path is set. Right now
     * this checks only if the string is non-
     * empty.
     *
     * @return true if there is a path set
     */
    bool
    isset() const;


    /**
     * adds a new URL component to the path
     *
     * @param component new component to add
     */
    void
    addComponent (const char * component);


    /**
     * adds a new URL component to the path
     *
     * @param component new component to add
     */
    void
    addComponent (const QString & component);


    /**
     * split path in its components
     *
     * @param dirpath directory/path component
     * @param basename filename
     */
    void
    split (QString & dirpath, QString & basename) const;


    /**
     * split path in its components including
     * file extension
     *
     * @param dir directory component
     * @param filename filename
     * @param ext extension (including leading dot ".")
     */
    void
    split (QString & dir, QString & filename, QString & ext) const;


    /**
     * returns the temporary directory
     */
    static Path
    getTempDir ();


    /** return the length of the path-string */
    unsigned int
    length () const;


    /** returns the path with native separators */
    QString
    native () const;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
