/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KIOSVN_H
#define KIOSVN_H

#include <qstring.h>
#include <qcstring.h>
#include <kurl.h>

#include <kio/global.h>
#include <kio/slavebase.h>


#include <sys/stat.h>
#include <qvaluelist.h>

class KioSvnData;

/**
@author Rajko Albrecht
*/
class kio_svnProtocol : public KIO::SlaveBase
{
public:
    kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
    virtual ~kio_svnProtocol();
    virtual void listDir (const KURL&url);
    virtual void stat(const KURL& url);
    virtual void get(const KURL& url);
    virtual void mkdir (const KURL &url, int permissions);
    virtual void rename(const KURL&src,const KURL&target,bool force);
    virtual void del(const KURL&url,bool isfile);
    virtual void copy(const KURL&src,const KURL&dest,int permissions,bool overwrite);

private:
    KioSvnData*m_pData;
    bool createUDSEntry( const QString& filename, const QString& user, long long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
    static QString makeSvnUrl(const KURL&url);
    bool getLogMsg(QString&);
};
#endif
