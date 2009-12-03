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
#if defined( _MSC_VER) && _MSC_VER <= 1550
#pragma warning( disable: 4786 )// debug symbol truncated
#endif

// svncpp
#include "svnqt/client.h"
#include "svnqt/client_impl.h"
#include "svnqt/svnqt_defines.h"

#include "svn_opt.h"

#include <svn_cmdline.h>

#include <qstringlist.h>
#include <qdir.h>

namespace svn
{
    //! this namespace contains only internal stuff not for public use
    namespace internal {
    //! small helper class
    /*!
        There will be an static instance created for calling the constructor at program load.
     */
        class SvnInit
        {
            public:
        //! constructor calling initialize functions
                SvnInit();
                ~SvnInit(){};
        };

        SvnInit::SvnInit() {
            svn_cmdline_init("svnqt",0);
            //qDebug("svn_cmdline_init done");
            QString BasePath=QDir::HOMEDIR();
            QDir d;
            if (!d.exists(BasePath)) {
                d.mkdir(BasePath);
            }
            BasePath=BasePath+'/'+".svnqt";
            if (!d.exists(BasePath)) {
                d.mkdir(BasePath);
            }

        }
    }

  Client::Client()
  {
  }

  Client::~Client ()
  {
  }

  Client*Client::getobject(ContextP context,int subtype)
  {
      static internal::SvnInit sInit;
    switch(subtype) {
      case 0:
       return new Client_impl(context);
       break;
      default:
       break;
    }
    return 0L;
  }
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
