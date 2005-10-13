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
#include "kiosvn.h"
#include "kiolistener.h"

#include "svnfrontend/svnactions.h"
#include "svnfrontend/dummydisplay.h"
#include "svncpp/dirent.hpp"
#include "svncpp/url.hpp"
#include "svncpp/status.hpp"
#include "helpers/sub2qt.h"

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdemacros.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>
#include <dcopclient.h>
#include <qcstring.h>
#include <kmimetype.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class KioSvnData
{
public:
    KioSvnData();
    virtual ~KioSvnData();

    void reInitClient();

    KioListener m_Listener;
    svn::Context* m_CurrentContext;
    svn::Client m_Svnclient;

    svn::Revision urlToRev(const KURL&);
};

KioSvnData::KioSvnData()
{
    m_CurrentContext = 0;
    reInitClient();
}

void KioSvnData::reInitClient()
{
    delete m_CurrentContext;
    m_CurrentContext = new svn::Context();
    m_CurrentContext->setListener(&m_Listener);
    m_Svnclient.setContext(m_CurrentContext);
}

KioSvnData::~KioSvnData()
{

}

svn::Revision KioSvnData::urlToRev(const KURL&url)
{
    QMap<QString,QString> q = url.queryItems();
    svn::Revision rev,tmp;
    rev = svn::Revision::UNDEFINED;
    if (q.find("rev")!=q.end()) {
        QString v = q["rev"];
        m_Svnclient.url2Revision(v,rev,tmp);
    }
    return rev;
}

kio_svnProtocol::kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket)
    : SlaveBase("kio_svn", pool_socket, app_socket)
{
    m_pData=new KioSvnData;
}

kio_svnProtocol::~kio_svnProtocol()
{
    delete m_pData;
}

extern "C"
{
/* we will get trouble on old fedora 2 systems with the origin compiler - don't know whats up
 * with there gcc
 */
 #define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
#if GCC_VERSION == 30303
#undef KDE_EXPORT
#define KDE_EXPORT
#endif
    KDE_EXPORT int kdemain(int argc, char **argv);
}

int kdemain(int argc, char **argv)    {
    kdDebug()<<"kdemain" << endl;
    KInstance instance( "kio_svn" );

    kdDebug(7101) << "*** Starting kio_svn " << endl;

    if (argc != 4) {
        kdDebug(7101) << "Usage: kio_svn  protocol domain-socket1 domain-socket2" << endl;
        exit(-1);
    }

    kio_svnProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** kio_svn Done" << endl;
    return 0;
}


/*!
    \fn kio_svnProtocol::listDir (const KURL&url)
 */
void kio_svnProtocol::listDir(const KURL&url)
{
    kdDebug() << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;
    m_pData->reInitClient();
    svn::DirEntries dlist;
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    try {
        dlist = m_pData->m_Svnclient.list(makeSvnUrl(url),rev,false);
    } catch (svn::ClientException e) {
            QString ex = QString::fromUtf8(e.message());
            kdDebug()<<ex<<endl;
            error(KIO::ERR_SLAVE_DEFINED,ex);
            return;
    }

    KIO::UDSEntry entry;
    for (unsigned int i=0; i < dlist.size();++i) {
        QDateTime dt = helpers::sub2qt::apr_time2qt(dlist[i].time());
        if (createUDSEntry(dlist[i].name(),dlist[i].lastAuthor(),dlist[i].size(),
            dlist[i].kind()==svn_node_dir?true:false,dt.toTime_t(),entry) ) {
            listEntry(entry,false);
        }
        entry.clear();
    }
    listEntry(entry, true );
    finished();
}

void kio_svnProtocol::stat(const KURL& url)
{
    kdDebug()<<"kio_svn::stat "<< url << endl;
    m_pData->reInitClient();
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    svn::Status _stat;
    try {
        m_pData->m_Svnclient.singleStatus(makeSvnUrl(url),false,rev);
    } catch  (svn::ClientException e) {
        QString ex = QString::fromUtf8(e.message());
        kdDebug()<<ex<<endl;
        error( KIO::ERR_SLAVE_DEFINED,ex);
        return;
    }

    KIO::UDSEntry entry;
    QDateTime dt;
    dt = helpers::sub2qt::apr_time2qt(_stat.entry().cmtDate());
    kdDebug()<<"DateTime: " << dt << endl;
    if (_stat.entry().kind()==svn_node_file) {
        createUDSEntry(url.filename(),"",0,false,dt.toTime_t(),entry);
    } else {
        createUDSEntry(url.filename(),"",0,true,dt.toTime_t(),entry);
    }
    statEntry(entry);
    finished();
}

void kio_svnProtocol::get(const KURL& url)
{
    kdDebug()<<"kio_svn::get "<< url << endl;
    m_pData->reInitClient();
    svn::Revision rev = m_pData->urlToRev(url);
    if (rev == svn::Revision::UNDEFINED) {
        rev = svn::Revision::HEAD;
    }
    QByteArray content;
    try {
        content = m_pData->m_Svnclient.cat(makeSvnUrl(url),rev);
    } catch (svn::ClientException e) {
        QString ex = QString::fromUtf8(e.message());
        kdDebug()<<ex<<endl;
        error( KIO::ERR_SLAVE_DEFINED,ex);
        return;
    }
    KMimeType::Ptr mt = KMimeType::findByContent(content);
    kdDebug(7128) << "KMimeType returned : " << mt->name() << endl;
    mimeType( mt->name() );
    totalSize(content.size());
    //send data
    data(content);
    data(QByteArray()); // empty array means we're done sending the data
    finished();
}

QString kio_svnProtocol::makeSvnUrl(const KURL&url)
{
    QString res;
    QString proto = svn::Url::transformProtokoll(url.protocol());
    KURL _url = url;
    _url.cleanPath(true);
    _url.setProtocol(proto);
    res = _url.url(-1);
    QStringList s = QStringList::split("?",res);
    if (s.size()>1) {
        res = s[0];
    }
    while (res.endsWith("/")) {
        res.truncate(res.length()-1);
    }
    kdDebug()<<"Resulting url: " << res << endl;
    return res;
}

bool kio_svnProtocol::createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry)
{
        kdDebug() << "MTime : " << ( long )mtime << endl;
        kdDebug() << "UDS filename : " << filename << endl;
        kdDebug()<< "UDS Size: " << size << endl;
        KIO::UDSAtom atom;
        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = filename;
        entry.append( atom );

        atom.m_uds = KIO::UDS_FILE_TYPE;
        atom.m_long = isdir ? S_IFDIR : S_IFREG;
        entry.append( atom );

        atom.m_uds = KIO::UDS_SIZE;
        atom.m_long = size;
        entry.append( atom );

        atom.m_uds = KIO::UDS_MODIFICATION_TIME;
        atom.m_long = mtime;
        entry.append( atom );

        atom.m_uds = KIO::UDS_USER;
        atom.m_str = user;
        entry.append( atom );

        return true;
}
