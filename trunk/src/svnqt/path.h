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

#ifndef SVNQT_PATH_H
#define SVNQT_PATH_H

#include <qstring.h>
#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"

namespace svn
{
  /**
   * Encapsulation for Subversion Path handling
   */
  class SVNQT_EXPORT Path
  {
  private:
    QString m_path;

    /**
     * initialize the class
     *
     * @param path Path string - when url this should NOT hold revision as @ parameter!!!!! (will filtered out)
     */
    void init (const QString& path);

  public:
    /**
     * Constructor that takes a string as parameter.
     * The string is converted to subversion internal
     * representation. The string is copied.
     *
     * @param path Path string - when url this should NOT hold revision as @ parameter!!!!! (will filtered out)
     */
    Path (const QString & path = QString());

    /**
     * Constructor
     *
     * @see Path::Path (const QString &)
     * @param path Path string - when url this should NOT hold revision as @ parameter!!!!! (will filtered out)
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
     * @return Path string
     */
    operator const QString&()const;

    /**
     * @return Path as pretty url
     */
    QString prettyPath()const;

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

    /** Reduce path to its parent folder.
     * If the path length is 1 (eg., only "/") it will cleared so
     * path length will get zero.
     * @sa svn_path_remove_component
     */
    void
    removeLast();


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

    /** Parse a string for a peg revision
     * @param pathorurl url to parse
     * @param _path target to store the cleaned url
     * @param _peg target where to store the peg url.
     * @throw svn::ClientException on errors
     */
    static void
    parsePeg(const QString&pathorurl,Path&_path,svn::Revision&_peg);


    /** return the length of the path-string */
    unsigned int
    length () const;


    /** returns the path with native separators */
    QString
    native () const;

    /** returns if the path is a valid url, eg. points to a remote */
    bool isUrl()const;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
