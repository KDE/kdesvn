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

#ifndef SVNQT_TARGETS_H
#define SVNQT_TARGETS_H

#include "svnqt/svnqt_defines.h"
#include "svnqt/svnqttypes.h"


// apr api
#include "apr_tables.h"

class QStringList;

namespace svn
{
  // forward declarations
  class Pool;

  /**
   * Encapsulation for Subversion target arrays handling
   */
  class SVNQT_EXPORT Targets
  {
  public:
    /**
     * Constructor
     *
     * @param targets vector of paths
     */
    Targets (const svn::Pathes & targets);

    /**
     * Constructor
     * @param path a single paths
     */
    Targets (const svn::Path & target);

    /**
     * Constructor from an APR array containing
     * char *.
     *
     * @param targets APR array header
     */
    Targets (const apr_array_header_t * targets);

    /**
     * Constructor. Initializes list with just
     * one entry
     *
     * @param target
     */
    Targets (const QString& target = QString());
    /**
     * Constructor. Initializes list with just
     * one entry
     *
     * @param target
     */
    Targets (const char * target);
    /**
     * Constructor. Convert stringlist into target list.
     * @param targets
     */
    Targets(const QStringList&targets);

    /**
     * Copy Constructor
     *
     * @param targets Source
     */
    Targets (const Targets & targets);

    /**
     * Destructor
     */
    virtual ~Targets ();

    /**
     * Returns an apr array containing
     * char *.
     *
     * @param pool Pool used for conversion
     */
    apr_array_header_t *
    array (const Pool & pool) const;

    /**
     * Returns a vector of paths
     *
     * @return vector of paths
     */
    const Pathes &
    targets() const;

    /**
     * @return the number of targets
     */
    size_t size () const;

    /**
     * operator to return the vector
     *
     * @return vector with targets
     */
    operator const Pathes & () const
    {
      return m_targets;
    }

    const Path& operator [](size_t which)const;
    /**
     * returns one single target.
     * the first in the vector, if no parameter given if there are more
     * than one. if there is no target or parameter > then stored pathes returns
     * an empty path
     * \param which which item we want
     * @return single path
     */
    const Path
    target(Pathes::size_type which) const;


  private:
    Pathes m_targets;
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
