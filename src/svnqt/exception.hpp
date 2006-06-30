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

#ifndef _SVNCPP_EXCEPTION_H_
#define _SVNCPP_EXCEPTION_H_

// subversion api
#include "svn_client.h"
#include <qstring.h>

namespace svn
{

  /**
   * Generic exception class.
   */
  class Exception
  {
  public:
    /**
     * Constructor.  Assigns the exception reason.
     */
    Exception (const char * message) throw ();
    Exception (const QString&message) throw();

    ~Exception () throw ();

    /**
     * @return the exception message.
     */
    const QString& msg() const;

    /**
     * @return the outermost error code.
     */
    const apr_status_t apr_err () const;

    static QString error2msg(svn_error_t*error);

  protected:
    struct Data;
    Data * m;

  private:

    Exception (const Exception &) throw ();

    Exception () throw ();

    Exception & operator = (const Exception &);
  };

  /**
   * Subversion client exception class.
   */
  class ClientException : public Exception
  {
  public:
    /**
     * Constructor.  Sets the error template and an optional message.
     * @param error the error to display. This will get cleared inside with svn_error_clear
     * so it isn't usable after that!
     */
    ClientException (svn_error_t * error) throw ();


    /**
     * Constructor that takes only an apr errorcode
     */
    ClientException (apr_status_t status) throw ();

   /**
    * Constructor
    */
    ClientException (const char*msg) throw ();

    /**
     * Copy constructor
     */
    ClientException (const ClientException & src) throw ();

    virtual ~ClientException () throw ();

  private:
    ClientException () throw ();

    ClientException & operator = (ClientException &);
  };

}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
