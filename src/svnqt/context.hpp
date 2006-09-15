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

#ifndef _SVNCPP_CONTEXT_HPP_
#define _SVNCPP_CONTEXT_HPP_

#include "svnqt/svnqt_defines.hpp"

// qt
#include <qstring.h>

// Subversion api
#include "svn_client.h"

// svncpp
#include "pool.hpp"


namespace svn
{
  // forward declarations
  class ContextListener;
  class ContextData;

  /**
   * This class will hold the client context
   * and replace the old notification and baton
   * stuff
   */
  class SVNQT_EXPORT Context
  {
  public:
    /**
     * default constructor
     *
     * @param configDir location where the
     *                  subversion api stores its
     *                  configuration
     */
    Context (const QString & configDir=QString::null);

    /**
     * copy constructor
     *
     * @param src
     */
    Context (const Context &src);

    /**
     * destructor
     */
    virtual ~Context ();

    /**
     * enable/disable authentication caching
     *
     * @param value true=enable/false=disable
     */
    void setAuthCache (bool value);

    /**
     * set username/password for authentication
     */
    void setLogin (const QString& username, const QString& password);

    /**
     * operator to get svn_client_ctx object
     */
    operator svn_client_ctx_t * ();

    /**
     * return the svn_client_ctx object
     */
    svn_client_ctx_t * ctx ();

    /**
     * this will be called at the beginning of an action.
     * the log message will be reset.
     */
    void reset ();

    /**
     * set log message
     *
     * @param msg
     */
    void setLogMessage (const QString& msg);

    /**
     * get log message
     *
     * @return log message
     */
    const QString&
    getLogMessage () const;

    /**
     * get username
     *
     * @return username
     */
    const QString&
    getUsername () const;

    /**
     * get password
     *
     * @return password
     */
    const QString&
    getPassword () const;

    /**
     * set the listener for the context. The listener will be
     * called to poll authentication information and other
     * information like this
     *
     * @param listener
     */
    void
    setListener (ContextListener * listener);

    /**
     * get the listener
     *
     * @return the listener
     */
    ContextListener *
    getListener () const;

  private:
    ContextData * m;

    /**
     * disable assignment operator
     */
    Context & operator = (const Context &);
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
