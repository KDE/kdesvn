/***************************************************************************
 *   Copyright (C) 2005-2007 by Rajko Albrecht                             *
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

#include "kiobytestream.h"

#include <qstring.h>
#include <qcstring.h>
#include <kurl.h>

#include <kio/global.h>
#include <kio/slavebase.h>


#include <sys/stat.h>
#include <qvaluelist.h>

namespace KIO
{

class KioSvnData;

/**
@author Rajko Albrecht
*/
class kio_svnProtocol : public KIO::SlaveBase,public StreamWrittenCb
{
public:
    kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
    virtual ~kio_svnProtocol();
    virtual void listDir (const KURL&url);
    virtual void stat(const KURL& url);
    virtual void get(const KURL& url);
    virtual void mkdir (const KURL &url, int permissions);
    virtual void mkdir (const KURL::List &urls, int permissions);
    virtual void rename(const KURL&src,const KURL&target,bool force);
    virtual void del(const KURL&url,bool isfile);
    virtual void copy(const KURL&src,const KURL&dest,int permissions,bool overwrite);
    virtual void checkout(const KURL&src,const KURL&target,const int rev, const QString&revstring);
    virtual void svnlog(int,const QString&,int, const QString&, const KURL::List&);
    virtual void revert(const KURL::List&);
    virtual void wc_switch(const KURL&,const KURL&,bool,int,const QString&);
    virtual void diff(const KURL&,const KURL&,int,const QString&,int, const QString&,bool);
    virtual void import( const KURL& repos, const KURL& wc);
    virtual void add(const KURL& wc);
    virtual void wc_delete(const KURL::List&);
    virtual void special(const QByteArray& data);
    virtual void wc_resolve(const KURL&,bool);
    /* looked on kio::svn from kdesdk */
    enum KSVN_METHOD {
        /* KURL repository, KURL target, int revnumber, QString revkind */
        SVN_CHECKOUT = 1,
        /* KURL wc, int revnumber, QString revkind */
        /* refkind may empty or HEAD or START, will get parsed if revnumber is -1 */
        SVN_UPDATE = 2,
        /* KURL::List */
        SVN_COMMIT = 3,
        /* int revstart, QString revstartstring, int revend, QString revendstring, KURL::List */
        SVN_LOG=4,
        SVN_IMPORT=5,
        /* KURL */
        SVN_ADD=6,
        /*KURL::List */
        SVN_DEL=7,
        /* KURL::List */
        SVN_REVERT=8,
        /* KURL wc,bool checkRepos, bool recurse */
        SVN_STATUS=9,
        /* KURL::List */
        SVN_MKDIR=10,
        /* KURL, bool */
        SVN_RESOLVE=11,
        /* KURL working copy, KURL new_repository_url, bool recurse, int rev, QString revstring */
        SVN_SWITCH=12,
        /* KURL uri1, KURL uri2, int r1, QString rstring1, int r2, QString rstring 2, bool recursive */
        SVN_DIFF=13
    };

    void contextProgress(long long int current, long long int max);
    virtual void streamWritten(const KIO::filesize_t current);
    virtual void streamPushData(QByteArray);
    virtual void streamSendMime(KMimeMagicResult*mt);
    virtual void streamTotalSizeNull();

protected:
    virtual void commit(const KURL::List&);
    virtual void status(const KURL&,bool,bool);
    virtual void update(const KURL&,int,const QString&);

private:
    KioSvnData*m_pData;
    bool createUDSEntry( const QString& filename, const QString& user, long long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
    QString makeSvnUrl(const KURL&url,bool check_wc=true);
    bool checkWc(const KURL&url);
    bool getLogMsg(QString&);
protected:
    QString getDefaultLog();
};

}

#endif
