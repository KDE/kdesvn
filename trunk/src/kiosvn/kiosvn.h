/***************************************************************************
 *   Copyright (C) 2005-2009 by Rajko Albrecht                             *
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
#include <kurl.h>

#include <kio/global.h>
#include <kio/slavebase.h>


#include <sys/stat.h>

namespace svn
{
    class DirEntry;
}

namespace KIO
{

class KioSvnData;

/**
@author Rajko Albrecht
*/
class kio_svnProtocol : public KIO::SlaveBase,public StreamWrittenCb
{
public:
    kio_svnProtocol(const QByteArray&pool_socket, const QByteArray&app_socket);
    virtual ~kio_svnProtocol();
    virtual void listDir (const KUrl&url);
    virtual void stat(const KUrl& url);
    virtual void get(const KUrl& url);
    virtual void mkdir (const KUrl &url, int permissions);
    virtual void mkdir (const KUrl::List &urls, int permissions);
    virtual void put(const KUrl&url,int permissions,KIO::JobFlags flags);
    virtual void rename(const KUrl&src,const KUrl&target,KIO::JobFlags flags);
    virtual void del(const KUrl&url,bool isfile);
    virtual void copy(const KUrl&src,const KUrl&dest,int permissions,KIO::JobFlags flags);
    virtual void checkout(const KUrl&src,const KUrl&target,const int rev, const QString&revstring);
    virtual void svnlog(int,const QString&,int, const QString&, const KUrl::List&);
    virtual void revert(const KUrl::List&);
    virtual void wc_switch(const KUrl&,const KUrl&,bool,int,const QString&);
    virtual void diff(const KUrl&,const KUrl&,int,const QString&,int, const QString&,bool);
    virtual void import( const KUrl& repos, const KUrl& wc);
    virtual void add(const KUrl& wc);
    virtual void wc_delete(const KUrl::List&);
    virtual void special(const QByteArray& data);
    virtual void wc_resolve(const KUrl&,bool);
    /* looked on kio::svn from kdesdk */
    enum KSVN_METHOD {
        /* KUrl repository, KUrl target, int revnumber, QString revkind */
        SVN_CHECKOUT = 1,
        /* KUrl wc, int revnumber, QString revkind */
        /* refkind may empty or HEAD or START, will get parsed if revnumber is -1 */
        SVN_UPDATE = 2,
        /* KUrl::List */
        SVN_COMMIT = 3,
        /* int revstart, QString revstartstring, int revend, QString revendstring, KUrl::List */
        SVN_LOG=4,
        SVN_IMPORT=5,
        /* KUrl */
        SVN_ADD=6,
        /*KUrl::List */
        SVN_DEL=7,
        /* KUrl::List */
        SVN_REVERT=8,
        /* KUrl wc,bool checkRepos, bool recurse */
        SVN_STATUS=9,
        /* KUrl::List */
        SVN_MKDIR=10,
        /* KUrl, bool */
        SVN_RESOLVE=11,
        /* KUrl working copy, KUrl new_repository_url, bool recurse, int rev, QString revstring */
        SVN_SWITCH=12,
        /* KUrl uri1, KUrl uri2, int r1, QString rstring1, int r2, QString rstring 2, bool recursive */
        SVN_DIFF=13
    };

    void contextProgress(long long int current, long long int max);
    virtual void streamWritten(const KIO::filesize_t current);
    virtual void streamPushData(QByteArray);
    virtual void streamSendMime(KMimeType::Ptr mt);

    virtual void listSendDirEntry(const svn::DirEntry&);

    virtual bool checkKioCancel()const;

protected:
    virtual void commit(const KUrl::List&);
    virtual void status(const KUrl&,bool,bool);
    virtual void update(const KUrl&,int,const QString&);

    virtual void notify(const QString&text);
    virtual void extraError(int _errid,const QString&text);

private:
    KioSvnData*m_pData;
    bool createUDSEntry( const QString& filename, const QString& user, long long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
    QString makeSvnUrl(const KUrl&url,bool check_wc=true);
    bool checkWc(const KUrl&url);
    bool getLogMsg(QString&);

    void registerToDaemon();
    void unregisterFromDaemon();
    void startOp(qulonglong max, const QString&title);
    void stopOp(const QString&message);

protected:
    QString getDefaultLog();
    bool supportOverwrite()const;
    bool useKioprogress()const;
};

}

#endif
