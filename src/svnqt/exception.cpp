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


// svncpp
#include "exception.h"
#include "svnqt/svnqt_defines.h"

#include <qstring.h>

#ifdef HAS_BACKTRACE_H
#include <execinfo.h>
#include <qstringlist.h>
#define SVNQT_BACKTRACE_LENGTH 20
#endif

namespace svn
{

  struct Exception::Data
  {
  private:
  public:
    QString message;
    apr_status_t apr_err;

    Data (const char * msg)
      : message(QString::FROMUTF8(msg)),apr_err(0)
    {
    }

    Data (const QString& msg)
      : message(msg),apr_err(0)
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

  apr_status_t
  Exception::apr_err () const
  {
    return m->apr_err;
  }

  const QString&
  Exception::msg () const
  {
    return m->message;
  }

  void Exception::setMessage(const QString&aMsg)
  {
      m->message=aMsg;
  }

  QString Exception::error2msg(svn_error_t*error)
  {
    QString message = "";
    if (error==0) {
        return message;
    }
    svn_error_t * next = error->child;
    if (error->message)
      message = QString::FROMUTF8(error->message);
    else
    {
      message = "Unknown error!\n";
      if (error->file)
      {
        message += QString::FROMUTF8("In file ");
        message += QString::FROMUTF8(error->file);
        message += QString(" Line %1").arg(error->line);
      }
    }
    while (next != NULL && next->message != NULL)
    {
      message = message + '\n' + QString::FROMUTF8(next->message);

      next = next->child;
    }

    return message;

  }

  ClientException::ClientException (const char*msg) throw ()
    : Exception (msg)
  {
  }

  ClientException::ClientException (const QString&msg) throw ()
    : Exception (msg)
  {
  }

  ClientException::ClientException (svn_error_t * error) throw ()
    : Exception ("")
  {
    init();
    if (error == 0)
        return;

    m->apr_err = error->apr_err;
    m->message += error2msg(error);
    svn_error_clear (error);
  }

  ClientException::ClientException (apr_status_t status) throw ()
    : Exception ("")
  {
      init();
      m->apr_err = status;
  }


  ClientException::~ClientException () throw ()
  {
  }

  ClientException::ClientException (const ClientException & src) throw ()
    : Exception (src.msg())
  {
    m->apr_err = src.apr_err();
  }

  void ClientException::init()
  {
#ifdef USEBACKTRACE
    if (m_backTraceConstr.length()==0) {
        m_backTraceConstr = getBackTrace();
        m->message=m_backTraceConstr;
    }
#else
    m_backTraceConstr="";
#endif
  }

  QString ClientException::getBackTrace()
  {
    QString Result;
    //qDebug("getBackTrace");
#ifdef HAS_BACKTRACE_H
    //qDebug("Generating backtrace");
    void *array[SVNQT_BACKTRACE_LENGTH];
    size_t size;
    size_t i;

    size = backtrace (array, SVNQT_BACKTRACE_LENGTH);
    if (!size) {
        return Result;
    }

    char ** strings = backtrace_symbols (array, size);

    QStringList r;
    for (i = 0; i < size; ++i) {
        r.push_back(QString::number(i) +
             QString::FROMUTF8(": ") +
             QString::FROMUTF8(strings[i]));
    }
    Result = QString::FROMUTF8("[\n")+r.join("\n")+QString::FROMUTF8("]\n");
    free (strings);
#endif
    return Result;
  }

}
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
