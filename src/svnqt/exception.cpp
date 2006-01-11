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

#include <qstring.h>

// svncpp
#include "exception.hpp"


namespace svn
{

  struct Exception::Data
  {
  public:
    QString message;
    apr_status_t apr_err;

    Data (const char * msg)
      : message(QString::fromUtf8(msg))
    {
    }

    Data (const QString& msg)
      : message(msg)
    {
    }


    Data (const Data& other)
      : message(other.message), apr_err(other.apr_err)
    {
    }
  };

  Exception::Exception (const char * message) throw ()
  {
    m = new Data (message);
  }

  Exception::Exception (const QString& message) throw ()
  {
    m = new Data (message);
  }

  Exception::Exception (const Exception & other) throw ()
  {
    m = new Data (*other.m);
  }

  Exception::~Exception () throw ()
  {
    delete m;
  }

  const apr_status_t
  Exception::apr_err () const
  {
    return m->apr_err;
  }

  const QString&
  Exception::msg () const
  {
    return m->message;
  }


  ClientException::ClientException (const char*msg) throw ()
    : Exception (msg)
  {
  }

  ClientException::ClientException (svn_error_t * error) throw ()
    : Exception ("")
  {
    if (error == 0)
      return;

    m->apr_err = error->apr_err;
    svn_error_t * next = error->child;
    /// @todo send rapidsvn an hint that error->message may sometimes NULL!
    if (error->message)
      m->message = QString::fromUtf8(error->message);
    else
    {
      m->message = "Unknown error!\n";
      if (error->file)
      {
        m->message += "In file ";
        m->message += QString::fromUtf8(error->file);
        m->message += QString(" Line %1").arg(error->line);
      }
    }
    while (next != NULL && next->message != NULL)
    {
      m->message = m->message + "\n" + QString::fromUtf8(next->message);

      next = next->child;
    }
    svn_error_clear (error);
  }


  ClientException::ClientException (apr_status_t status) throw ()
    : Exception ("")
  {
    m->apr_err = status;
  }


  ClientException::~ClientException () throw ()
  {
  }

  ClientException::ClientException (const ClientException & src) throw ()
    : Exception (src.msg())
  {
  }
}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
