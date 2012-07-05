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
#if defined( _MSC_VER) && _MSC_VER <= 1200
#pragma warning( disable: 4786 )// debug symbol truncated
#endif


// svncpp
#include "pool.h"
#include "url.h"
#include "svnqt_defines.h"


#include <svn_dirent_uri.h>
#include <svn_version.h>

#include <qglobal.h>

namespace svn
{

    Url::Url ():m_Uri(),m_Pool() {}

    Url::Url(const QString&a_uri)
    :m_Pool()
    {
        data(a_uri);
    }
    
    Url::Url(const Url& other)
    :m_Pool()
    {
        m_Uri = other.m_Uri;
    }

    
    Url::Url(const QByteArray&a_uri)
    :m_Pool()
    {
        data(a_uri);
    }

    void Url::data(const QString&a_uri )
    {
        data(a_uri.toUtf8());
    }
    
    void Url::data(const QByteArray& a_uri)
    {
        m_Uri = a_uri;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 7)) || (SVN_VER_MAJOR > 1)
        if (!svn_uri_is_canonical(m_Uri.constData(),m_Pool))
        {
            m_Uri = svn_uri_canonicalize(m_Uri.constData(),m_Pool);
        }
#endif
    }

    Url::operator const char*()const
    {
        return m_Uri.constData();
    }
    
    Url::operator const QByteArray&()const
    {
        return m_Uri;
    }

    Url::~Url () {}
    
    

    /* static helpers */
  bool Url::isLocal(const QString& url)
  {
    static Qt::CaseSensitivity cs=Qt::CaseInsensitive;
    static QString stf("file://");
    static QString stsf("svn+file://");
    static QString stkf("ksvn+file://");
    if (
        url.startsWith(stf,cs) ||
        url.startsWith('/') ||
        url.startsWith(stsf,cs) ||
        url.startsWith(stkf,cs) )
    {
        return true;
    }
    return false;
  }

  bool Url::isValid (const QString& url)
  {
    static QString
    VALID_SCHEMAS [] =
    {
        QString("http"),QString("https"),QString("file"),
        QString("svn"),QString("svn+ssh"),QString("svn+http"),QString("svn+https"),QString("svn+file"),
        QString("ksvn"),QString("ksvn+ssh"),QString("ksvn+http"),QString("ksvn+https"),QString("ksvn+file"),
        QString()
    };
    QString urlTest(url);
    unsigned int index = 0;
    while (!VALID_SCHEMAS[index].isEmpty())
    {
      QString&schema = VALID_SCHEMAS[index];
      QString urlComp = urlTest.mid(0, schema.length());

      if (schema == urlComp)
      {
        return true;
      }
      ++index;
    }
    return false;
  }

  QString
  Url::transformProtokoll(const QString&prot)
  {
    QString _prot = prot.toLower();
    if (QString::compare(_prot,"svn+http")==0||
        QString::compare(_prot,"ksvn+http")==0) {
        return QString("http");
    } else if (QString::compare(_prot,"svn+https")==0||
               QString::compare(_prot,"ksvn+https")==0) {
        return QString("https");
    }else if (QString::compare(_prot,"svn+file")==0||
              QString::compare(_prot,"ksvn+file")==0) {
        return QString("file");
    } else if (QString::compare(_prot,"ksvn+ssh")==0) {
        return QString("svn+ssh");
    } else if (QString::compare(_prot,"ksvn")==0) {
        return QString("svn");
    }
    return _prot;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
