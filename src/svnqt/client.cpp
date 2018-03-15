/*
 * Port for usage with qt-framework and development for kdesvn
 * Copyright (C) 2005-2009 by Rajko Albrecht (ral@alwins-world.de)
 * http://kdesvn.alwins-world.de
 */
/*
 * ====================================================================
 * Copyright (c) 2002-2005 The RapidSvn Group.  All rights reserved.
 * dev@rapidsvn.tigris.org
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
#include "client.h"
#include "client_impl.h"
#include "svnqt_defines.h"

#include <svn_opt.h>
#include <svn_cmdline.h>

#include <QDir>

namespace svn
{

Client::Client()
{
}

Client::~Client()
{
}

ClientP Client::getobject(const ContextP &context)
{
    static bool s_initialized = false;
    if (!s_initialized) {
        svn_cmdline_init("svnqt", nullptr);
        QString basePath = QDir::homePath();
        QDir d;
        if (!d.exists(basePath)) {
            d.mkpath(basePath);
        }
        basePath = basePath + QLatin1String("/.svnqt");
        if (!d.exists(basePath)) {
            d.mkdir(basePath);
        }
    }
    return ClientP(new Client_impl(context));
}
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
