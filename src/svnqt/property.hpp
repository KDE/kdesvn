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

#ifndef _SVNCPP_PROPERTY_H_
#define _SVNCPP_PROPERTY_H_


// Ignore MSVC 6 compiler warning: debug symbol truncated
#if defined (_MSC_VER) && _MSC_VER <= 1200
#pragma warning (disable: 4786)
#endif

// Ignore MSVC 7 compiler warning: C++ exception specification
#if defined (_MSC_VER) && _MSCVER > 1200 && _MSCVER <= 1310
#pragma warning (disable: 4290)
#endif

// qt
#if QT_VERSION < 0x040000

#include <qstring.h>
#include <qvaluelist.h>

#else

#include <QtCore>

#endif

// svncpp
#include "context.hpp"
#include "path.hpp"


namespace svn
{
  struct PropertyEntry
  {
    QString name;
    QString value;

    PropertyEntry (const char * name, const char * value);
    PropertyEntry (const QString& name = QString::null, const QString& value = QString::null);
  };

  // forward declarations
  class Path;

  /**
   * Class for manipulating Subversion properties.
   */
  class Property
  {
  public:
    Property (Context * context = 0,
              const Path & path = "");

    virtual ~Property ();

    /**
     * get the list of properties for the path.
     * throws an exception if the path isnt versioned.
     */
#if QT_VERSION < 0x040000
    const QValueList<PropertyEntry> &
#else
    const QList<PropertyEntry> &
#endif
    entries () const
    {
      return m_entries;
    }

    /**
     * Sets an existing property with a new value or adds a new
     * property.  If a result is added it does not reload the
     * result set.  Run loadPath again.
     * @exception ClientException
     */
    void set (const QString& name, const QString& value);

    /**
     * Deletes a property.
     * @exception ClientException
     */
    void remove (const QString& name);

  private:
    Context * m_context;
    Path m_path;
#if QT_VERSION < 0x040000
    QValueList<PropertyEntry> m_entries;
#else
    QList<PropertyEntry> m_entries;
#endif

    QString getValue (const QString& name);
    void list ();

  };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
